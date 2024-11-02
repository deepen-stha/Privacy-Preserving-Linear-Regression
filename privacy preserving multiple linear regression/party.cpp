/*
    @file src/party.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/
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
    
    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    Y.resize(rows);
    asio::read(socket, asio::buffer(Y.data(), rows * sizeof(double)));
}

void send_coefficients(tcp::socket& socket, const VectorXd& b) {
    int size = b.size();
    asio::write(socket, asio::buffer(&size, sizeof(int)));
    asio::write(socket, asio::buffer(b.data(), size * sizeof(double)));
}

int main() {
    asio::io_context io_context;

    // Accept connection from Client1 on port 5000
    tcp::acceptor acceptor1(io_context, tcp::endpoint(tcp::v4(), 5000));
    tcp::socket socket_client1(io_context);
    cout << "Party: Waiting for Client1 connection on port 5000..." << endl;
    acceptor1.accept(socket_client1);
    cout << "Party: Client1 connected." << endl;

    // Receive data from Client1
    MatrixXd X1;
    VectorXd Y1;
    receive_data(socket_client1, X1, Y1);
    cout << "Party: Received data from Client1." << endl;

    // Accept connection from Client2 on port 5001
    tcp::acceptor acceptor2(io_context, tcp::endpoint(tcp::v4(), 5001));
    tcp::socket socket_client2(io_context);
    cout << "Party: Waiting for Client2 connection on port 5001..." << endl;
    acceptor2.accept(socket_client2);
    cout << "Party: Client2 connected." << endl;

    // Receive data from Client2
    MatrixXd X2;
    VectorXd Y2;
    receive_data(socket_client2, X2, Y2);
    cout << "Party: Received data from Client2." << endl;

    // Combine data from both clients
    MatrixXd X(X1.rows() + X2.rows(), X1.cols());
    X << X1, X2;
    VectorXd Y(Y1.size() + Y2.size());
    Y << Y1, Y2;

    // Compute regression coefficients
    MatrixXd XTX = X.transpose() * X;
    VectorXd XTY = X.transpose() * Y;
    VectorXd b = XTX.ldlt().solve(XTY);
    cout << "Party: Computed regression coefficients:\n" << b.transpose() << endl;

    // Send coefficients back to both clients
    send_coefficients(socket_client1, b);
    cout << "Party: Sent coefficients to Client1." << endl;
    send_coefficients(socket_client2, b);
    cout << "Party: Sent coefficients to Client2." << endl;

    return 0;
}
