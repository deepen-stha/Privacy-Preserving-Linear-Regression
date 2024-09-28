#include <iostream>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <Eigen/Dense>  // Eigen for matrix operations

using namespace Eigen;
using namespace std;

class DataLoader {
public:
    MatrixXd loadCSV(const string &filePath, int rows, int cols, bool hasHeader = true) {
        MatrixXd data(rows, cols);
        ifstream file(filePath);
        string line, val;
        int row = 0, col = 0;

        // Skip the header if present
        if (hasHeader) {
            if (!getline(file, line)) {
                cerr << "Error: Unable to read header from file." << endl;
                exit(EXIT_FAILURE);
            }
        }

        while (getline(file, line)) {
            stringstream s(line);
            col = 0;

            while (getline(s, val, ',')) {
                try {
                    if (!val.empty()) {
                        data(row, col) = stod(val);
                    } else {
                        cerr << "Error: Empty value found at row " << row + 1 << ", col " << col + 1 << endl;
                        data(row, col) = 0.0; // Handle empty value case
                    }
                } catch (const invalid_argument& e) {
                    cerr << "Error: Invalid numeric value at row " << row + 1 << ", col " << col + 1 << ": " << val << endl;
                    data(row, col) = 0.0; // Set invalid values to 0.0
                } catch (const out_of_range& e) {
                    cerr << "Error: Numeric value out of range at row " << row + 1 << ", col " << col + 1 << ": " << val << endl;
                    exit(EXIT_FAILURE);
                }
                col++;
            }
            row++;
            if (row >= rows) break;  // Stop if we've reached the expected number of rows
        }

        if (row != rows) {
            cerr << "Warning: Expected " << rows << " rows, but read " << row << " rows from file." << endl;
        }

        return data;
    }
};

class NoiseGenerator {
public:
    vector<VectorXd> generateRandomNoise(int len_A, int numRows) {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 5);

        vector<VectorXd> noiseVectors;
        for (int i = 0; i < len_A; i++) {
            VectorXd noiseVec(numRows);
            for (int j = 0; j < numRows; j++) {
                noiseVec(j) = dis(gen);
            }
            noiseVectors.push_back(noiseVec);
        }
        return noiseVectors;
    }
};

class SecureLinearRegression {
public:
    MatrixXd XaTXa, XbTXb, XaTXb, XaTY;

    void computeLocalMultiplications(const MatrixXd &X_a, const MatrixXd &X_b) {
        if (X_a.cols() != X_b.cols()) {
            cerr << "Error: Matrix dimensions are incompatible for multiplication." << endl;
            exit(EXIT_FAILURE);
        }

        XaTXa = X_a.transpose() * X_a;
        XbTXb = X_b.transpose() * X_b;
        XaTXb = X_a.transpose() * X_b;
    }

    void addNoiseToData(MatrixXd &data, const vector<VectorXd> &randomNoise) {
        if (data.cols() != randomNoise.size()) {
            cerr << "Error: Noise dimensions do not match data dimensions." << endl;
            exit(EXIT_FAILURE);
        }

        for (size_t i = 0; i < data.cols(); i++) {
            if (data.rows() != randomNoise[i].size()) {
                cerr << "Error: Noise vector size does not match the data row size." << endl;
                exit(EXIT_FAILURE);
            }
            data.col(i) += randomNoise[i];
        }
    }

    MatrixXd computeSecureRegression(const MatrixXd &pp_XTX, const MatrixXd &pp_XTY, double lambda = 1e-5) {
        // Add regularization to avoid singularity (Ridge regularization)
        MatrixXd regularized_XTX = pp_XTX + lambda * MatrixXd::Identity(pp_XTX.rows(), pp_XTX.cols());

        // Check if the matrix is still singular
        if (regularized_XTX.determinant() == 0) {
            cerr << "Error: (X^T * X) matrix is still singular even after regularization and cannot be inverted." << endl;
            exit(EXIT_FAILURE);
        }

        return regularized_XTX.inverse() * pp_XTY;
    }
};

int main() {
    // Data loading
    DataLoader dataLoader;

    // Load datasets from CSV files
    MatrixXd myDataA = dataLoader.loadCSV("./data/preprocessed_dataFile_A.csv", 20, 5);  // 5 features for Party A
    MatrixXd myDataB = dataLoader.loadCSV("./data/preprocessed_dataFile_B.csv", 20, 5);  // 5 features for Party B

    cout << "--------------myDataA----------------" << endl;
    cout << myDataA << endl;

    cout << "--------------myDataB----------------" << endl;
    cout << myDataB << endl;
    
    cout << "myDataA dimensions: " << myDataA.rows() << " x " << myDataA.cols() << endl;
    cout << "myDataB dimensions: " << myDataB.rows() << " x " << myDataB.cols() << endl;

    // Add bias term (column of 1's) to both datasets to ensure consistent dimensions
    myDataA.conservativeResize(myDataA.rows(), myDataA.cols() + 1);
    myDataA.col(myDataA.cols() - 1) = VectorXd::Ones(myDataA.rows());

    myDataB.conservativeResize(myDataB.rows(), myDataB.cols() + 1);
    myDataB.col(myDataB.cols() - 1) = VectorXd::Ones(myDataB.rows());

    cout << "myDataA dimensions after adding bias: " << myDataA.rows() << " x " << myDataA.cols() << endl;
    cout << "myDataB dimensions after adding bias: " << myDataB.rows() << " x " << myDataB.cols() << endl;

    // Secure computation setup
    SecureLinearRegression regression;
    regression.computeLocalMultiplications(myDataA, myDataB);

    // Noise generation
    NoiseGenerator noiseGen;
    auto noises_A = noiseGen.generateRandomNoise(myDataA.cols(), myDataA.rows());
    auto noises_B = noiseGen.generateRandomNoise(myDataB.cols(), myDataB.rows());

    // Add noise to the data
    regression.addNoiseToData(myDataA, noises_A);
    regression.addNoiseToData(myDataB, noises_B);

    // Final computation of (X^T * X) and (X^T * Y)
    MatrixXd pp_XTX = regression.XaTXa + regression.XbTXb + regression.XaTXb.transpose();  // Combine Party A and B's XTX
    VectorXd pp_XTY = myDataA.transpose() * VectorXd::Ones(myDataA.rows()) + myDataB.transpose() * VectorXd::Ones(myDataB.rows());  // Y = ones for now

    // Perform secure regression with regularization
    MatrixXd result = regression.computeSecureRegression(pp_XTX, pp_XTY);

    // Print the results
    cout << "Coefficients:\n" << result.bottomRows(result.rows() - 1) << endl;  // Coefficients
    cout << "Intercept: " << result(0, 0) << endl;  // Intercept

    return 0;
}
