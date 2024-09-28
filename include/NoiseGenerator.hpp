#ifndef NOISEGENERATOR_HPP
#define NOISEGENERATOR_HPP

#include <Eigen/Dense>
#include <vector>
#include <random>

using namespace Eigen;
using namespace std;

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

#endif // NOISEGENERATOR_HPP
