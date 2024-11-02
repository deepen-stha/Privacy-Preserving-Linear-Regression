/*
    @file party1.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/
#include <iostream>
#include <Eigen/Dense>
#include <asio.hpp>

using asio::ip::tcp;
using namespace Eigen;
using namespace std;

void receive_data(tcp::socket& socket, MatrixXd& X, VectorXd& Y) {
    int rows, cols;
    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    asio::read(socket, asio::buffer(&cols, sizeof(int)));
    X.resize(rows, cols);
    asio::read(socket, asio::buffer(X.data(), rows * cols * sizeof(double)));
    // cout << "Party1: Received matrix X_b of size (" << rows << ", " << cols << "):\n" << X << endl;

    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    Y.resize(rows);
    asio::read(socket, asio::buffer(Y.data(), rows * sizeof(double)));
    // cout << "Party1: Received vector Y_b of size (" << rows << "):\n" << Y << endl;
}

void send_coefficients(tcp::socket& socket, const VectorXd& b) {
    int size = b.size();
    asio::write(socket, asio::buffer(&size, sizeof(int)));
    asio::write(socket, asio::buffer(b.data(), size * sizeof(double)));
    cout << "Party1: Sent regression coefficients to client." << endl;
}

int main() {
    asio::io_context io_context;

    // Accept connection from Client
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5001));
    tcp::socket socket(io_context);
    cout << "Party1: Waiting for Client connection..." << endl;
    acceptor.accept(socket);
    cout << "Party1: Client connected." << endl;

    MatrixXd X_b;
    VectorXd Y_b;

    // Receive data from Client
    receive_data(socket, X_b, Y_b);

    // Perform local computation
    MatrixXd XbTXb = X_b.transpose() * X_b;
    VectorXd XbTYb = X_b.transpose() * Y_b;
    cout << "Party1: Computed XbTXb and XbTYb." << endl;

    // Accept connection from Party0
    tcp::acceptor acceptor_party0(io_context, tcp::endpoint(tcp::v4(), 5002));
    tcp::socket socket_party0(io_context);
    cout << "Party1: Waiting for Party0 connection..." << endl;
    acceptor_party0.accept(socket_party0);
    cout << "Party1: Party0 connected." << endl;

    MatrixXd XaTXa;
    VectorXd XaTYa;

    // Receive computed results from Party0
    receive_data(socket_party0, XaTXa, XaTYa);

    // Combine results
    MatrixXd combined_XTX = XaTXa + XbTXb;
    VectorXd combined_XTY = XaTYa + XbTYb;
    cout << "Party1: Combined XTX and XTY for regression." << endl;

    // Solve for regression coefficients
    VectorXd b = combined_XTX.ldlt().solve(combined_XTY);
    cout << "Party1: Regression coefficients (b):\n" << b.transpose() << endl;

    // Connect to Client to send regression coefficients
    tcp::socket socket_to_client(io_context);
    socket_to_client.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5003));
    send_coefficients(socket_to_client, b);

    return 0;
}
