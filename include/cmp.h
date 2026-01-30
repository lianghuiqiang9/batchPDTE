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
    //vector<uint64_t> index_map;

    inline static std::mt19937 gen{ std::random_device{}() };

    virtual ~CMP() = default;

    virtual vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) = 0;
    
    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b);

    virtual vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a) = 0;

    Plaintext init_one_zero_zero();

    Plaintext init_x_zero_zero(const vector<uint64_t>& x);

    virtual Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) = 0;

    void clear_up(Ciphertext& result);

    vector<uint64_t> decrypt(const Ciphertext& ct);

    vector<uint64_t> decode(const std::vector<uint64_t>& res);

    vector<uint64_t> recover(const Ciphertext& ct);

    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b);
    
    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& a);

    vector<uint64_t> raw_decode_b(const vector<vector<uint64_t>>& encoded_out, size_t original_b_size);

    virtual vector<vector<uint64_t>> decode_b(const vector<Ciphertext>& cts) = 0;

    vector<vector<uint64_t>> random_raw_encode_b();

    vector<vector<uint64_t>> random_raw_encode_a();

    Plaintext get_one_hot(uint64_t start, uint64_t width);

    Plaintext get_double_one_hot(uint64_t start, uint64_t width);

    vector<Ciphertext> fill_double_width_hot(vector<Ciphertext>& data, uint64_t start, uint64_t width, uint64_t repeat);

    vector<Ciphertext> exchange(const vector<Ciphertext>& b, uint64_t end_index, uint64_t start_index);
    
    vector<Ciphertext> exchange(const vector<Ciphertext>& b, const vector<Ciphertext>& b_rot_columns, uint64_t end_index, uint64_t start_index);
    
    Ciphertext exchange(const Ciphertext& b, uint64_t end_index, uint64_t start_index);
    
    vector<Ciphertext> fill_width_hot(vector<Ciphertext>& data, uint64_t start, uint64_t width, uint64_t repeat);
    
    void print();
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