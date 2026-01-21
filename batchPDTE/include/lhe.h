#ifndef __LHE__
#define __LHE__

#include<seal/seal.h>
#include<vector>
#include<string>

using namespace std;
using namespace seal;

class LHE {
public:
    string scheme;
    uint8_t scheme_id; // 
    uint64_t depth;
    vector<int> steps;

    EncryptionParameters parms;
    shared_ptr<SEALContext> context; 
    
    unique_ptr<Encryptor> encryptor;
    unique_ptr<Evaluator> evaluator;
    unique_ptr<BatchEncoder> batch_encoder;
    RelinKeys rlk;
    GaloisKeys gal_keys;
    
    uint64_t log_poly_mod_degree;
    uint64_t plain_modulus;
    uint64_t slot_count;
    

    LHE(string scheme, int depth = 3, vector<int> steps = vector<int>{});


    ~LHE() = default;

    Plaintext encode(const std::vector<uint64_t>& a);

    Ciphertext encrypt(const Plaintext& pt);

    Ciphertext encrypt(const std::vector<uint64_t>& a);

    std::vector<uint64_t> decode(const Plaintext& pt);

    Plaintext decrypt(const Ciphertext& ct);

    int get_noise_budget(const Ciphertext& ct);

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2);

    void mod_switch(const Ciphertext& ct, Plaintext& pt);

    void print();

    Ciphertext multiply_plain(const Ciphertext& ct, Plaintext& pt);

    void multiply_plain_inplace(Ciphertext& ct, Plaintext& pt);

    void multiply_plain_inplace(Ciphertext& ct, vector<uint64_t>& a);

    Ciphertext multiply_plain(const Ciphertext& ct, const std::vector<uint64_t>& a);

    Ciphertext multiply(Ciphertext& ct1, Ciphertext& ct2);
    
    Ciphertext multiply_many(vector<Ciphertext>& ct_many);
    
    void multiply_inplace(Ciphertext& ct1, Ciphertext& ct2);

    Ciphertext rotate_rows(const Ciphertext& ct, int step);

    void add_inplace(Ciphertext& ct1, Ciphertext& ct2);

    void sub_inplace(Ciphertext& ct1, Ciphertext& ct2);

    void sub_plain_inplace(Ciphertext& ct, Plaintext& pt);

    Ciphertext sub_plain(Ciphertext& ct, Plaintext& pt);

    Ciphertext add(Ciphertext& ct1, Ciphertext& ct2);

    void add_plain_inplace(Ciphertext& ct, Plaintext& pt);

    void add_plain_inplace(Ciphertext& ct, const std::vector<uint64_t>& a);

    Ciphertext add_plain(const Ciphertext& ct, Plaintext& pt);

    Ciphertext add_plain(const Ciphertext& ct, const std::vector<uint64_t>& a);

    Ciphertext add_many(vector<Ciphertext>& ct_many);

    void relinearize_inplace(Ciphertext& ct);

    void negate_inplace(Ciphertext& ct);

    void negate(Ciphertext& ct1, Ciphertext& ct2);

private:
    unique_ptr<Decryptor> decryptor;

};

#endif