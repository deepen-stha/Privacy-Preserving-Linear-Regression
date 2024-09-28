#ifndef DATALOADER_HPP
#define DATALOADER_HPP

#include <Eigen/Dense>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Eigen;
using namespace std;

class DataLoader {
public:
    MatrixXd loadCSV(const string &filePath, int rows, int cols, bool hasHeader = true) {
        MatrixXd data(rows, cols);
        ifstream file(filePath);

        // Print the file path being used
        cout << "Trying to open file: " << filePath << endl;

        // Check if file is open
        if (!file.is_open()) {
            cerr << "Error: Unable to open file at " << filePath << endl;
            exit(EXIT_FAILURE);
        }

        string line, val;
        int row = 0, col = 0;

        // Skip the header if present
        if (hasHeader) {
            if (!getline(file, line)) {
                cerr << "Error: Unable to read header from file: " << filePath << endl;
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

#endif // DATALOADER_HPP
