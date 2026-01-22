#include "lhe.h"
#include "bfv.h"
#include "bgv.h"
#include "utils.h"
#include<unistd.h>

// g++ -o lhe_depth_test -O3 lhe_depth_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./lhe_depth_test -t 0

int main(int argc, char* argv[]) {

    cout << "--- Initializing LHE System ---" << endl;
    const int depth = 12;
    const vector<int> steps;

    int opt;
    int lhe_type = 0;
    unique_ptr<LHE> lhe;

    while ((opt = getopt(argc, argv, "ft:")) != -1) {
        switch (opt) {
        case 't': lhe_type = atoi(optarg); break;
        }
    }
    switch (lhe_type) {
        case 0: lhe = make_unique<BFV>(depth, steps); break;
        case 1: lhe = make_unique<BGV>(depth, steps); break;
    }

    print_vec(steps, steps.size(), ""); 
    cout << endl;
    auto mod = lhe->plain_modulus;
    auto slots = lhe->slot_count;
    
    cout << endl << "--- Actural Depth ---" << endl;
    
    {
        vector<uint64_t> a(slots, 0);
        for(size_t i = 0; i < slots; i++) {
            a[i] = i % mod;           // [0, 1, 2, ...]
        }
        auto ct = lhe->encrypt(a);
        for (int i = 0; i < 20; ++i) {

            ct = lhe->multiply(ct, ct);
            lhe->relinearize_inplace(ct);
            a = mul(a, a, mod);

            auto pt = lhe->decrypt(ct);
            auto result = lhe->decode(pt);

            print_vec(result, 10);
            print_vec(a, 10);

            if (a!=result){
                cout<<"actural max depth : "<<i<<endl;
                break;
            }
        }    
    }
    cout << endl << "--- Actural Depth (fresh ciphertext) ---" << endl;

    {
        vector<uint64_t> a(slots, 0);
        for(size_t i = 0; i < slots; i++) {
            a[i] = i % mod;           // [0, 1, 2, ...]
        }
        auto ct = lhe->encrypt(a);
        auto a_temp = a;
        auto ct_temp = lhe->encrypt(a);

        for (int i = 0; i < 20; ++i) {

            ct = lhe->multiply(ct, ct_temp);
            lhe->relinearize_inplace(ct);
            a = mul(a, a_temp, mod);

            auto pt = lhe->decrypt(ct);
            auto result = lhe->decode(pt);

            print_vec(result, 10);
            print_vec(a, 10);

            if (a!=result){
                cout<<"actural max depth (fresh ciphertext) : "<<i<<endl;
                break;
            }
        }
    }



    return 0;
}