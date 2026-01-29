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

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a);

    // a > E(b)
    Ciphertext great_than(vector<Plaintext>& raw_a, vector<Ciphertext>& raw_b);

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& raw_b) override;

    vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) override;

};


#endif