

#include "lhe.h"
#include "utils.h"

// g++ -o lhe_test -O3 lhe_test.cc -I /usr/local/include/SEAL-4.1 -lseal-4.1
// ./lhe_test

int main() {

    cout << "--- Initializing LHE System ---" << endl;
    const string scheme = "bgv";
    const int depth = 12;
    const vector<int> steps = {-1,1};
    LHE lhe(scheme, depth, steps);
    lhe.print();
    const uint64_t slots = lhe.slot_count;
    const uint64_t mod = lhe.plain_modulus;


    vector<uint64_t> a(slots, 0);
    vector<uint64_t> b(slots, 0);
    for(size_t i = 0; i < slots; i++) {
        a[i] = i % mod;           // [0, 1, 2, ...]
        b[i] = i * 2 % mod;       // [0, 2, 4, ...]
    }

    cout << "--- Performance & Correctness Test ---" << endl;

    Plaintext pt1;
    Ciphertext ct1;
    profile("Encode              ", [&]() { pt1 = lhe.encode(a);});
    profile("Encrypt             ", [&]() { ct1 = lhe.encrypt(pt1);});
    auto pt2 = lhe.encode(b); 
    auto ct2 = lhe.encrypt(pt2);

    profile("Add_Inplace         ", [&]() { lhe.add_inplace(ct1, ct2); }); 
    profile("Multiply_Plain      ", [&]() { ct1 = lhe.multiply_plain(ct1, pt2); });

    int step = -1;
    profile("Rotate_Rows (step=1)", [&]() { ct1 = lhe.rotate_rows(ct1, step); });
    Ciphertext ct_res;
    profile("Multiply            ", [&]() { ct_res = lhe.multiply(ct1, ct2); lhe.relinearize_inplace(ct_res); });


    cout << endl << "--- Decrypting & Verifying ---" << endl;
    
    Plaintext pt_res;
    vector<uint64_t> result;
    profile("Decrypt             ", [&]() { pt_res = lhe.decrypt(ct_res); });
    profile("Decode              ", [&]() { result = lhe.decode(pt_res); });
    print(result, 10);

    auto actural_result = mul(
                            rotate(
                                mul( 
                                    add(a, 
                                        b, mod),
                                     b, mod),
                                 step), 
                            b, mod);

    print(actural_result, 10);

    cout << endl << "--- Multiply Performance ---" << endl;
    {
        auto ct = lhe.encrypt(a);
        profile("Depth times multiply", [&]() {
            for (int i = 0; i < depth; ++i) {
                profile("Multiply            ", [&]() { 
                    ct = lhe.multiply(ct, ct); 
                    lhe.relinearize_inplace(ct);
                });
            }
        });
    }

       return 0;
}