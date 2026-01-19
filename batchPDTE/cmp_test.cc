#include"cdcmp.h"
#include"tecmp.h"
#include"rdcmp.h"
#include"utils.h"
#include<unistd.h>

// g++ -o cmp_test -O3 cmp_test.cc -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./cmp_test -m 8 -l 2 -n 16 -t 0

int main(int argc, char* argv[]) {
    int l,m,n; 
    int opt;
    int cmp_name = 0;
    unique_ptr<CMP> cmp;

    while ((opt = getopt(argc, argv, "fl:m:n:t:")) != -1) {
        switch (opt) {
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': n = atoi(optarg);break;
        case 't': cmp_name = atoi(optarg);break;
        }
    }
    
    switch (cmp_name) {
        case 0: cmp = make_unique<Tecmp>(l, m); break;
        case 1: cmp = make_unique<Cdcmp>(n); break;
        case 2: cmp = make_unique<Rdcmp>(n);break;
    }

    //unique_ptr<CMP> cmp = make_unique<Rdcmp>(n);
    //unique_ptr<CMP> cmp = make_unique<Cdcmp>(n);
    //unique_ptr<CMP> cmp = make_unique<Tecmp>(l, m);

    auto num_cmps = cmp->num_cmps;
    auto raw_encode_a = cmp->random_raw_encode_a();
    auto raw_encode_b = cmp->random_raw_encode_b();
    print(raw_encode_a, 10, "a: ");
    print(raw_encode_b, 10, "b: ");

    auto cmp_encode_a = cmp->encode_a(raw_encode_a);
    auto cmp_encode_b = cmp->encode_b(raw_encode_b);
    auto cmp_encode_b_cipher = cmp->encrypt(cmp_encode_b);
    
    //cmp
    Ciphertext result;
    auto finish = profile("Cmp", [&]() { result = cmp->great_than(cmp_encode_a, cmp_encode_b_cipher);});
    
    cmp->clear_up(result);
    auto expect_result = cmp->decode(result);
    auto actural_result = cmp->verify(raw_encode_a, raw_encode_b);

    print(expect_result, 10, "expect_result ");
    print(actural_result, 10, "actural_result");
    auto is_correct = cmp->verify(actural_result, expect_result);

    long comm = cmp->communication_cost(cmp_encode_b_cipher, result);

    cmp->print();
cout<< " decrypt result a > b                      : "<< is_correct 
    << " \n overall time cost                         : "<< finish/1000     
    << " ms\n overall comm. cost                        : "<< comm/1024 // /8 or not?
    << " KB\n amortized time cost                       : "<< (float)finish/1024/num_cmps 
    << " ms\n amortized comm. cost                      : "<< (float)comm/1024 /num_cmps <<" KB"
    << endl;

}
