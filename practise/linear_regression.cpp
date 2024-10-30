#include <iostream>
#include <Eigen/Dense>
#include <fstream>
#include <sstream>
#include <vector>

using namespace Eigen;
using namespace std;

// Trim whitespace from the start and end of a string
string trim(const string& str) {
    const auto strBegin = str.find_first_not_of(" \t");
    const auto strEnd = str.find_last_not_of(" \t");
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

// Function to read data from a CSV file into an Eigen::MatrixXd
MatrixXd read_csv(const string& filename, VectorXd& Y, bool skip_header = true) {
    vector<vector<double>> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Could not open the file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    bool is_first_line = true;
    while (getline(file, line)) {
        if (is_first_line && skip_header) {
            // Skip the header row
            is_first_line = false;
            continue;
        }

        stringstream ss(line);
        string value;
        vector<double> row;

        while (getline(ss, value, ',')) {
            try {
                row.push_back(stod(trim(value))); // Convert each trimmed value to double
            } catch (const invalid_argument& e) {
                cerr << "Error: Non-numeric data found in the file. Check if there is a header or incorrect value." << endl;
                exit(EXIT_FAILURE);
            }
        }

        // Only add rows with the correct number of columns (for incomplete rows)
        if (!row.empty()) {
            data.push_back(row);
        }
    }
    file.close();

    // Convert the vector of vectors to Eigen::MatrixXd
    int rows = data.size();
    int cols = data[0].size() - 1; // Last column is the target variable
    MatrixXd X(rows, cols);
    Y.resize(rows);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            X(i, j) = data[i][j];
        }
        Y(i) = data[i][cols]; // Last column is Y
    }

    return X;
}

VectorXd calculate_regression_coefficients(const MatrixXd& X, const VectorXd& Y) {
    MatrixXd XTX = X.transpose() * X;
    VectorXd XTY = X.transpose() * Y;
    return XTX.inverse() * XTY; // b = (X^T * X)^-1 * X^T * Y
}

int main() {
    string filename = "../data/Student_Performance.csv"; // Replace with the path to your CSV file
    VectorXd Y;
    MatrixXd X = read_csv(filename, Y);

    // Calculate regression coefficients
    VectorXd b = calculate_regression_coefficients(X, Y);
    cout << "Regression coefficients: " << b.transpose() << endl;

    // Make a prediction using the first row of X
    VectorXd sample_data = X.row(0);
    double predicted_value = b.dot(sample_data);
    cout << "Predicted value for the first row: " << predicted_value << endl;

    return 0;
}
