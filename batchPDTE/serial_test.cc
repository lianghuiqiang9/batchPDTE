#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"serial.h"
#include"node.h"
#include"utils.h"

using namespace std;

/*
std::vector<uint64_t> generate_k_bit_random_batch(size_t count, int k) {
    if (k < 1) return std::vector<uint64_t>(count, 0);
    if (k > 64) k = 64;

    static std::random_device rd;
    static std::mt19937_64 gen(rd());

    std::vector<uint64_t> results;
    results.reserve(count); 

    if (k == 64) {
        for (size_t i = 0; i < count; ++i) {
            results.push_back(gen());
        }
    } else {
        uint64_t max_val = (1ULL << k) - 1;
        std::uniform_int_distribution<uint64_t> dis(0, max_val);
        for (size_t i = 0; i < count; ++i) {
            results.push_back(dis(gen));
        }
    }

    return results;
}
*/

// g++ -o serial_test -O3 serial_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./serial_test -i ./data/heart_11bits -o ./data/heart_11bits_temp -s 16

int main(int argc, char* argv[]){
    string input_address;
    string output_address;
    int data_size = 10;
    int opt;
    while ((opt = getopt(argc, argv, "ft:i:o:s:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;
        case 'o': output_address = string(optarg); break;
        case 's': data_size = atoi(optarg); break;
        }
    }

    auto data = load_matrix(input_address + "/x_test.csv", data_size);
    print(data, 10, "data");
    save_data(data, output_address + "/x_test.csv");

    Node root = Node(input_address + "/model.json");
    root.print_tree();
    root.save_tree(output_address + "/model.json");

    Node root_temp = Node(output_address + "/model.json");
    root_temp.print_tree();

}