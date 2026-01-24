#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"pdte.h"
using namespace std;

// g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lbpdte -Wl,-dpath,./lib

// ./pdte_main -i ../data/heart_11bits -c 1 -n 16 

int main(int argc, char* argv[]){

    string input_address;
    int data_rows = 2;
    int l = 8, m = 2, n = 16;    // make sure l * m or n >= data_bit_length
    int cmp_type = 0, bpdte_type = 0, extra = 0;
    int opt;
    while ((opt = getopt(argc, argv, "fi:d:l:m:n:c:e:p:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;  
        case 'd': data_rows = atoi(optarg); break;
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': n = atoi(optarg);break;
        case 'c': cmp_type = atoi(optarg);break;
        case 'e': extra = atoi(optarg);break;     
        case 'p': bpdte_type = atoi(optarg);break;    
        }
    }

    // bpdte setup
    auto pdte = std::make_unique<PDTE>();

    auto root = pdte->load_tree(input_address + "/model.json");
    root->print_tree();

    pdte->setup_cmp(cmp_type, l, m, n, extra);
    
    //auto leaf_flatten = pdte->encode_tree(root); 

    auto data = pdte->load_data(input_address + "/x_test.csv", data_rows);
    print_vec(data, pdte->data_cols,"data");

    auto data_cipher = pdte->encode_data(data);

}