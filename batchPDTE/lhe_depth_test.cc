
#include "lhe.h"
#include "utils.h"


// g++ -o lhe_depth_test -O3 lhe_depth_test.cc -I /usr/local/include/SEAL-4.1 -lseal-4.1

// ./lhe_depth_test

int main() {

    cout << "--- Initializing LHE System ---" << endl;
    const string scheme = "bgv";
    const int depth = 12;
    const vector<int> steps;

    LHE lhe(scheme, depth, steps);

    const uint64_t slots = lhe.slot_count;
    const uint64_t mod = lhe.plain_modulus;
    cout << "System Parameters:" << endl;
    cout << "  - Scheme:        " << scheme << endl;
    cout << "  - Max Depth:     " << depth << endl;
    cout << "  - Slots:         " << slots << endl;
    cout << "  - Plain Modulus: " << mod << endl;
    cout << "  - Rotation Steps: ";
    print(steps, steps.size(), ""); 
    cout << endl;

    cout << endl << "--- Actural Depth ---" << endl;
    
    {
        vector<uint64_t> a(slots, 0);
        for(size_t i = 0; i < slots; i++) {
            a[i] = i % mod;           // [0, 1, 2, ...]
        }
        auto ct = lhe.encrypt(a);
        for (int i = 0; i < 20; ++i) {

            ct = lhe.multiply(ct, ct);
            lhe.relinearize_inplace(ct);
            a = mul(a, a, mod);

            auto pt = lhe.decrypt(ct);
            auto result = lhe.decode(pt);

            print(result, 10);
            print(a, 10);

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
        auto ct = lhe.encrypt(a);
        auto a_temp = a;
        auto ct_temp = lhe.encrypt(a);

        for (int i = 0; i < 20; ++i) {

            ct = lhe.multiply(ct, ct_temp);
            lhe.relinearize_inplace(ct);
            a = mul(a, a_temp, mod);

            auto pt = lhe.decrypt(ct);
            auto result = lhe.decode(pt);

            print(result, 10);
            print(a, 10);

            if (a!=result){
                cout<<"actural max depth (fresh ciphertext) : "<<i<<endl;
                break;
            }
        }
    }



    return 0;
}