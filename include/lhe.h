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
    uint64_t depth;
    vector<int> steps;

    EncryptionParameters parms;
    shared_ptr<SEALContext> context; 
    
    unique_ptr<Encryptor> encryptor;
    unique_ptr<Evaluator> evaluator;
    unique_ptr<BatchEncoder> batch_encoder;

    PublicKey pk;
    RelinKeys rlk;
    GaloisKeys gal_keys;
    
    uint64_t log_poly_mod_degree;
    uint64_t plain_modulus;
    uint64_t slot_count;

    ~LHE() = default;

    Plaintext encode(const std::vector<uint64_t>& a);

    Ciphertext encrypt(const Plaintext& pt);

    Ciphertext encrypt(const std::vector<uint64_t>& a);

    std::vector<uint64_t> decode(const Plaintext& pt);

    virtual Plaintext decrypt(const Ciphertext& ct) = 0;

    virtual int get_noise_budget(const Ciphertext& ct) = 0;

    virtual void mod_switch(Ciphertext& ct1, Ciphertext& ct2) = 0;

    virtual void mod_switch(const Ciphertext& ct, Plaintext& pt) = 0;

    virtual void print() = 0;

    void multiply_plain_inplace(Ciphertext& ct, Plaintext& pt);

    void multiply_plain_inplace(Ciphertext& ct, vector<uint64_t>& a);

    void multiply_plain_inplace(vector<Ciphertext>& cts, Plaintext& pt);
    
    vector<Ciphertext> multiply_plain(vector<Ciphertext> cts, Plaintext& pt);

    Ciphertext multiply_plain(Ciphertext ct, std::vector<uint64_t>& a);

    Ciphertext multiply_plain(Ciphertext ct, Plaintext& pt);

    Ciphertext multiply(Ciphertext ct1, Ciphertext& ct2);
    
    Ciphertext multiply_many(vector<Ciphertext>& ct_many);

    vector<Ciphertext> multiply_many(vector<vector<Ciphertext>>& ct_manys);
    
    void multiply_inplace(Ciphertext& ct1, Ciphertext& ct2);

    void rotate_rows_inplace(Ciphertext& ct, int step);

    void rotate_rows_inplace(vector<Ciphertext>& cts, int step);

    Ciphertext rotate_rows(Ciphertext ct, int step);

    vector<Ciphertext> rotate_rows(vector<Ciphertext> cts, int step);

    void rotate_columns_inplace(Ciphertext& ct);

    void rotate_columns_inplace(vector<Ciphertext>& cts);

    Ciphertext rotate_columns(Ciphertext ct);

    vector<Ciphertext> rotate_columns(vector<Ciphertext> cts);

    void add_inplace(Ciphertext& ct1, Ciphertext& ct2);

    void add_inplace(vector<Ciphertext>& ct1, vector<Ciphertext>& ct2);

    void sub_inplace(Ciphertext& ct1, Ciphertext& ct2);

    void sub_plain_inplace(Ciphertext& ct, Plaintext& pt);

    Ciphertext sub_plain(Ciphertext ct, Plaintext& pt);

    Ciphertext add(Ciphertext& ct1, Ciphertext& ct2);

    void add_plain_inplace(Ciphertext& ct, Plaintext& pt);

    void add_plain_inplace(Ciphertext& ct, const std::vector<uint64_t>& a);

    Ciphertext add_plain(Ciphertext ct, Plaintext& pt);

    Ciphertext add_plain(Ciphertext ct, const std::vector<uint64_t>& a);

    Ciphertext add_many(vector<Ciphertext>& ct_many);

    void relinearize_inplace(Ciphertext& ct);

    void negate_inplace(Ciphertext& ct);

    Ciphertext negate(Ciphertext ct);

    long rlk_size();
    long pk_size();
    long glk_size();

};

#endif