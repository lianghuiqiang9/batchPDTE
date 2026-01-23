#ifndef __TECMP__
#define __TECMP__
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "cmp.h"

class Tecmp : public CMP {
public:
    uint8_t id; // 0x1, 0x2
    int l;  // 
    int m;
    uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t num_cmps_per_row;
    Plaintext one_zero_zero;
    Ciphertext one_zero_zero_cipher;
    vector<uint64_t> index_map;

    Tecmp(int l, int m, int n = 0, int extra = 0, uint8_t id = 0x1);

    ~Tecmp() = default;

    // input
    // num_cmps 
    // b = [ b00, b10, b20
    //       b01, b11, b21
    //       b02, b12, b22 ]
    // b0 = b00 + 2^m * b01 + (2^m)^2 * b02 ;
    // b1 = b10 + 2^m * b11 + (2^m)^2 * b12 ;
    // b2 = b20 + 2^m * b21 + (2^m)^2 * b22 ;
    // output
    // b = [ te(b00), te(b10), te(b20)
    //       te(b01), te(b11), te(b21)
    //       te(b02), te(b12), te(b22) ]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) override;

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) override ;

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a) override;

    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_zero_zero();

    Plaintext init_x_zero_zero(const vector<uint64_t>& salt) override;

    Ciphertext great_than(vector<Plaintext>& pt_a, vector<Ciphertext>& b) override;

    void clear_up(Ciphertext& result) override;

    vector<uint64_t> decrypt(const Ciphertext& ct) override;

    vector<uint64_t> decode(const std::vector<uint64_t>& res);

    vector<uint64_t> recover(const Ciphertext& ct) override ;

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& b) override;

    // input= [ b0,  b1,  b2 ]
    // out  = [ b00, b10, b20
    //          b01, b11, b21
    //          b02, b12, b22 ]
    // b0 = b00 + 2^m * b01 + (2^m)^2 * b02 ;
    // b1 = b10 + 2^m * b11 + (2^m)^2 * b12 ;
    // b2 = b20 + 2^m * b21 + (2^m)^2 * b22 ;
    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) override;

    // input = a
    // out = [a01, a02, a03, ...]
    // a = a00 + 2^m * a01 + (2^m)^2 * a02 ;
    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& raw_a) override;

    // low to high
    vector<vector<uint64_t>> random_raw_encode_b() override;

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override;

    void print() override;

};

#endif