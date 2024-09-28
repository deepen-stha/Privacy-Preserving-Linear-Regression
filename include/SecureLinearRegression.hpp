#ifndef SECURELINEARREGRESSION_HPP
#define SECURELINEARREGRESSION_HPP

#include <Eigen/Dense>
#include <vector>

using namespace Eigen;
using namespace std;

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

#endif // SECURELINEARREGRESSION_HPP
