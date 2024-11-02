#include <iostream>
#include <Eigen/Dense>
#include <Eigen/SVD>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace Eigen;
using namespace std;

// Function to trim whitespace from strings
string trim(const string& str) {
    const auto strBegin = str.find_first_not_of(" \t");
    const auto strEnd = str.find_last_not_of(" \t");
    const auto strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
}

// Function to read and partition data from a CSV file
void read_and_partition_data(const string& filename, MatrixXd& Xa, MatrixXd& Xb, VectorXd& Y, bool skip_header = true) {
    vector<double> hours_studied, previous_scores, performance_index;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Could not open the file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    bool is_first_line = true;
    while (getline(file, line)) {
        if (is_first_line && skip_header) {
            is_first_line = false;
            continue;
        }

        stringstream ss(line);
        string value;

        // Read each column
        getline(ss, value, ',');
        hours_studied.push_back(stod(trim(value)));

        getline(ss, value, ',');
        previous_scores.push_back(stod(trim(value)));

        getline(ss, value, ',');
        performance_index.push_back(stod(trim(value)));
    }
    file.close();

    int rows = hours_studied.size();
    Xa.resize(rows, 1);  // Alice's data (Hours Studied)
    Xb.resize(rows, 1);  // Bob's data (Previous Scores)
    Y.resize(rows);      // Target variable (Performance Index)

    for (int i = 0; i < rows; ++i) {
        Xa(i, 0) = hours_studied[i];
        Xb(i, 0) = previous_scores[i];
        Y(i) = performance_index[i];
    }
}

// Generate random matrix for masking
MatrixXd generateRandomMatrix(int rows, int cols) {
    MatrixXd mat(rows, cols);
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            mat(i, j) = rand() % 10; // Random values between 0 and 9
    return mat;
}

// Secure matrix multiplication with masking (Debugging: Masking disabled)
MatrixXd secureMatrixMultiply(const MatrixXd& A, const MatrixXd& B) {
    // For debugging, perform direct multiplication without masking
    cout << "Performing direct multiplication without masking for debugging." << endl;
    return A * B;
}

// Calculate secure regression coefficients using masked data
VectorXd secureRegressionCoefficients(const MatrixXd& Xa, const MatrixXd& Xb, const VectorXd& Y) {
    // Concatenate Xa and Xb horizontally to form the full feature matrix X
    MatrixXd X(Xa.rows(), Xa.cols() + Xb.cols());
    X.block(0, 0, Xa.rows(), Xa.cols()) = Xa;
    X.block(0, Xa.cols(), Xb.rows(), Xb.cols()) = Xb;

    MatrixXd XTX = secureMatrixMultiply(X.transpose(), X);
    VectorXd XTY = X.transpose() * Y;

    // Use pseudo-inverse to calculate regression coefficients for numerical stability
    JacobiSVD<MatrixXd> svd(XTX, ComputeThinU | ComputeThinV);
    return svd.solve(XTY); // b = pseudo-inverse(X^T * X) * X^T * Y
}

// Function to calculate regression coefficients directly (without masking)
VectorXd calculateDirectRegressionCoefficients(const MatrixXd& Xa, const MatrixXd& Xb, const VectorXd& Y) {
    // Concatenate Xa and Xb horizontally to form the full feature matrix X
    MatrixXd X(Xa.rows(), Xa.cols() + Xb.cols());
    X.block(0, 0, Xa.rows(), Xa.cols()) = Xa;
    X.block(0, Xa.cols(), Xb.rows(), Xb.cols()) = Xb;

    MatrixXd XTX = X.transpose() * X;
    VectorXd XTY = X.transpose() * Y;

    // Use pseudo-inverse for direct calculation as well
    JacobiSVD<MatrixXd> svd(XTX, ComputeThinU | ComputeThinV);
    return svd.solve(XTY); // b = pseudo-inverse(X^T * X) * X^T * Y
}

// Predict function for making predictions using secure coefficients
double securePredict(const VectorXd& coeffs, const VectorXd& sample_data_A, const VectorXd& sample_data_B) {
    // Combine Alice's and Bob's data parts for the prediction
    VectorXd sample_data(sample_data_A.size() + sample_data_B.size());
    sample_data << sample_data_A, sample_data_B;

    // Compute prediction as a dot product
    return coeffs.dot(sample_data);
}

// Function to calculate predictions given the coefficients
VectorXd predict(const MatrixXd& Xa, const MatrixXd& Xb, const VectorXd& coeffs) {
    MatrixXd X(Xa.rows(), Xa.cols() + Xb.cols());
    X.block(0, 0, Xa.rows(), Xa.cols()) = Xa;
    X.block(0, Xa.cols(), Xb.rows(), Xb.cols()) = Xb;
    return X * coeffs;
}

// Function to calculate Mean Squared Error
double meanSquaredError(const VectorXd& predictions, const VectorXd& Y) {
    return (predictions - Y).squaredNorm() / Y.size();
}

// Function to calculate R-Squared
double rSquared(const VectorXd& predictions, const VectorXd& Y) {
    double ss_tot = (Y.array() - Y.mean()).square().sum();
    double ss_res = (Y - predictions).squaredNorm();
    return 1 - (ss_res / ss_tot);
}

int main() {
    srand(time(0)); // Seed random generator

    // Load and partition data from Student_Performance.csv
    MatrixXd Xa, Xb;
    VectorXd Y;
    read_and_partition_data("../data/Student_Performance.csv", Xa, Xb, Y);

    // Calculate secure regression coefficients
    VectorXd b_secure = secureRegressionCoefficients(Xa, Xb, Y);
    cout << "Secure Regression coefficients: " << b_secure.transpose() << endl;

    // Calculate direct regression coefficients
    VectorXd b_direct = calculateDirectRegressionCoefficients(Xa, Xb, Y);
    cout << "Direct Regression coefficients: " << b_direct.transpose() << endl;

    // Check if the coefficients are close
    bool coefficients_match = (b_secure.isApprox(b_direct, 1e-6)); // Tolerance for floating-point comparison
    cout << "Do secure and direct coefficients match? " << (coefficients_match ? "Yes" : "No") << endl;

    // Calculate predictions using both secure and direct coefficients
    VectorXd predictions_secure = predict(Xa, Xb, b_secure);
    VectorXd predictions_direct = predict(Xa, Xb, b_direct);

    // Calculate Mean Squared Error (MSE) and R-Squared for both
    cout << "MSE (Secure): " << meanSquaredError(predictions_secure, Y) << endl;
    cout << "MSE (Direct): " << meanSquaredError(predictions_direct, Y) << endl;

    cout << "R-Squared (Secure): " << rSquared(predictions_secure, Y) << endl;
    cout << "R-Squared (Direct): " << rSquared(predictions_direct, Y) << endl;

    // Making a prediction with a new sample (for example, first row in Alice and Bob's datasets)
    VectorXd sample_data_A = Xa.row(0); // Alice's part of the sample data (Hours Studied)
    VectorXd sample_data_B = Xb.row(0); // Bob's part of the sample data (Previous Scores)
    double predicted_value = securePredict(b_secure, sample_data_A, sample_data_B);
    cout << "Predicted value for the sample: " << predicted_value << endl;

    return 0;
}
