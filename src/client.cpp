/*
    @file src/client.cpp
    @brief
    @details
    @author Deepen Shrestha <deepens23@iitk.ac.in>
    @copyright Copyright (c) 2023-2025 Deepen Shrestha and others
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <asio.hpp>
#include <tuple>
#include <limits>

static constexpr uint16_t party0_port = 31337;
static constexpr uint16_t party1_port = 31338;

struct RegressionSums {
    double sum_x = 0.0;
    double sum_y = 0.0;
    double sum_xy = 0.0;
    double sum_x2 = 0.0;
    size_t n = 0;

    static RegressionSums parse(const std::string& response) {
        std::stringstream ss(response);
        RegressionSums sums;
        char comma;
        ss >> sums.sum_x >> comma >> sums.sum_y >> comma >> sums.sum_xy >> comma >> sums.sum_x2 >> comma >> sums.n;
        return sums;
    }

    static RegressionSums combine(const RegressionSums& a, const RegressionSums& b) {
        // std::cout << "Before combining:" << std::endl;
        // std::cout << "Party A - sum_x: " << a.sum_x << ", sum_y: " << a.sum_y << ", sum_xy: " << a.sum_xy << ", sum_x2: " << a.sum_x2 << ", n: " << a.n << std::endl;
        // std::cout << "Party B - sum_x: " << b.sum_x << ", sum_y: " << b.sum_y << ", sum_xy: " << b.sum_xy << ", sum_x2: " << b.sum_x2 << ", n: " << b.n << std::endl;

        RegressionSums combined;
        combined.sum_x = a.sum_x + b.sum_x;
        combined.sum_y = a.sum_y + b.sum_y;
        combined.sum_xy = a.sum_xy + b.sum_xy;
        combined.sum_x2 = a.sum_x2 + b.sum_x2;
        combined.n = a.n;  // Assuming n is the same from both parties

        // std::cout << "After combining:" << std::endl;
        // std::cout << "Combined - sum_x: " << combined.sum_x << ", sum_y: " << combined.sum_y << ", sum_xy: " << combined.sum_xy << ", sum_x2: " << combined.sum_x2 << ", n: " << combined.n << std::endl;

        return combined;
    }


   std::pair<double, double> computeCoefficients() const {
        double mean_x = sum_x / n;
        double mean_y = sum_y / n;
        double var_x = (sum_x2 / n) - (mean_x * mean_x);
        double cov_xy = (sum_xy / n) - (mean_x * mean_y);

        // std::cout << "Debugging computeCoefficients:" << std::endl;
        // std::cout << "  sum_x: " << sum_x << ", sum_y: " << sum_y << ", sum_xy: " << sum_xy << ", sum_x2: " << sum_x2 << ", n: " << n << std::endl;
        // std::cout << "  mean_x: " << mean_x << ", mean_y: " << mean_y << std::endl;
        // std::cout << "  var_x: " << var_x << ", cov_xy: " << cov_xy << std::endl;

        if (var_x == 0) {
            std::cerr << "Variance of x is zero, cannot compute slope." << std::endl;
            return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
        }

        double slope = cov_xy / var_x;
        double intercept = mean_y - slope * mean_x;

        std::cout << "  slope: " << slope << ", intercept: " << intercept << std::endl;

        return {intercept, slope};
    }
};

std::vector<int> read_csv(const std::string& filename) {
    std::vector<int> data;
    std::ifstream file(filename);
    std::string line;
    bool firstLine = true;  // Flag to skip the header

    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    while (std::getline(file, line)) {
        if (firstLine) {
            firstLine = false;
            continue;
        }
        std::stringstream linestream(line);
        std::string cell;
        bool firstCell = true;  // Flag to skip the first column

        while (std::getline(linestream, cell, ',')) {
            if (firstCell) {
                firstCell = false;
                continue;
            }
            try {
                data.push_back(std::stoi(cell));
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << cell << " in line (skipped)" << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Out of range: " << cell << " in line (skipped)" << std::endl;
            }
        }
    }
    return data;
}

void secret_share(const std::vector<int>& data, std::vector<int>& share1, std::vector<int>& share2) {
    std::mt19937 gen(42); // Consistent seed for debugging
    std::uniform_int_distribution<> dist(1, 1000); // Smaller range to control share size

    for (int value : data) {
        int share = dist(gen);
        share1.push_back(value + share); // Adjust to add a share
        share2.push_back(-share); // Negative of the share to balance the addition in share1
    }
}

void send_data(asio::ip::tcp::socket& socket, const std::vector<int>& data) {
    for (int value : data) {
        std::string message = std::to_string(value) + "\n";
        asio::write(socket, asio::buffer(message));
    }
}

void send_termination_signal(asio::ip::tcp::socket& socket) {
    std::string termination_signal = "END\n";
    asio::write(socket, asio::buffer(termination_signal));
}

void read_response(asio::ip::tcp::socket& socket, RegressionSums& sums) {
    asio::streambuf buffer;
    asio::read_until(socket, buffer, "\n");
    std::istream is(&buffer);
    std::string response;
    std::getline(is, response);
    std::cout << "Received response from server: " << response << std::endl;
    sums = RegressionSums::parse(response);
}

int main() {
    try {
        asio::io_context io_context;
        asio::ip::tcp::socket socket0(io_context), socket1(io_context);

        socket0.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), party0_port));
        socket1.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), party1_port));

        auto data = read_csv("./data/dataFile_a.csv");
        std::vector<int> share1, share2;
        secret_share(data, share1, share2);

        send_data(socket0, share1);
        send_data(socket1, share2);

        send_termination_signal(socket0);
        send_termination_signal(socket1);

        RegressionSums sums0, sums1;
        read_response(socket0, sums0);
        read_response(socket1, sums1);

        RegressionSums combined = RegressionSums::combine(sums0, sums1);
        auto [intercept, slope] = combined.computeCoefficients();
        std::cout << "Regression line: y = " << slope << "x + " << intercept << std::endl;

        double newX = 10;
        double predictedY = slope * newX + intercept;
        std::cout << "Predicted y for x = " << newX << ": " << predictedY << std::endl;

        socket0.close();
        socket1.close();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}
