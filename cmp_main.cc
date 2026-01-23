#include"cdcmp.h"
#include"tecmp.h"
#include"rdcmp.h"
#include"utils.h"
#include<unistd.h>

// g++ -o cmp_main -O3 cmp_main.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1 -L ./lib -lpdte -Wl,-rpath,./lib

// ./cmp_main -m 8 -l 2 -n 16 -c 0

int main(int argc, char* argv[]) {
    int l = 8, m = 2, n = -1;
    int opt;
    int cmp_type = 0;
    unique_ptr<CMP> cmp;

    while ((opt = getopt(argc, argv, "fl:m:n:c:")) != -1) {
        switch (opt) {
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': n = atoi(optarg);break;
        case 'c': cmp_type = atoi(optarg);break;
        }
    }
    if ( n == -1 ){n = l * m;}

    switch (cmp_type) {
        case 0: cmp = make_unique<Tecmp>(l, m, n); break;
        case 1: cmp = make_unique<Cdcmp>(l, m, n); break;
        case 2: cmp = make_unique<Rdcmp>(l, m, n);break;
    }
    auto num_cmps = cmp->num_cmps;
    //auto raw_encode_a = cmp->random_raw_encode_a();
    //auto raw_encode_b = cmp->random_raw_encode_b();

    vector<vector<uint64_t>> raw_encode_a;
    vector<vector<uint64_t>> raw_encode_b;
    if ( n <= 64){
        auto a = random_k_bit(n, num_cmps);
        auto b = random_k_bit(n, num_cmps);

        print_vec(a, 10, "a: ");
        print_vec(b, 10, "b: ");

        raw_encode_a = cmp->raw_encode_a(a);
        raw_encode_b = cmp->raw_encode_b(b);
    }else{
        raw_encode_a = cmp->random_raw_encode_a();
        raw_encode_b = cmp->random_raw_encode_b();

        print_vec(raw_encode_a, 10, "raw_encode_a: ");
        print_vec(raw_encode_a, 10, "raw_encode_a: ");
    }


    auto cmp_encode_a = cmp->encode_a(raw_encode_a);
    auto cmp_encode_b = cmp->encode_b(raw_encode_b);
    auto cmp_encode_b_cipher = cmp->encrypt(cmp_encode_b);
    
    //cmp
    Ciphertext result;
    auto finish = profile("CMP", [&]() { result = cmp->great_than(cmp_encode_a, cmp_encode_b_cipher);});
    
    cmp->clear_up(result);
    auto expect_result = cmp->recover(result);
    auto actural_result = cmp->verify(raw_encode_a, raw_encode_b);

    print_vec(expect_result, 10, "expect_result ");
    print_vec(actural_result, 10, "actural_result");
    auto is_correct = cmp->verify(actural_result, expect_result);

    long comm = cmp->comm_cost(cmp_encode_b_cipher, result);

    cmp->print();
    cout<< " decrypt result a > b                     : "<< is_correct 
        << " \n keys size                                : "<< cmp->keys_size()/1024
        << " kB\n cmp time cost                            : "<< finish/1000     
        << " ms\n cmp comm. cost                           : "<< comm/1024 //
        << " kB\n amortized time cost                      : "<< (float)finish/1024/num_cmps 
        << " ms\n amortized comm. cost                     : "<< (float)comm/1024/num_cmps 
        << " kB" << endl;

}
