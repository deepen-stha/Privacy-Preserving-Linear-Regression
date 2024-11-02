/*
    @file client1.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/
#include <iostream>
#include <Eigen/Dense>
#include <asio.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>

using asio::ip::tcp;
using namespace Eigen;
using namespace std;

// Function to read data from a CSV file into an Eigen::MatrixXd
MatrixXd read_csv(const string& filename) {
    vector<vector<double>> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Could not open the file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    bool is_first_line = true;  // Flag to skip the header line

    while (getline(file, line)) {
        if (is_first_line) {
            is_first_line = false;  // Skip the header line
            continue;
        }

        stringstream ss(line);
        string value;
        vector<double> row;

        while (getline(ss, value, ',')) {
            row.push_back(stod(value));  // Convert each value to double
        }

        data.push_back(row);
    }
    file.close();

    // Convert the vector of vectors to Eigen::MatrixXd
    int rows = data.size();
    int cols = data[0].size();
    MatrixXd matrix(rows, cols);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            matrix(i, j) = data[i][j];

    return matrix;
}

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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: ./client <csv_file> <port>" << endl;
        return -1;
    }

    string filename = argv[1];
    int port = stoi(argv[2]);

    asio::io_context io_context;

    // Connect to Party
    tcp::socket socket_party(io_context);
    if (!connect_to_party(socket_party, "127.0.0.1", port)) {
        cerr << "Client: Exiting due to connection failure." << endl;
        return -1;
    }

    // Read data from CSV file
    MatrixXd data = read_csv(filename);
    cout << "Client: Data loaded from " << filename << endl;

    // Split data into X (features) and Y (target)
    int n = data.rows();
    int d = data.cols() - 1;  // Last column is assumed to be the target
    MatrixXd X = data.leftCols(d);  // All columns except the last one
    VectorXd Y = data.col(d);       // Last column as the target

    // Send data to Party
    cout << "Client: Sending data to Party" << endl;
    send_data(socket_party, X, Y);
    cout << "Client: Data sent to Party." << endl;

    // Wait to receive regression coefficients from Party
    VectorXd b;
    receive_coefficients(socket_party, b);

    // Use the first data point in X for prediction
    VectorXd new_data_point = X.row(0);  // First row of X as the prediction point
    double predicted_value = b.dot(new_data_point);
    cout << "Client: Using first data point for prediction: " << new_data_point.transpose() << endl;
    cout << "Client: Predicted value: " << predicted_value << endl;

    return 0;
}
