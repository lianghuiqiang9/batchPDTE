#ifndef __TCMP__
#define __TCMP__
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "cmp.h"

class TCMP : public CMP {
public:
    uint8_t id; // 0x1, 0x2
    Ciphertext one_zero_zero_cipher;

    TCMP(int l, int m, int extra = 0, bool is_rotate = false, uint8_t id = 0x1);

    ~TCMP() = default;

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

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a) override;

    // a > E(b)
    Ciphertext great_than(vector<Plaintext>& pt_a, vector<Ciphertext>& b) override;

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& b) override;

    vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) override;
 
};

#endif