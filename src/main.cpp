#include "../include/DataLoader.hpp"
#include "../include/NoiseGenerator.hpp"
#include "../include/SecureLinearRegression.hpp"

int main() {
    // Data loading
    DataLoader dataLoader;

    // Load datasets from CSV files
    MatrixXd myDataA = dataLoader.loadCSV("./data/preprocessed_dataFile_A.csv", 20, 5);  // 5 features for Party A
    MatrixXd myDataB = dataLoader.loadCSV("./data/preprocessed_dataFile_B.csv", 20, 5);  // 5 features for Party B

    // cout << "--------------myDataA----------------" << endl;
    // cout << myDataA << endl;

    // cout << "--------------myDataB----------------" << endl;
    // cout << myDataB << endl;
    
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

    // Extract coefficients and intercept
    VectorXd coefficients = result.bottomRows(result.rows() - 1);  // Coefficients (without intercept)
    double intercept = result(0, 0);  // Intercept (bias term)

    cout << "Coefficients dimensions: " << coefficients.rows() << " x " << coefficients.cols() << endl;
    cout << "Intercept: " << intercept << endl;

    // -------------- Create Temporary Data and Make Predictions ----------------
    // Create a synthetic data matrix with the same number of features as myDataA + myDataB
    MatrixXd tempData = MatrixXd::Random(myDataA.rows(), coefficients.rows());

    cout << "Temporary data dimensions: " << tempData.rows() << " x " << tempData.cols() << endl;

    // Make predictions using the temporary data
    // TODO: actual data will come here
    try {
        VectorXd predictions = tempData * coefficients + VectorXd::Ones(tempData.rows()) * intercept;
        cout << "Predictions:\n" << predictions << endl;
    } catch (const std::exception &e) {
        cerr << "Error during matrix multiplication: " << e.what() << endl;
    }

    return 0;
}
