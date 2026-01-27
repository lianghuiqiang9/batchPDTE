#ifndef __DCMP__
#define __DCMP__

#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "cmp.h"

class DCMP : public CMP {
public:

    DCMP(int l, int m, int extra = 0, bool is_rotate = false);

    ~DCMP() = default;

    // input
    // b = [ b00, b01, b02, ..., b10, b11, b12, ...]
    // output
    // neg_b = [ 1 - b00, 1 - b01, 1 - b02, ..., 1 - b10, 1 - b11, 1 - b12, ...]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& raw_b) override;


    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& raw_b) override;

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a);

    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_zero_zero();

    Plaintext init_x_zero_zero(const vector<uint64_t>& x) override;

    // a > E(b)
    Ciphertext great_than(vector<Plaintext>& raw_a, vector<Ciphertext>& raw_b);

    void clear_up(Ciphertext& result) override;

    vector<uint64_t> decrypt(const Ciphertext& ct) override;

    vector<uint64_t> decode(const std::vector<uint64_t>& res);

    vector<uint64_t> recover(const Ciphertext& ct) override;

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& raw_b) override;

    // input= [ b0,  b1,  b2 ]
    // out  = [ b00, b01, b02, ..., b10, b11, b12, ..., b20, b21, b22, ... ]
    // b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
    // b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
    // b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) override;

    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& in) override;

    vector<uint64_t> raw_decode_b(const vector<vector<uint64_t>>& encoded_out, size_t original_b_size) override;
    
    vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) override;
    // low to high  
    vector<vector<uint64_t>> random_raw_encode_b() override;

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override;

    Plaintext get_one_hot(uint64_t start, uint64_t width) override;

    vector<Ciphertext> rotate_m_rows(const vector<Ciphertext>& b, const vector<Ciphertext>& b_inv_rows, int step) override;

    Ciphertext rotate_m_rows(const Ciphertext& b, const Ciphertext& b_inv_rows, int step) override;

    vector<Ciphertext> rotate_m_rows(const vector<Ciphertext>& b, int step) override;

    Ciphertext rotate_m_rows(const Ciphertext& b, int step) override;

    void print() override;

};


#endif