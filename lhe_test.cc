#include "bgv.h"
#include "bfv.h"
#include "utils.h"
#include<unistd.h>

// g++ -o lhe_test -O3 lhe_test.cc -I ./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./lhe_test -t 0

int main(int argc, char* argv[]) {
    cout << "--- Initializing LHE System ---" << endl;
    int depth = 12;
    vector<int> steps = {-1,1};

    int opt;
    int lhe_type = 0;
    unique_ptr<LHE> lhe;

    while ((opt = getopt(argc, argv, "ft:d:")) != -1) {
        switch (opt) {
        case 't': lhe_type = atoi(optarg); break;
        case 'd': depth = atoi(optarg); break;
        }
    }
    switch (lhe_type) {
        case 0: lhe = make_unique<BFV>(depth, steps); break;
        case 1: lhe = make_unique<BGV>(depth, steps); break;
    }

    lhe->print();
    const uint64_t slots = lhe->slot_count;
    const uint64_t mod = lhe->plain_modulus;

    vector<uint64_t> a(slots, 0);
    vector<uint64_t> b(slots, 0);
    for(size_t i = 0; i < slots; i++) {
        a[i] = i % mod;           // [0, 1, 2, ...]
        b[i] = i * 2 % mod;       // [0, 2, 4, ...]
    }

    cout << "--- Performance & Correctness Test ---" << endl;

    Plaintext pt1;
    Ciphertext ct1;
    profile("Encode              ", [&]() { pt1 = lhe->encode(a);});
    profile("Encrypt             ", [&]() { ct1 = lhe->encrypt(pt1);});
    auto pt2 = lhe->encode(b); 
    auto ct2 = lhe->encrypt(pt2);

    profile("Add_Inplace         ", [&]() { lhe->add_inplace(ct1, ct2); }); 
    profile("Multiply_Plain      ", [&]() { ct1 = lhe->multiply_plain(ct1, pt2); });

    int step = -1;
    profile("Rotate_Rows (step=1)", [&]() { ct1 = lhe->rotate_rows(ct1, step); });
    Ciphertext ct_res;
    profile("Multiply            ", [&]() { ct_res = lhe->multiply(ct1, ct2); lhe->relinearize_inplace(ct_res); });
    profile("Rotate_Cols         ", [&]() { ct_res = lhe->rotate_columns(ct_res); });

    cout << endl << "--- Decrypting & Verifying ---" << endl;
    
    Plaintext pt_res;
    vector<uint64_t> result;
    profile("Decrypt             ", [&]() { pt_res = lhe->decrypt(ct_res); });
    profile("Decode              ", [&]() { result = lhe->decode(pt_res); });
    print_vector(result, 10);

    auto actural_result = rotate_cols(
                            mul(
                                rotate_rows(
                                    mul( 
                                        add(a, 
                                            b, mod),
                                        b, mod),
                                    step), 
                            b, mod));

    print_vector(actural_result, 10);

    cout << endl << "--- Multiply Performance ---" << endl;
    {
        auto ct = lhe->encrypt(a);
        profile(std::to_string(depth) +" times multiply", [&]() {
            for (int i = 0; i < depth; ++i) {
                profile("Multiply            ", [&]() { 
                    ct = lhe->multiply(ct, ct); 
                    lhe->relinearize_inplace(ct);
                });
            }
        });
    }

    
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

            print_vector(result, 10);
            print_vector(a, 10);

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

            print_vector(result, 10);
            print_vector(a, 10);

            if (a!=result){
                cout<<"actural max depth (fresh ciphertext) : "<<i<<endl;
                break;
            }
        }
    }

    auto &coeff_modulus = lhe->parms.coeff_modulus();
    vector<uint64_t> q_vec(coeff_modulus.size());
    for (size_t i = 0; i < coeff_modulus.size(); i++) {
        q_vec[i] = log2(coeff_modulus[i].value())+1;
    }

cout<<   "lhe params: "
    << "\n N: "<< (1<<lhe->log_poly_mod_degree)
    << ",\n p: "<< lhe->plain_modulus
    << ",\n"; print_vector(q_vec, q_vec.size()," q:");


    return 0;
}