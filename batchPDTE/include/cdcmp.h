#ifndef __CDCMP__
#define __CDCMP__

#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"
#include "cmp.h"

class Cdcmp : public CMP{
public:
    uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t num_cmps_per_row;
    Plaintext one_zero_zero;
    //Ciphertext one_zero_zero_cipher;
    vector<uint64_t> index_map;

    Cdcmp(int l, int m, int n, int tree_depth = 0, int extra = 0);

    ~Cdcmp() = default;

    // input
    // b = [ b00, b01, b02, ..., b10, b11, b12, ...]
    // output
    // neg_b = [ 1 - b00, 1 - b01, 1 - b02, ..., 1 - b10, 1 - b11, 1 - b12, ...]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& raw_b) override;

    /*
    vector<uint64_t> _encode_process(const vector<vector<uint64_t>>& raw_b){
        auto b = raw_b[0];
        vector<uint64_t> out(slot_count, 0ULL);     
        size_t num_elements = std::min((size_t)num_cmps, b.size() / n);
        for (size_t i = 0; i < num_elements; ++i) {
            size_t target_base_pos = index_map[i];
            size_t source_base_pos = i * n;
            for (int j = 0; j < n; ++j) {
                out[target_base_pos + j] = b[source_base_pos + j];
            }
        }
        return out;
    }
    */

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& raw_b) override;

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a);

    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_zero_zero();

    Plaintext init_x_zero_zero(const vector<uint64_t>& salt) override;

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

    // low to high  
    vector<vector<uint64_t>> random_raw_encode_b() override;

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override;

    void print() override;

};


#endif