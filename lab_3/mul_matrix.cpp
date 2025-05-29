#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <mpi.h>

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

vector<vector<int>> mul_matrix_mpi(const vector<vector<int>>& matrix1, 
                                 const vector<vector<int>>& matrix2,
                                 int rank, int size, int matrix_size) {
    vector<vector<int>> result(matrix_size, vector<int>(matrix_size, 0));

    int rows_per_process = matrix_size / size;
    int remainder = matrix_size % size;
    
    int start_row = rank * rows_per_process + min(rank, remainder);
    int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
    

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < matrix_size; ++j) {
            for (int k = 0; k < matrix_size; ++k) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
   
    if (size > 1) {
        if (rank == 0) {
          
            for (int p = 1; p < size; ++p) {
                int p_rows = matrix_size / size + (p < matrix_size % size ? 1 : 0);
                for (int i = 0; i < p_rows; ++i) {
                    int row_idx = p * (matrix_size / size) + min(p, matrix_size % size) + i;
                    MPI_Recv(result[row_idx].data(), matrix_size, MPI_INT, 
                            p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
        } else {
           
            for (int i = start_row; i < end_row; ++i) {
                MPI_Send(result[i].data(), matrix_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    return result;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    vector<int> counts = {10, 30, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000};
    vector<double> times(counts.size(), 0.0);
    
    if (rank == 0) {
        create_directories();
        
        for (const auto& count : counts) {
            for (int i = 1; i < 3; ++i) {
                vector<vector<int>> matrix = generate(count);
                string path = "matrices/" + to_string(i) + "-" + to_string(count) + ".txt";
                write_to_file(matrix, path);
            }
        }
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    for (size_t idx = 0; idx < counts.size(); ++idx) {
        int count = counts[idx];
        string path_1 = "matrices/1-" + to_string(count) + ".txt";
        string path_2 = "matrices/2-" + to_string(count) + ".txt";
        string result_path = "results/result-" + to_string(count) + ".txt";
        
        double start_time = MPI_Wtime();
        
        vector<vector<int>> matrix_1, matrix_2;
        if (rank == 0) {
            matrix_1 = read_from_file(path_1);
            matrix_2 = read_from_file(path_2);
        }
        
        int matrix_size;
        if (rank == 0) {
            matrix_size = matrix_1.size();
        }
        MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        
        matrix_1.resize(matrix_size, vector<int>(matrix_size));
        matrix_2.resize(matrix_size, vector<int>(matrix_size));
        
        for (int i = 0; i < matrix_size; ++i) {
            MPI_Bcast(matrix_2[i].data(), matrix_size, MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        int rows_per_process = matrix_size / size;
        int remainder = matrix_size % size;
        
        for (int i = 0; i < matrix_size; ++i) {
            int target_rank = (i < remainder * (rows_per_process + 1)) 
                           ? i / (rows_per_process + 1) 
                           : remainder + (i - remainder * (rows_per_process + 1)) / rows_per_process;
            
            if (rank == target_rank) {
                if (rank != 0) {
                    MPI_Recv(matrix_1[i].data(), matrix_size, MPI_INT, 
                            0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            } else if (rank == 0) {
                MPI_Send(matrix_1[i].data(), matrix_size, MPI_INT, 
                        target_rank, 0, MPI_COMM_WORLD);
            }
        }
        
        vector<vector<int>> result = mul_matrix_mpi(matrix_1, matrix_2, rank, size, matrix_size);
        
        if (rank == 0) {
            double end_time = MPI_Wtime();
            times[idx] = (end_time - start_time) * 1000; 
            
            write_to_file(result, result_path);
        }
    }
    
    if (rank == 0) {
       
        ofstream out("results/stats_mpi.txt");
        for (size_t i = 0; i < counts.size(); ++i) {
            out << counts[i] << " " << times[i] << endl;
        }
    }
    
    MPI_Finalize();
    return 0;
}