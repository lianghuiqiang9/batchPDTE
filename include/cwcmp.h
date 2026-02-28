#ifndef __CWCMP__
#define __CWCMP__

#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "cmp.h"

class CWEncoder {
private:
    std::vector<std::vector<uint64_t>> combo_table;

    void precompute_combinations(uint64_t max_l);

    static uint64_t nCr(uint64_t n, uint64_t r);

public:
    uint64_t l = 0; 
    uint64_t h = 0; 
    uint64_t n = 0; 

    CWEncoder() = default;
    CWEncoder(uint64_t h, uint64_t n);

    static uint64_t find_best_l(uint64_t h, uint64_t n);

    std::vector<uint64_t> encode(int64_t x) const;
};

class CWCMP : public CMP {
public:
    //bool is_padding = true;
    CWEncoder encoder;

    Ciphertext one_zero_zero_cipher;
    vector<Plaintext> h_factorial_inv_pt;
    vector<Plaintext> h_vec_pt;

    CWCMP(int l, int m, int extra = 0, bool is_rotate = false);

    ~CWCMP() = default;

    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) override;
    
    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& a) override;

    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& raw_b) override;

    vector<vector<uint64_t>> encode_a(const vector<vector<uint64_t>>& raw_a);

    // E(b) > a
    Ciphertext great_than(vector<vector<uint64_t>>& raw_a, vector<Ciphertext>& raw_b);

    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& raw_b) override;

    vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) override;

    void print() override;
    vector<Plaintext> init_h_factorial_inv_pt();
    vector<Plaintext> init_h_vec();
    Ciphertext map_to_boolean(Ciphertext& a, uint64_t h);
    Ciphertext piecewise_comp_rec(vector<Ciphertext>& b, vector<uint64_t>& a, uint64_t h);
    Ciphertext piecewise_comp_rec_GT_ab(vector<Ciphertext>& b, vector<uint64_t>& a, uint64_t h);
    Ciphertext map_to_boolean_zero(Ciphertext& a, uint64_t h);
};


#endif