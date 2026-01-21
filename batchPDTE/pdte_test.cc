#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"pdte.h"
using namespace std;

// g++ -o pdte_test -O3 pdte_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./pdte_test -i ./data/heart_11bits -s 2048 -t 0 -l 8 -m 2 -n 16 -e 0


int main(int argc, char* argv[]){
    if (argc < 2) {
        cout << "Need at least a model and data" << endl;
        return -1;
    }
    string input_address;
    int data_size = 10;
    int l, m, n;    // make sure l * m or n >= data_bit_length
    int cmp_type = 0;
    //int d;
    int extra = 0;

    int opt;
    while ((opt = getopt(argc, argv, "fi:s:l:m:n:t:e:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;  
        case 's': data_size = atoi(optarg); break;
        case 't': cmp_type = atoi(optarg);break;
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': n = atoi(optarg);break;
        case 'e':extra = atoi(optarg);break;       
        }
    }
    // pdte setup
    PDTE pdte;

    auto root = pdte.load_tree(input_address + "/model.json");
    //root->print_tree();
    pdte.setup_cmp(cmp_type, l, m, n, extra);
    auto leaf_flatten = pdte.encode_tree(root); 

    auto data = pdte.load_data(input_address + "/x_test.csv", data_size);
    //print(data,pdte.data_cols,"data");
    auto data_cipher = pdte.encode_data(data);


    // evaluate
    Ciphertext result;
    auto finish = profile("PDTE", [&]() { result = pdte.evaluate(root, data_cipher, leaf_flatten, data);});
    pdte.clear_up(result);

    // recover
    auto expect_result = pdte.recover(result);

    // verify
    auto actural_result = root->eval(data);
    auto is_correct = pdte.verify(expect_result, actural_result);

    long comm = pdte.communication_cost(data_cipher, result);

    pdte.print();
    cout<< " pdte result is correct                   : "<< is_correct 
        << " \n input_address                            : "<<input_address
        << " \n data_size                                : "<< data_size
        << " \n overall time cost                        : "<< finish/1000     
        << " ms\n overall comm. cost                       : "<< comm/1024 
        << " kB\n amortized time cost                      : "<< finish/1024/data_size 
        << " ms\n amortized comm. cost                     : "<< comm/1024 /data_size <<" kB"
        << endl;

}