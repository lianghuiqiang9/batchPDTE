#include"dcmp.h"
#include"tcmp.h"
#include"utils.h"
#include<unistd.h>

// g++ -o cmp_main -O3 cmp_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./lib -lbpdte -Wl,-rpath,./lib

// ./cmp_main -l 2 -m 8 -c 0

int main(int argc, char* argv[]) {
    int l = 8, m = 2;
    int opt;
    int cmp_type = 0;
    unique_ptr<CMP> cmp;

    while ((opt = getopt(argc, argv, "fl:m:c:")) != -1) {
        switch (opt) {
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'c': cmp_type = atoi(optarg);break;
        }
    }
    int n = l * m;

    switch (cmp_type) {
        case 0: cmp = make_unique<TCMP>(l, m); break;
        case 1: cmp = make_unique<DCMP>(l, m); break;
    }
    
    auto num_cmps = cmp->num_cmps;

    vector<vector<uint64_t>> raw_encode_a;
    vector<vector<uint64_t>> raw_encode_b;
    if ( n <= 64){
        auto a = random_k_bit(n, num_cmps);
        auto b = random_k_bit(n, num_cmps);

        print_vector(a, 10, "a: ");
        print_vector(b, 10, "b: ");

        raw_encode_a = cmp->raw_encode_a(a);
        raw_encode_b = cmp->raw_encode_b(b);
    }else{
        raw_encode_a = cmp->random_raw_encode_a();
        raw_encode_b = cmp->random_raw_encode_b();

        print_matrix(raw_encode_a, 10, 10, "raw_encode_a: ");
        print_matrix(raw_encode_a, 10, 10, "raw_encode_a: ");
    }

    auto cmp_encode_a = cmp->encode_a(raw_encode_a);
    auto cmp_encode_b = cmp->encode_b(raw_encode_b);
    auto cmp_encode_b_cipher = cmp->encrypt(cmp_encode_b);

    //cmp
    Ciphertext result;
    auto finish = profile("CMP", [&]() { 
        result = cmp->great_than(cmp_encode_a, cmp_encode_b_cipher);
        cmp->clear_up(result);
    });
    
    auto expect_result = cmp->recover(result);
    auto actural_result = cmp->verify(raw_encode_a, raw_encode_b);

    print_vector(expect_result, 10, "expect_result ");
    print_vector(actural_result, 10, "actural_result");
    auto is_correct = cmp->verify(actural_result, expect_result);

    long comm = cmp->comm_cost(cmp_encode_b_cipher, result);

    cmp->print();
    cout<< " decrypt result a > b                     : "<< is_correct 
        << " \n keys size                                : "<< cmp->keys_size()/1024
        << " kB\n batch nums                               : "<< num_cmps
        << " \n cmp time cost                            : "<< finish/1000     
        << " ms\n cmp comm. cost                           : "<< comm/1024 //
        << " kB\n amortized time cost                      : "<< (float)finish/1024/num_cmps 
        << " ms\n amortized comm. cost                     : "<< (float)comm/1024/num_cmps 
        << " kB" << endl;
}
