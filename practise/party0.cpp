// party0.cpp
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

// Function to read data from a CSV file into an Eigen::MatrixXd
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

// Generate random noise matrix
MatrixXd generate_noise(int rows, int cols, int seed) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> dis(0, 5);
    MatrixXd noise(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            noise(i, j) = dis(gen);
    return noise;
}

// Send matrix and vector to Party1
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

// Receive matrix and vector from Party1
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
    MatrixXd data = read_csv("../data/Student_Performance_client1.csv");
    int n = data.rows();
    int d = data.cols() - 1;
    MatrixXd X_a = data.leftCols(d);
    VectorXd Y_a = data.col(d);

    // Local computation of XaTXa and XaTYa
    MatrixXd XaTXa = X_a.transpose() * X_a;
    VectorXd XaTYa = X_a.transpose() * Y_a;

    // Generate random noise matrix
    MatrixXd noise_matrix = generate_noise(d, d, 42);

    // Prepare secure scalar product data with noise
    MatrixXd XaTXb_noisy = X_a.transpose() * X_a + noise_matrix;

    // Send data to Party1 and receive
    tcp::socket socket(io_context);
    socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 5001));
    send_data(socket, XaTXa + noise_matrix, XaTYa);
    MatrixXd received_XTXb;
    VectorXd received_XTYb;
    receive_data(socket, received_XTXb, received_XTYb);

    // Final combined computation
    MatrixXd final_XTX = XaTXa + received_XTXb - noise_matrix;
    VectorXd final_XTY = XaTYa + received_XTYb;

    // Compute regression coefficients
    VectorXd b = final_XTX.inverse() * final_XTY;
    cout << "Party0: Final regression coefficients: " << b.transpose() << endl;

    // Prediction: Use the first row of X_a as an example
    VectorXd sample_data = X_a.row(0);  // Select the first row of features
    double predicted_value = b.dot(sample_data);
    cout << "Party0: Predicted value using first row: " << predicted_value << endl;

    return 0;
}
