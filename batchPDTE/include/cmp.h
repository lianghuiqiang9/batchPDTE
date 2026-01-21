#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>
#include "lhe.h"

using namespace std;
using namespace seal;

class CMP {
public:
    string name;
    int n;
    uint64_t num_cmps;
    uint64_t slot_count;
    uint64_t row_count;
    uint64_t depth;
    shared_ptr<LHE> lhe;
    inline static std::mt19937 gen{ std::random_device{}() };

    virtual ~CMP() = default;

    virtual vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) = 0;
    
    virtual vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) = 0;

    virtual vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a) = 0;

    virtual Plaintext init_x_zero_zero(const vector<uint64_t>& salt) = 0;

    virtual Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) = 0;

    virtual void clear_up(Ciphertext& result) = 0;

    virtual vector<uint64_t> decrypt(const Ciphertext& ct) = 0;

    virtual vector<uint64_t> recover(const Ciphertext& ct) = 0;

    virtual vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) = 0;
    
    virtual vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& a) = 0;

    virtual vector<vector<uint64_t>> random_raw_encode_b() = 0;

    virtual vector<vector<uint64_t>> random_raw_encode_a() = 0;

    virtual void print() = 0;

    // 
    virtual vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) = 0;

    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
        for(int i=0;i<num_cmps;i++){
            //cout<<"actural_result: "<<actural_result[i]<< " result: "<<result[i]<<endl;
            
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }

    long communication_cost(const vector<Ciphertext>& ct1, const Ciphertext& ct2) {
        long comm = 0;
        for(const auto& e : ct1) comm += e.save_size();
        comm += ct2.save_size();
        return comm;
    }

    long communication_cost(const Ciphertext& ct1, const Ciphertext& ct2){
        long comm = 0;
        comm+=ct1.save_size();
        comm+=ct2.save_size();
        return comm;
    }

};