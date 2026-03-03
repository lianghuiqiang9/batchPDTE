#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"pdte.h"
#include"multipath.h"
#include"sumpath.h"
#include"sumpath2.h"
using namespace std;

// g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lbpdte -Wl,-dpath,./lib

// ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 0
// ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 1
// ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 1 -c 1
// ./pdte_main -i ../data/heart_11bits -n 11 -m 11 -d 10 -p 2 -c 1

int main(int argc, char* argv[]){

    string input_address;
    int data_rows = 1;
    int n = 16, m = 16;    // make sure n >= data_bit_length
    int cmp_type = 0, pdte_type = 0, extra = 0;
    int opt;
    while ((opt = getopt(argc, argv, "fi:d:n:m:h:c:e:p:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;  
        case 'd': data_rows = atoi(optarg); break;
        case 'n': n = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'h': m = atoi(optarg); break; // reuse the m.
        case 'c': cmp_type = atoi(optarg);break;
        case 'e': extra = atoi(optarg);break;     
        case 'p': pdte_type = atoi(optarg);break;    
        }
    }

    unique_ptr<PDTE> pdte;
    switch (pdte_type) {
        case 0: pdte = make_unique<SumPath>(); break;
        case 1: pdte = make_unique<SumPath2>(); break;
        case 2: pdte = make_unique<MultiPath>(); break;
    }

    auto root = pdte->load_tree(input_address + "/model.json");
    // root->print_tree();
    pdte->setup_cmp(cmp_type, n, m, extra);
    auto tree_flatten = pdte->encode_tree(root); 

    auto raw_data = pdte->load_data(input_address + "/x_test.csv", data_rows);

    float finish = 0;
    bool is_correct = true;
    long comm = 0;
    for (size_t i = 0 ; i <raw_data.size(); i++){
        auto data = vector<vector<uint64_t>>{raw_data[i]};
        auto data_cipher = pdte->encode_data(data);
        // evaluate
        vector<vector<Ciphertext>> result;
        finish += profile("pdte_row_" + std::to_string(i), [&]() { 
            result = pdte->evaluate(root, data_cipher, tree_flatten);
        });
        auto expect_result = pdte->recover(result);
        auto actural_result = root->eval(data);
        is_correct = is_correct && pdte->verify(expect_result, actural_result);
        comm += pdte->comm_cost(data_cipher, result);
        
        if (is_correct == false){
            cout << " i: "<< i << endl;
            break;
        }
    }

    pdte->print();
    cout<< " pdte result is correct                   : "<< is_correct 
        << " \n input_address                            : "<<input_address
        << " \n data_rows                                : "<< data_rows
        << " \n keys size                                : "<< (float)pdte->keys_size()/1024
        << " KB\n evaluate time cost                       : "<< (float)finish/1000     
        << " ms\n evaluate comm. cost                      : "<< (float)comm/1024 
        << " KB\n average time cost                        : "<< (float)finish/1000/data_rows 
        << " ms\n average comm. cost                       : "<< (float)comm/1024 /data_rows 
        << " KB"<< endl;


    return 0;
}