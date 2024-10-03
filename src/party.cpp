/*
    @file src/party.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/

#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#ifdef PARTY0
static constexpr uint16_t server_port = 31337;
const char* party_name = "Party0";
#else
static constexpr uint16_t server_port = 31338;
const char* party_name = "Party1";
#endif

struct RegressionSums {
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x2 = 0.0;
    size_t n = 0;

    void addShares(double x_share, double y_share) {
        sum_x += x_share;
        sum_y += y_share;
        sum_xy += x_share * y_share;
        sum_x2 += x_share * x_share;
        n++;
    }

    std::pair<double, double> computeRegressionCoefficients() const {
        if (n == 0) {
            std::cerr << "No data points to process." << std::endl;
            return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
        }
        double mean_x = sum_x / n;
        double mean_y = sum_y / n;
        double var_x = (sum_x2 / n) - (mean_x * mean_x);
        if (var_x == 0) {
            std::cerr << "Variance of x is zero, cannot compute slope." << std::endl;
            return {mean_y, std::numeric_limits<double>::infinity()};  // Assuming infinite slope if no variance in x
        }
        double cov_xy = (sum_xy / n) - (mean_x * mean_y);

        double beta1 = cov_xy / var_x;
        double beta0 = mean_y - beta1 * mean_x;

        return {beta0, beta1};
    }

    std::string serialize() const {
        std::stringstream ss;
        ss << sum_x << ',' << sum_y << ',' << sum_xy << ',' << sum_x2 << ',' << n;
        return ss.str();
    }
};

void handle_client(asio::ip::tcp::socket& socket) {
    try {
        std::cout << party_name << ": Connection accepted." << std::endl;
        RegressionSums sums;
        asio::streambuf buffer;
        std::string line;
        asio::error_code error;
        bool expect_x = true;
        double x_share, y_share;

        while (true) {
            size_t len = asio::read_until(socket, buffer, "\n", error);
            if (error) {
                if (error == asio::error::eof) {
                    std::cout << party_name << ": End of data stream. Processing data." << std::endl;
                    break;
                } else {
                    throw asio::system_error(error);
                }
            }

            std::istream is(&buffer);
            while (std::getline(is, line)) {
                if (line == "END") {
                    std::cout << party_name << ": End of data signal received." << std::endl;
                    break;
                }

                if (!line.empty()) {
                    std::istringstream iss(line);
                    if (expect_x) {
                        if (iss >> x_share) {
                            expect_x = false;
                        }
                    } else {
                        if (iss >> y_share) {
                            sums.addShares(x_share, y_share);
                            std::cout << party_name << ": Added x_share=" << x_share << ", y_share=" << y_share << std::endl;
                            expect_x = true;
                        }
                    }
                }
            }
            if (line == "END") break;
        }

        auto coefficients = sums.computeRegressionCoefficients();
        std::cout << party_name << ": Regression line: y = " << coefficients.first << " + " << coefficients.second << "x" << std::endl;

        std::string message = sums.serialize() + "\n";
        asio::write(socket, asio::buffer(message));
        std::cout << party_name << ": Computed sums and regression coefficients sent." << std::endl;
    } catch (std::exception& e) {
        std::cerr << party_name << " Exception: " << e.what() << std::endl;
    }
}

int main() {
    try {
        asio::io_context io_context;
        asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), server_port));
        std::cout << party_name << ": Listening on port " << server_port << std::endl;
        asio::ip::tcp::socket socket(io_context);
        acceptor.accept(socket);
        handle_client(socket);
    } catch (std::exception& e) {
        std::cerr << party_name << " Main exception: " << e.what() << std::endl;
    }

    return 0;
}

