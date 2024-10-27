// party0.cpp
#include <iostream>
#include <Eigen/Dense>
#include <asio.hpp>
#include <thread>

using asio::ip::tcp;
using namespace Eigen;
using namespace std;

void receive_data(tcp::socket& socket, MatrixXd& X, VectorXd& Y) {
    int rows, cols;
    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    asio::read(socket, asio::buffer(&cols, sizeof(int)));
    X.resize(rows, cols);
    asio::read(socket, asio::buffer(X.data(), rows * cols * sizeof(double)));
    
    // Print received matrix X
    cout << "Party0: Received matrix X_a of size (" << rows << ", " << cols << "):\n" << X << endl;

    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    Y.resize(rows);
    asio::read(socket, asio::buffer(Y.data(), rows * sizeof(double)));
    
    // Print received vector Y
    cout << "Party0: Received vector Y_a of size (" << rows << "):\n" << Y << endl;
}

void send_data(tcp::socket& socket, const MatrixXd& X, const VectorXd& Y) {
    int rows = X.rows();
    int cols = X.cols();
    cout << "Party0: Sending computed XaTXa and XaTYa to Party1." << endl;
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(&cols, sizeof(int)));
    asio::write(socket, asio::buffer(X.data(), rows * cols * sizeof(double)));
    
    rows = Y.size();
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(Y.data(), rows * sizeof(double)));
}

bool connect_to_party1(tcp::socket& socket) {
    for (int attempt = 0; attempt < 5; ++attempt) {
        asio::error_code ec;
        socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5002), ec);
        if (!ec) {
            cout << "Party0: Connected to Party1 on port 5002" << endl;
            return true;
        }
        cout << "Party0: Failed to connect to Party1, retrying... (attempt " << attempt + 1 << "/5)" << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
    cerr << "Party0: Could not connect to Party1 after multiple attempts." << endl;
    return false;
}

int main() {
    asio::io_context io_context;

    // Accept connection from Client
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5000));
    tcp::socket socket(io_context);
    cout << "Party0: Waiting for Client connection..." << endl;
    acceptor.accept(socket);
    cout << "Party0: Client connected." << endl;

    MatrixXd X_a;
    VectorXd Y_a;

    // Receive data from Client
    receive_data(socket, X_a, Y_a);

    // Perform local computation
    MatrixXd XaTXa = X_a.transpose() * X_a;
    VectorXd XaTYa = X_a.transpose() * Y_a;
    cout << "Party0: Computed XaTXa:\n" << XaTXa << endl;
    cout << "Party0: Computed XaTYa:\n" << XaTYa << endl;

    // Connect to Party1
    tcp::socket socket_party1(io_context);
    if (!connect_to_party1(socket_party1)) {
        cerr << "Party0: Exiting due to failure to connect to Party1." << endl;
        return -1;
    }

    // Send computed results to Party1
    send_data(socket_party1, XaTXa, XaTYa);
    cout << "Party0: Sent computed data to Party1." << endl;

    return 0;
}
