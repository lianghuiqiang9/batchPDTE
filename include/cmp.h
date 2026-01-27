#ifndef __CMP__
#define __CMP__

#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>
#include "lhe.h"
#include "bfv.h"
#include "bgv.h"

class CMP {
public:
    string scheme;
    int l;  // 
    int m;
    int n;
    uint64_t depth;
    shared_ptr<LHE> lhe;
    uint64_t num_cmps;
    uint64_t slot_count;
    uint64_t row_count;
    uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t num_cmps_per_row;
    Plaintext one_zero_zero;
    //Ciphertext one_zero_zero_cipher;
    vector<uint64_t> index_map;

    inline static std::mt19937 gen{ std::random_device{}() };

    virtual ~CMP() = default;

    virtual vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) = 0;
    
    virtual vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) = 0;

    virtual vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a) = 0;

    virtual Plaintext init_x_zero_zero(const vector<uint64_t>& x) = 0;

    virtual Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) = 0;

    virtual void clear_up(Ciphertext& result) = 0;

    virtual vector<uint64_t> decrypt(const Ciphertext& ct) = 0;

    virtual vector<uint64_t> recover(const Ciphertext& ct) = 0;

    virtual vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) = 0;
    
    virtual vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& a) = 0;

    virtual vector<uint64_t> raw_decode_b(const vector<vector<uint64_t>>& encoded_out, size_t original_b_size) = 0;

    virtual vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) = 0;

    virtual vector<vector<uint64_t>> random_raw_encode_b() = 0;

    virtual vector<vector<uint64_t>> random_raw_encode_a() = 0;

    virtual Plaintext get_one_hot(uint64_t start, uint64_t width) = 0;

    virtual vector<Ciphertext> rotate_m_rows(const vector<Ciphertext>& b, const vector<Ciphertext>& b_inv_rows, int step) = 0;
    
    virtual Ciphertext rotate_m_rows(const Ciphertext& b, const Ciphertext& b_inv_rows, int step) = 0;

    virtual vector<Ciphertext> rotate_m_rows(const vector<Ciphertext>& b, int step) = 0;

    virtual Ciphertext rotate_m_rows(const Ciphertext& b, int step) = 0;

    virtual void print() = 0;

    // 
    virtual vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) = 0;

    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result);

    long keys_size();

    long comm_cost(const vector<Ciphertext>& ct1, const Ciphertext& ct2);

    long comm_cost(const Ciphertext& ct1, const Ciphertext& ct2);

    long comm_cost_estimate(const vector<Ciphertext>& ct1, const Ciphertext& ct2);

    long comm_cost_estimate(const Ciphertext& ct1, const Ciphertext& ct2);

};


#endif