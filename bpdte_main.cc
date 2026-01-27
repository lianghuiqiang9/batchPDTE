#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"dcmp.h"
#include"tcmp.h"
#include"asm.h"
#include"esm.h"
using namespace std;

// g++ -o bpdte_main -O3 bpdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lbpdte -Wl,-dpath,./lib

// ./bpdte_main -i ../data/heart_11bits -d 128 -l 1 -m 16

int main(int argc, char* argv[]){

    string input_address;
    int data_rows = 10;
    int l = 8, m = 2;    // make sure l * m or n >= data_bit_length
    int cmp_type = 0, bpdte_type = 0, extra = 0;
    int opt;
    while ((opt = getopt(argc, argv, "fi:d:l:m:c:e:p:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;  
        case 'd': data_rows = atoi(optarg); break;
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'c': cmp_type = atoi(optarg);break;
        case 'e': extra = atoi(optarg);break;     
        case 'p': bpdte_type = atoi(optarg);break;    
        }
    }

    // bpdte setup
    unique_ptr<BPDTE> bpdte;
    switch (bpdte_type) {
        case 0: bpdte = make_unique<ASM>(); break;
        case 1: bpdte = make_unique<ESM>(); break;
    }

    auto root = bpdte->load_tree(input_address + "/model.json");
    // root->print_tree();
    bpdte->setup_cmp(cmp_type, l, m, extra);
    auto leaf_flatten = bpdte->encode_tree(root); 

    auto data = bpdte->load_data(input_address + "/x_test.csv", data_rows);
    //print_vector(data,bpdte.data_cols,"data");
    auto data_cipher = bpdte->encode_data(data);

    // evaluate
    vector<vector<Ciphertext>> result;
    auto finish = profile("bpdte", [&]() { 
        result = bpdte->evaluate(root, data_cipher, leaf_flatten);
        bpdte->clear_up(result);
    });

    // recover
    auto expect_result = bpdte->recover(result);

    // verify
    auto actural_result = root->eval(data);
    auto is_correct = bpdte->verify(expect_result, actural_result);

    long comm = bpdte->comm_cost(data_cipher, result);
    //long comm = bpdte->comm_cost_estimate(data_cipher, result);

    bpdte->print();
    cout<< " bpdte result is correct                  : "<< is_correct 
        << " \n input_address                            : "<<input_address
        << " \n data_rows                                : "<< data_rows
        << " \n keys size                                : "<< bpdte->keys_size()/1024
        << " kB\n evaluate time cost                       : "<< finish/1000     
        << " ms\n evaluate comm. cost                      : "<< comm/1024 
        << " kB\n amortized time cost                      : "<< finish/1024/data_rows 
        << " ms\n amortized comm. cost                     : "<< comm/1024 /data_rows 
        << " kB"<< endl;

}