#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"pdte.h"
using namespace std;

// g++ -o pdte_main -O3 pdte_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./build -lbpdte -Wl,-dpath,./lib

// ./pdte_main -i ../data/heart_11bits -l 1 -m 16

int main(int argc, char* argv[]){

    string input_address;
    int data_rows = 1;
    int l = 1, m = 8;    // make sure l * m or n >= data_bit_length
    int cmp_type = 1, pdte_type = 0, extra = 0;
    int opt;
    while ((opt = getopt(argc, argv, "fi:d:l:m:c:e:p:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;  
        case 'd': data_rows = atoi(optarg); break;
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'c': cmp_type = atoi(optarg);break;
        case 'e': extra = atoi(optarg);break;     
        case 'p': pdte_type = atoi(optarg);break;    
        }
    }

    unique_ptr<PDTE> pdte;
    switch (pdte_type) {
        case 0: pdte = make_unique<PDTE>(); break;
    }

    auto root = pdte->load_tree(input_address + "/model.json");
    root->print_tree();

    pdte->setup_cmp(cmp_type, l, m, extra);
    
    auto tree_flatten = pdte->encode_tree(root); 

    auto raw_data = pdte->load_data(input_address + "/x_test.csv", data_rows);

    for (int i = 0 ; i <raw_data.size(); i++){
        auto data = vector<vector<uint64_t>>{raw_data[i]};
            
        auto data_cipher = pdte->encode_data(data);

            // evaluate
        vector<vector<Ciphertext>> result;
        auto finish = profile("pdte", [&]() { 
            result = pdte->evaluate(root, data_cipher, tree_flatten);
        });
        auto expect_result = pdte->recover(result);
        auto actural_result = root->eval(data);
        cout<<" i: "<< i <<endl;
        cout<<"expect_result: "<< expect_result[0] <<endl;    
        cout<<"actural_result: "<< actural_result[0] <<endl;
        if (expect_result[0] != actural_result[0]){
            cout<<" i: "<< i <<endl;
            break;
        }
    }

/*
    auto data_cipher = pdte->encode_data(data);

        // evaluate
    vector<vector<Ciphertext>> result;
    auto finish = profile("pdte", [&]() { 
        result = pdte->evaluate(root, data_cipher, tree_flatten);
    });
    cout<<" * "<<endl;
    auto expect_result = pdte->recover(result);

    auto actural_result = root->eval(data);
    cout<<"expect_result: "<< expect_result[0] <<endl;    
    cout<<"actural_result: "<< actural_result[0] <<endl;

    auto is_correct = pdte->verify(expect_result, actural_result);
    long comm = pdte->comm_cost(data_cipher, result);

    pdte->print();
    cout<< " bpdte result is correct                  : "<< is_correct 
        << " \n input_address                            : "<<input_address
        << " \n data_rows                                : "<< data_rows
        << " \n keys size                                : "<< pdte->keys_size()/1024
        << " kB\n evaluate time cost                       : "<< finish/1000     
        << " ms\n evaluate comm. cost                      : "<< comm/1024
        << " kB"<< endl; 
*/
    return 0;
}