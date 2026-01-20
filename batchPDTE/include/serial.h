#pragma once
#include "json.hpp"
#include<vector>
#include <fstream>
#include <iostream>

std::vector<std::vector<uint64_t>> load_matrix(std::string filename, int data_size){
    std::vector<std::vector<uint64_t>> data;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "open files error" << std::endl;
        return data;
    }
    std::string line;

    // read csv line by line
    int i=0;
    while(i<data_size && std::getline(file, line)){
        std::vector<uint64_t> row;
        std::stringstream lineStream(line);
        std::string cell;
        while (std::getline(lineStream, cell, ',')) {
            uint64_t cellValue;
            std::istringstream(cell) >> cellValue;
            row.push_back(cellValue);
        }

        data.push_back(row);
        i++;
    }
    return data;
}

void save_data(const std::vector<std::vector<uint64_t>>& data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << filename << std::endl;
        std::cout<<"Need mkdir the output file first "<<std::endl;
        exit(0);
        return;
    }
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
}


