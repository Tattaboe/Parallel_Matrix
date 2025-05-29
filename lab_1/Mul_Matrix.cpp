#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <sstream>
#include <chrono>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

auto engine = std::mt19937(std::time(nullptr));

void create_directories() {
    fs::create_directory("matrices");
    fs::create_directory("results");
}

vector<vector<int>> generate(size_t size) {
    vector<vector<int>> matrix(size, vector<int>(size)); 
    for (auto& i : matrix) {
        for (int& j : i) {
            j = engine() % 100; 
        }
    }
    return matrix;
}

void write_to_file(const vector<vector<int>>& matrix, const string& path) {
    ofstream out(path);
    for (const auto& i : matrix) {
        for (int j : i) 
            out << j << " ";
        out << endl;
    }
}

vector<vector<int>> read_from_file(const string& path) {
    ifstream in(path);
    vector<vector<int>> matrix;
    string line;

    while (getline(in, line)) {
        istringstream iss(line);
        vector<int> row;
        int value;
        while (iss >> value) {
            row.push_back(value);
        }
        if (!row.empty()) {
            matrix.push_back(row);
        }
    }
    return matrix;
}

vector<vector<int>> mul_matrix(const vector<vector<int>>& matrix1, const vector<vector<int>>& matrix2) {
    size_t size = matrix1.size();
    vector<vector<int>> result(size, vector<int>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            for (int k = 0; k < size; ++k) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
    return result;
}

int main() {
    create_directories();

    vector<int> counts = {10, 30, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000};
    vector<double> times(counts.size(), 0.0);

    for (const auto& count : counts) {
        for (int i = 1; i < 3; ++i) {
            vector<vector<int>> matrix = generate(count);
            string path = "matrices/" + to_string(i) + "-" + to_string(count) + ".txt";
            write_to_file(matrix, path);
        }
    }

    for (size_t i = 0; i < counts.size(); ++i) {
        int count = counts[i];
        string path_1 = "matrices/1-" + to_string(count) + ".txt";
        string path_2 = "matrices/2-" + to_string(count) + ".txt";
        string result_path = "results/result-" + to_string(count) + ".txt";

        auto start_time = chrono::steady_clock::now();
        vector<vector<int>> matrix_1 = read_from_file(path_1);
        vector<vector<int>> matrix_2 = read_from_file(path_2);
        vector<vector<int>> result = mul_matrix(matrix_1, matrix_2);
        write_to_file(result, result_path);
        auto end_time = chrono::steady_clock::now();

        times[i] = chrono::duration<double, std::milli>(end_time - start_time).count();
    }

    ofstream out("results/stats.txt");
    for (size_t i = 0; i < counts.size(); ++i) {
        out << counts[i] << " " << times[i] << endl;
    }
}