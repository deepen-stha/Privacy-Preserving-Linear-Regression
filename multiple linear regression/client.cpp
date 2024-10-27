// client.cpp
#include <iostream>
#include <Eigen/Dense>
#include <asio.hpp>
#include <thread>

using asio::ip::tcp;
using namespace Eigen;
using namespace std;

void send_data(tcp::socket& socket, const MatrixXd& X, const VectorXd& Y) {
    int rows = X.rows();
    int cols = X.cols();
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(&cols, sizeof(int)));
    asio::write(socket, asio::buffer(X.data(), rows * cols * sizeof(double)));
    
    rows = Y.size();
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(Y.data(), rows * sizeof(double)));
}

void receive_coefficients(tcp::socket& socket, VectorXd& b) {
    int size;
    asio::read(socket, asio::buffer(&size, sizeof(int)));
    b.resize(size);
    asio::read(socket, asio::buffer(b.data(), size * sizeof(double)));
    cout << "Client: Received regression coefficients:\n" << b.transpose() << endl;
}

bool connect_to_party(tcp::socket& socket, const string& ip, int port) {
    for (int attempt = 0; attempt < 5; ++attempt) {
        asio::error_code ec;
        socket.connect(tcp::endpoint(asio::ip::address::from_string(ip), port), ec);
        if (!ec) {
            cout << "Client: Connected to " << ip << ":" << port << endl;
            return true;
        }
        cout << "Client: Failed to connect to " << ip << ":" << port << ", retrying... (attempt " << attempt + 1 << "/5)" << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cerr << "Client: Could not connect to " << ip << ":" << port << " after multiple attempts." << endl;
    return false;
}

int main() {
    asio::io_context io_context;

    // Setup sockets for Party0 and Party1
    tcp::socket socket_party0(io_context);
    tcp::socket socket_party1(io_context);

    // Retry connection to Party0 and Party1 until successful
    if (!connect_to_party(socket_party0, "127.0.0.1", 5000) || 
        !connect_to_party(socket_party1, "127.0.0.1", 5001)) {
        cerr << "Client: Exiting due to connection failure." << endl;
        return -1;
    }

    // Generate dummy data for X and Y
    int n = 10;  // Number of data points
    int d = 3;   // Number of features
    MatrixXd X = MatrixXd::Random(n, d);
    VectorXd Y = VectorXd::Random(n);

    // Split data into two parts
    int mid = n / 2;
    MatrixXd X_a = X.topRows(mid);
    VectorXd Y_a = Y.head(mid);
    MatrixXd X_b = X.bottomRows(n - mid);
    VectorXd Y_b = Y.tail(n - mid);

    // Send data to Party0
    cout << "Client: Sending data to Party0" << endl;
    send_data(socket_party0, X_a, Y_a);

    // Send data to Party1
    cout << "Client: Sending data to Party1" << endl;
    send_data(socket_party1, X_b, Y_b);

    cout << "Client: Data sent to both parties." << endl;

    // Accept connection from Party1 to receive regression coefficients
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5003));
    tcp::socket socket_for_coefficients(io_context);
    cout << "Client: Waiting to receive regression coefficients from Party1..." << endl;
    acceptor.accept(socket_for_coefficients);

    VectorXd b;
    receive_coefficients(socket_for_coefficients, b);

    // Example prediction
    VectorXd new_data_point = VectorXd::Random(d);
    double predicted_value = b.dot(new_data_point);
    cout << "Client: New data point for prediction: " << new_data_point.transpose() << endl;
    cout << "Client: Predicted value: " << predicted_value << endl;

    return 0;
}