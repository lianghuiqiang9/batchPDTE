#include"tecmp.h"
#include"utils.h"
#include<unistd.h>

// g++ -o tecmp_test -O3 tecmp_test.cc -I /usr/local/include/SEAL-4.1 -lseal-4.1
// ./tecmp_test -m 8 -l 2

int main(int argc, char* argv[]) {
    int l,m; 
    int opt;
    while ((opt = getopt(argc, argv, "fl:m:")) != -1) {
        switch (opt) {
        case 'l': l = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        }
    }
    Tecmp tecmp(l,m);
    int n = l * m;
    auto num_cmps = tecmp.num_cmps;
    auto raw_encode_a = tecmp.random_raw_encode_a();
    auto raw_encode_b = tecmp.random_raw_encode_b();
    print(raw_encode_a, 10, "a: ");
    print(raw_encode_b, 10, "b: ");

    auto te_a = tecmp.encode_a(raw_encode_a);
    auto te_b = tecmp.encode_b(raw_encode_b);
    auto te_b_cipher = tecmp.encrypt(te_b);
    
    //cmp
    Ciphertext result;
    auto finish = profile("Cmp", [&]() { result = tecmp.gt(te_a, te_b_cipher);});
    
    tecmp.clear_up(result);
    auto expect_result = tecmp.decode(result);
    auto actural_result = tecmp.verify(raw_encode_a, raw_encode_b);

    print(expect_result, 10, "expect_result");
    print(actural_result, 10, "actural_result");
    auto is_correct = tecmp.verify(actural_result, expect_result);

    long comm = tecmp.communication_cost(te_b_cipher, result);

cout<<    " decrypt result res[0] == a > b            : "<< is_correct 
    << ",\n l                                         : "<< l 
    << ",\n m                                         : "<< m
    << ",\n bit precision                             : "<< n 
    << ",\n compare number                            : "<< num_cmps 
    << ",\n overall time cost                         : "<< finish/1000     
    << " ms\n overall comm. cost                        : "<< comm/1024 // /8 or not?
    << " KB\n amortized time cost                       : "<< (float)finish/1024/num_cmps 
    << " ms\n amortized comm. cost                      : "<< (float)comm/1024 /num_cmps <<" KB"
    << endl;

}
