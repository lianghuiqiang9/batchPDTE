#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"pdte.h"
#include"utils.h"
using namespace std;

// g++ -o pdte_test -O3 pdte_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./pdte_test -i ./data/heart_11bits -s 10 

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
        //case 'd':d = atoi(optarg);break;
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
    auto result_cipher = pdte.evaluate(root, data_cipher, leaf_flatten, data);


    // decrypt
    auto result = pdte.recovery(result_cipher);

    // verify
    auto actural_result = root->eval(data);
    auto is_correct = pdte.verify(result, actural_result);


cout<< " pdte result a > b                      : "<< is_correct 
    <<endl;
}