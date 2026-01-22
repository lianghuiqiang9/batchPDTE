#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"cdcmp.h"
#include"tecmp.h"
#include"rdcmp.h"
#include"asm.h"
#include"esm.h"
using namespace std;

// g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./lib -lpdte -Wl,-rpath,./lib

// ./pdte_main -i ./data/heart_11bits -d 2048 -l 8 -m 2 -n 16 -c 0 -e 0 -p 0


int main(int argc, char* argv[]){
    if (argc < 2) {
        cout << "Need at least a model and data" << endl;
        return -1;
    }
    string input_address;
    int data_rows = 10;
    int l =8, m = 2, n = 16;    // make sure l * m or n >= data_bit_length
    int cmp_type = 0;
    //int d;
    int extra = 0;
    int pdte_type = 0;
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
        case 'p': pdte_type = atoi(optarg);break;    
        }
    }
    // pdte setup
    unique_ptr<PDTE> pdte;

    switch (pdte_type) {
        case 0: pdte = make_unique<ASM>(); break;
        case 1: pdte = make_unique<ESM>(); break;
    }

    auto root = pdte->load_tree(input_address + "/model.json");
    //root->print_tree();
    pdte->setup_cmp(cmp_type, l, m, n, extra);
    auto leaf_flatten = pdte->encode_tree(root); 

    auto data = pdte->load_data(input_address + "/x_test.csv", data_rows);
    //print_vec(data,pdte.data_cols,"data");
    auto data_cipher = pdte->encode_data(data);


    // evaluate
    vector<vector<Ciphertext>> result;
    auto finish = profile("PDTE", [&]() { result = pdte->evaluate(root, data_cipher, leaf_flatten);});
    pdte->clear_up(result);

    // recover
    auto expect_result = pdte->recover(result);

    // verify
    auto actural_result = root->eval(data);
    auto is_correct = pdte->verify(expect_result, actural_result);

    long comm = pdte->communication_cost(data_cipher, result);

    pdte->print();
    cout<< " pdte result is correct                   : "<< is_correct 
        << " \n input_address                            : "<<input_address
        << " \n data_rows                                : "<< data_rows
        << " \n overall time cost                        : "<< finish/1000     
        << " ms\n overall comm. cost                       : "<< comm/1024 
        << " kB\n amortized time cost                      : "<< finish/1024/data_rows 
        << " ms\n amortized comm. cost                     : "<< comm/1024 /data_rows 
        << " kB"<< endl;

}