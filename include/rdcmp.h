#ifndef __RDCMP__
#define __RDCMP__
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "cmp.h"

class Rdcmp : public CMP {
public:
    Plaintext one_one_zero;

    Rdcmp(int l, int m, int n, int extra = 0);

    ~Rdcmp() override = default;

    // input
    // b = [ b00, b10, ...
    //       b01, b11, ...
    //       b02, b12, ...]
    // output
    // b = [ 1 - b00, 1 - b10, ...
    //       1 - b01, 1 - b11, ...
    //       1 - b02, 1 - b12, ...]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) override;

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) override;

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a) override;

    
    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_one_zero();

    Plaintext init_x_zero_zero(const vector<uint64_t>& salt) override;
    
    Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) override;

    void clear_up(Ciphertext& result) override;

    vector<uint64_t> decrypt(const Ciphertext& ct) ;

    vector<uint64_t> decode(const std::vector<uint64_t>& res);

    vector<uint64_t> recover(const Ciphertext& ct) override;

    // raw_a, raw_b. 
    vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) override;

    // input= [ b0,  b1,  b2, ... ]
    // out  = [ b00, b10, b20, ...
    //          b01, b11, b21, ...
    //          b02, b12, b22, ... ]
    // b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
    // b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
    // b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) override;

    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& in) override;

    // low to high  
    vector<vector<uint64_t>> random_raw_encode_b();

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override ;

    void print() override;

};


#endif