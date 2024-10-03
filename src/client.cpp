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
        char comma; // to consume the commas
        ss >> sums.sum_x >> comma >> sums.sum_y >> comma >> sums.sum_xy >> comma >> sums.sum_x2 >> comma >> sums.n;
        return sums;
    }

    static RegressionSums combine(const RegressionSums& a, const RegressionSums& b) {
        RegressionSums combined;
        combined.sum_x = a.sum_x + b.sum_x;
        combined.sum_y = a.sum_y + b.sum_y;
        combined.sum_xy = a.sum_xy + b.sum_xy;
        combined.sum_x2 = a.sum_x2 + b.sum_x2;
        combined.n = a.n;  // Assuming n is the same from both parties
        return combined;
    }

    std::pair<double, double> computeCoefficients() const {
        double mean_x = sum_x / n;
        double mean_y = sum_y / n;
        double var_x = (sum_x2 / n) - (mean_x * mean_x);
        double cov_xy = (sum_xy / n) - (mean_x * mean_y);

        double slope = cov_xy / var_x;
        double intercept = mean_y - slope * mean_x;
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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, INT_MAX);

    for (int value : data) {
        int share = dist(gen);
        share1.push_back(share);
        share2.push_back(value - share);
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

        // Read responses and compute regression coefficients
        RegressionSums sums0, sums1;
        read_response(socket0, sums0);
        read_response(socket1, sums1);

        RegressionSums combined = RegressionSums::combine(sums0, sums1);
        auto [intercept, slope] = combined.computeCoefficients();
        std::cout << "Regression line: y = " << slope << "x + " << intercept << std::endl;

        // Example prediction
        double newX = 10;  // Example new x value
        double predictedY = slope * newX + intercept;
        std::cout << "Predicted y for x = " << newX << ": " << predictedY << std::endl;

        socket0.close();
        socket1.close();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}