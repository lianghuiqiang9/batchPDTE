#include<iostream>
#include<unistd.h>
#include<vector>
#include<random>
#include"node.h"
#include"utils.h"

using namespace std;


// g++ -o serial_test -O3 serial_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./serial_test -i ./data/heart_11bits -o ./data/heart_11bits_temp -d 16

int main(int argc, char* argv[]){
    string input_address;
    string output_address;
    int data_rows = 10;
    int opt;
    while ((opt = getopt(argc, argv, "ft:i:o:s:")) != -1) {
        switch (opt) {
        case 'i': input_address = string(optarg); break;
        case 'o': output_address = string(optarg); break;
        case 'd': data_rows = atoi(optarg); break;
        }
    }

    auto data = load_matrix(input_address + "/x_test.csv", data_rows);
    print_vec(data, 10, "data");
    save_data(data, output_address + "/x_test.csv");

    Node root = Node(input_address + "/model.json");
    root.print_tree();
    root.save_tree(output_address + "/model.json");

    Node root_temp = Node(output_address + "/model.json");
    root_temp.print_tree();

}