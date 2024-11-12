/*
    @file src/party.cpp
    @brief Secure two-party linear regression with Du-Atallah multiplication.
    @details This file represents one of two parties.
    @authored by Deepen Shrestha <deepens23@iitk.ac.in>
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

    void addShares(double x_share, double y_share, double u, double v) {
        sum_x += x_share;
        sum_y += y_share;

        // Du-Atallah multiplication using synchronized random shares u and v
        #ifdef PARTY0
        sum_xy += (x_share + u) * (y_share + v) - x_share * v - y_share * u;
        sum_x2 += (x_share + u) * (x_share + u) - x_share * u - x_share * u;
        #else
        sum_xy += x_share * v + y_share * u;
        sum_x2 += x_share * u + x_share * u;
        #endif

        n++;
        std::cout << party_name << ": sum_x=" << sum_x << ", sum_y=" << sum_y
                  << ", sum_xy=" << sum_xy << ", sum_x2=" << sum_x2 << ", n=" << n << std::endl;
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
        double x_share, y_share, u, v;

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

                std::istringstream iss(line);
                iss >> x_share >> y_share >> u >> v;
                sums.addShares(x_share, y_share, u, v);
            }
            if (line == "END") break;
        }

        std::string message = sums.serialize() + "\n";
        asio::write(socket, asio::buffer(message));
        std::cout << party_name << ": Computed sums sent to client." << std::endl;
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
