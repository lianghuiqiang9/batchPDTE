#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"

class Cdcmp {
public:
    string name;
    int n;
    unique_ptr<LHE> lhe; 
    uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t slot_count;
    uint64_t row_count;
    uint64_t num_cmps_per_row;
    uint64_t num_cmps;
    Plaintext one_zero_zero;
    //Ciphertext one_zero_zero_cipher;

    Cdcmp(int n, int tree_depth = 0, int extra = 0) {
        this->name = "cdcmp";
        this->n = n;

        int cmp_depth_need =  static_cast<int>(std::ceil(std::log2(n)) + 1);
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);

        slot_count = lhe->slot_count;
        row_count = slot_count / 2;
        num_slots_per_element = n;
        num_cmps_per_row = row_count / num_slots_per_element;
        num_cmps = num_cmps_per_row * 2;

        // for cmp
        one_zero_zero = init_one_zero_zero();
        //one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    }


    // input
    // b = [ b00, b01, b02, ..., b10, b11, b12, ...]
    // output
    // neg_b = [ 1 - b00, 1 - b01, 1 - b02, ..., 1 - b10, 1 - b11, 1 - b12, ...]
    vector<uint64_t> encode_b(const vector<uint64_t>& b){
        vector<uint64_t> out(b.size());
        for(int i = 0 ; i < b.size(); i++){
            out[i] = 1 - b[i];
        }
        return out;
    }

    Ciphertext encrypt(const vector<uint64_t>& b){
        return lhe->encrypt(b);
    }

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<uint64_t> encode_a(const vector<uint64_t>& a){
        return a;
    }

    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_zero_zero(){
        vector<uint64_t> one_zero_zero(slot_count, 0ULL);
        for(int i = 0; i < num_cmps ; i++){
            auto idx = get_index(i);
            one_zero_zero[idx] = 1ULL;
        }
        return lhe->encode(one_zero_zero);
    }

    Ciphertext gt(const vector<uint64_t>& a, Ciphertext& b){

        auto pt_a = lhe->encode(a);
        auto gt = lhe->multiply_plain(b, pt_a);
        auto eq = lhe->add(gt, gt);
        lhe->negate_inplace(eq);
        lhe->add_inplace(eq, b);
        lhe->add_plain_inplace(eq, pt_a);


        int depth = log(n)/log(2);
        for(int i = 0; i < depth ; i++){
            int temp1 = 1<<i;

            auto gt_temp = lhe->rotate_rows(gt, temp1);
            auto eq_temp = lhe->rotate_rows(eq, temp1);
            lhe->multiply_inplace(eq, eq_temp);
            lhe->relinearize_inplace(eq);
            lhe->multiply_inplace(gt, eq_temp);
            lhe->relinearize_inplace(gt);
            lhe->add_inplace(gt, gt_temp);

        }
        return gt;

    }

    void clear_up(Ciphertext& result){
        lhe->multiply_plain_inplace(result, one_zero_zero);
    }

    vector<uint64_t> decrypt(const Ciphertext& ct){
        auto pt = lhe->decrypt(ct);
        return lhe->decode(pt);
    }

    vector<uint64_t> decode(const std::vector<uint64_t>& res){
        vector<uint64_t> ans(num_cmps);
        for(uint64_t i = 0; i < num_cmps ; i++){
            auto idx = get_index(i);
            ans[i] = res[idx];
        }
        return ans;
    }

    vector<uint64_t> decode(const Ciphertext& ct){
        auto res = this->decrypt(ct);
        return this->decode(res);
    }

    uint64_t get_index(uint64_t i){
        //idx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = i < num_cmps_per_row; 
        uint64_t idx = flag ? ( i * num_slots_per_element ) : ( row_count + (i - num_cmps_per_row) * num_slots_per_element);
        return idx;
    }

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<uint64_t>& a, const vector<uint64_t>& b){
        vector<bool> out(num_cmps, false);
        for(int i = 0; i < num_cmps ; ++i){
            auto offset = i*n;
            for(int k = offset + n - 1;k >= offset; --k){
                if(a[k] > b[k]){
                    out[i] = true;
                    break;
                }else if(a[k] == b[k]){
                    continue;
                }else{
                    break;
                }
            }
        }
        return out;
    }

    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
        for(int i=0;i<num_cmps;i++){
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }

    bool verify(const vector<uint64_t>& a, const vector<uint64_t>& b, const vector<uint64_t>& result){
        auto actural_result = verify(a,b);

        return verify(actural_result, result);
    }



    // input= [ b0,  b1,  b2 ]
    // out  = [ b00, b01, b02, ..., b10, b11, b12, ..., b20, b21, b22, ... ]
    // b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
    // b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
    // b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
    vector<uint64_t> raw_encode(vector<uint64_t> in){
        vector<uint64_t> out(slot_count, 0ULL);
        for(int i = 0; i < num_cmps_per_row; ++i){

            auto offset = i * num_slots_per_element;
            auto offset1 = num_cmps_per_row + i;

            for(int j = 0 ; j < num_slots_per_element; ++j){ 
                auto offset2 = j + offset;
                out[ offset2 ] = (out[i] >> j ) & 1;  
                out[ row_count + offset2 ]= (out[ offset1 ] >> j ) & 1;
            }
        }
        return out;
    }


    // low to high  
    vector<uint64_t> random_raw_encode_b(){
        //srand(time(NULL));
        vector<uint64_t> out(slot_count);
        for(int i = 0; i < slot_count ;i++){
            out[i] = rand() % 2;  
        }
        return out;
    }

    // low to high
    vector<uint64_t> random_raw_encode_a(){
        srand(time(NULL));
        vector<uint64_t> out(slot_count);
        for(int i = 0; i < slot_count ;i++){
            out[i] = rand() % 2;  
        }
        return out;
    }

    long communication_cost(const Ciphertext& ct1, const Ciphertext& ct2){
        stringstream send;
        long comm;
        comm+=ct1.save(send);
        comm+=ct2.save(send);
        return comm;
    }
    void print(){
        cout << " Name                                      : " << name
             << "\n bit precision                             : "<< n 
             << "\n compare number                            : "<< num_cmps
             << "\n";
    }


};