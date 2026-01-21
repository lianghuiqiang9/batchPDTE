#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"
#include "cmp.h"

class Rdcmp : public CMP {
public:
    Plaintext zero_zero_one;
    //Ciphertext one_zero_zero_cipher;
    // inline static std::mt19937 gen{ std::random_device{}() };

    Rdcmp(int l, int m, int n, int tree_depth = 0, int extra = 0) {
        this->name = "rdcmp";
        this->n = n;

        int cmp_depth_need =  static_cast<int>(std::ceil(std::log2(n)) + 1);
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);

        slot_count = lhe->slot_count;
        row_count = slot_count / 2;
        //num_slots_per_element = n;
        //num_cmps_per_row = row_count / num_slots_per_element;
        num_cmps = slot_count - 1;//Reserve a padding bit to prevent the plaintext from being 0

        // for cmp
        zero_zero_one = init_zero_zero_one();
        //one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    }
    ~Rdcmp() override = default;

    // input
    // b = [ b00, b10, ...
    //       b01, b11, ...
    //       b02, b12, ...]
    // output
    // b = [ 1 - b00, 1 - b10, ...
    //       1 - b01, 1 - b11, ...
    //       1 - b02, 1 - b12, ...]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) override {
        vector<vector<uint64_t>> out(n);
        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps, 0);
            for(int j = 0; j<num_cmps; j++){

                // negitive
                temp[j] = 1 - b[i][j]; 
            }
            out[i] = temp;
        }
        return out;
    }

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) override {
        vector<Ciphertext> out(n);
        for(int i = 0 ; i < n; i++){
            out[i] = lhe->encrypt(b[i]);
        }

        return out;
    }

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a) override {
        vector<Plaintext> out(n);
        for(int i = 0; i < n; ++i) {
            out[i] = lhe->encode(a[i]);
        }
        return out;
    }

    
    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_zero_zero_one(){
        vector<uint64_t> zero_zero_one(slot_count,1);
        zero_zero_one[num_cmps] = 0;
        return lhe->encode(zero_zero_one);
    }
    
    Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) override {
        vector<Ciphertext> eq(n);
        vector<Ciphertext> gt(n);

        for(int i = 0; i < n; ++i){
            gt[i] = lhe->multiply_plain(b[i], a[i]);
            eq[i] = lhe->add(gt[i], gt[i]);
            lhe->negate_inplace(eq[i]);
            lhe->add_inplace(eq[i], b[i]);
            lhe->add_plain_inplace(eq[i], a[i]);
        }

        int depth = static_cast<int>(log2(n));
        for(int i = 0; i < depth ; ++i){
            int temp1 = 1<<i;
            int temp0 = 1<<(i + 1);
            //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
            for(int j = 0; j < n; j = j + temp0){
                lhe->multiply_inplace(gt[j], eq[j + temp1]);
                lhe->relinearize_inplace(gt[j]);
                lhe->add_inplace(gt[j], gt[j + temp1]);
                lhe->multiply_inplace(eq[j], eq[j + temp1]);
                lhe->relinearize_inplace(eq[j]);

            }   
        }
        return std::move(gt[0]);

    }

    void clear_up(Ciphertext& result) override {
        lhe->multiply_plain_inplace(result, zero_zero_one);
    }

    vector<uint64_t> decrypt(const Ciphertext& ct) override {
        auto pt = lhe->decrypt(ct);
        return lhe->decode(pt);
    }

    vector<uint64_t> decode(const std::vector<uint64_t>& res) {
        return res;
    }

    vector<uint64_t> recover(const Ciphertext& ct) override {
        auto res = this->decrypt(ct);
        return this->decode(res);
    }

    // raw_a, raw_b. 
    vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) override {
        vector<bool> out(num_cmps, false);
        for(int i = 0;i < num_cmps;i++){
            uint64_t actural_result = 0;
            for(int k = n-1;k>=0;k--){
                if(a[k][i] > b[k][i]){
                    out[i] = true;
                    break;
                }else if(a[k][i] == b[k][i]){
                    continue;
                }else{
                    break;
                }
            }
        }
        return out;
    }

    // input= [ b0,  b1,  b2, ... ]
    // out  = [ b00, b10, b20, ...
    //          b01, b11, b21, ...
    //          b02, b12, b22, ... ]
    // b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
    // b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
    // b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& b) override {
        vector<vector<uint64_t>> out(n, vector<uint64_t>(slot_count, 0ULL));
        
        for(int i = 0; i < n; i++) {
            uint64_t* out_ptr = out[i].data();
            for(int j = 0; j < b.size(); j++) {
                out_ptr[j] = (b[j] >> i) & 1; 
            }
            out_ptr[num_cmps] = 13; // Padding bit
        }
        return out;
    }
    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& in) override {
        return raw_encode_b(in);
    }

    // low to high  
    vector<vector<uint64_t>> random_raw_encode_b() override {
        vector<vector<uint64_t>> out(n, vector<uint64_t>(num_cmps));
        std::uniform_int_distribution<uint64_t> dist(0, 1);

        for (int i = 0; i < n; i++) {
            uint64_t* row_ptr = out[i].data();
            for (uint64_t j = 0; j < num_cmps; j++) {
                row_ptr[j] = dist(gen);
            }
        }
        return out;
    }

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override {
        return random_raw_encode_b();
    }

    void print() override {
        cout << " name                                     : " << name
             << " \n max depth                                : "<< depth
             << " \n bit precision (n)                        : "<< n 
             << " \n compare number                           : "<< num_cmps
             << " \n";
    }


};