/*
    @file party1.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/
#include <iostream>
#include <thread>
#include <asio.hpp>
#include <Eigen/Dense>
#include <fstream>
#include <sstream>
#include <vector>
#include <chrono>
#include <random>

using namespace Eigen;
using asio::ip::tcp;
using namespace std;

MatrixXd read_csv(const string& filename) {
    vector<vector<double>> data;
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open the file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    string line;
    bool is_first_line = true;
    while (getline(file, line)) {
        if (is_first_line) {
            is_first_line = false;
            continue;
        }
        stringstream ss(line);
        string value;
        vector<double> row;
        while (getline(ss, value, ',')) {
            row.push_back(stod(value));
        }
        data.push_back(row);
    }
    file.close();

    int rows = data.size();
    int cols = data[0].size();
    MatrixXd matrix(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            matrix(i, j) = data[i][j];
    return matrix;
}

MatrixXd generate_noise(int rows, int cols, int seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 5);
    MatrixXd noise(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            noise(i, j) = dis(gen);
    return noise;
}

void send_data(tcp::socket& socket, const MatrixXd& matrix, const VectorXd& vector) {
    int rows = matrix.rows();
    int cols = matrix.cols();
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(&cols, sizeof(int)));
    asio::write(socket, asio::buffer(matrix.data(), rows * cols * sizeof(double)));

    rows = vector.size();
    asio::write(socket, asio::buffer(&rows, sizeof(int)));
    asio::write(socket, asio::buffer(vector.data(), rows * sizeof(double)));
}

void receive_data(tcp::socket& socket, MatrixXd& matrix, VectorXd& vector) {
    int rows, cols;
    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    asio::read(socket, asio::buffer(&cols, sizeof(int)));
    matrix.resize(rows, cols);
    asio::read(socket, asio::buffer(matrix.data(), rows * cols * sizeof(double)));

    asio::read(socket, asio::buffer(&rows, sizeof(int)));
    vector.resize(rows);
    asio::read(socket, asio::buffer(vector.data(), rows * sizeof(double)));
}

int main() {
    asio::io_context io_context;
    MatrixXd data = read_csv("../data/Student_Performance_client2.csv");
    int n = data.rows();
    int d = data.cols() - 1;
    MatrixXd X_b = data.leftCols(d);
    VectorXd Y_b = data.col(d);

    // Local computation of XbTXb and XbTYb
    MatrixXd XbTXb = X_b.transpose() * X_b;
    VectorXd XbTYb = X_b.transpose() * Y_b;

    // Generate random noise matrix
    MatrixXd noise_matrix = generate_noise(d, d, 42);

    // Prepare secure scalar product data with noise
    MatrixXd XbTXa_noisy = X_b.transpose() * X_b + noise_matrix;

    // Listen for Party0
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5001));
    tcp::socket socket(io_context);
    acceptor.accept(socket);

    // Receive data from Party0
    MatrixXd received_XTXa;
    VectorXd received_XTYa;
    receive_data(socket, received_XTXa, received_XTYa);

    // Send data to Party0
    send_data(socket, XbTXb + noise_matrix, XbTYb);

    // Final combined computation
    MatrixXd final_XTX = XbTXb + received_XTXa - noise_matrix;
    VectorXd final_XTY = XbTYb + received_XTYa;

    // Compute regression coefficients
    VectorXd b = final_XTX.inverse() * final_XTY;
    cout << "Party1: Final regression coefficients: " << b.transpose() << endl;

    // Prediction: Use the first row of X_b as an example
    VectorXd sample_data = X_b.row(0);  // Select the first row of features
    double predicted_value = b.dot(sample_data);
    cout << "Party1: Predicted value using first row: " << predicted_value << endl;

    return 0;
}
