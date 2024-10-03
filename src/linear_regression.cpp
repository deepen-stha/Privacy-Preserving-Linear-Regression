/*
Implementation of basic linear regression without privacy preserving
*/ 
#include <iostream>
#include <vector>

// Function to calculate the mean of a vector
double mean(const std::vector<double>& v) {
    double sum = 0;
    for (double value : v) {
        sum += value;
    }
    return sum / v.size();
}

// Function to calculate variance and covariance
double covariance(const std::vector<double>& x, const std::vector<double>& y, double x_mean, double y_mean) {
    double cov = 0;
    for (size_t i = 0; i < x.size(); i++) {
        cov += (x[i] - x_mean) * (y[i] - y_mean);
    }
    return cov;
}

double variance(const std::vector<double>& v, double mean) {
    double var = 0;
    for (double value : v) {
        var += (value - mean) * (value - mean);
    }
    return var;
}

// Function to perform linear regression and return coefficients
std::pair<double, double> linearRegression(const std::vector<double>& x, const std::vector<double>& y) {
    double x_mean = mean(x);
    double y_mean = mean(y);

    double cov = covariance(x, y, x_mean, y_mean);
    double var = variance(x, x_mean);

    double beta1 = cov / var;
    double beta0 = y_mean - beta1 * x_mean;

    return {beta0, beta1};
}

// Function to predict new value
double predict(double x, std::pair<double, double> coefficients) {
    return coefficients.first + coefficients.second * x;
}

int main() {
    std::vector<double> x = {1, 2, 3, 4, 5};
    std::vector<double> y = {2, 4, 5, 4, 5};

    // Perform linear regression
    std::pair<double, double> coefficients = linearRegression(x, y);
    std::cout << "The regression line is y = " << coefficients.first << " + " << coefficients.second << "x" << std::endl;

    // Predict new values
    double x_to_predict = 10;
    double y_predicted = predict(x_to_predict, coefficients);
    std::cout << "Predicted value of y for x = " << x_to_predict << " is " << y_predicted << std::endl;

    return 0;
}
