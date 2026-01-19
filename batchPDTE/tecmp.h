#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"
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

    Tecmp(int l, int m, int tree_depth = 0, int extra = 0, uint8_t id = 0x1) {
        this->name = "tecmp";
        this->id = id;
        this->l = l;
        this->m = m;
        this->n = l * m;

        int cmp_depth_need = (this->id == 0x1) ? static_cast<int>(std::ceil(std::log2(l))) : l;
        if(((1<<cmp_depth_need) !=l) && (this->id == 0x1)){
            cout<<" l should be 2^x, error"<<endl;
            exit(0);
        }
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);

        if (m >= this->lhe->log_poly_mod_degree - 1) {
            cout<< m <<" " << this->lhe->log_poly_mod_degree<<endl;
            cout << "Error: params m is too large." << endl;
            exit(0);
        }

        slot_count = lhe->slot_count;
        row_count = slot_count / 2;
        num_slots_per_element = 1 << m;
        num_cmps_per_row = row_count / num_slots_per_element;
        num_cmps = num_cmps_per_row * 2;

        // index_map
        //idx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        index_map.resize(num_cmps);
        for(uint64_t i = 0; i < num_cmps; i++) {
            bool flag = i < num_cmps_per_row;
            index_map[i] = flag ? (i * num_slots_per_element) : (row_count + (i - num_cmps_per_row) * num_slots_per_element);
        }
        // for cmp
        one_zero_zero = init_one_zero_zero();
        one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    }
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
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b) override {
        size_t rows = b.size();
        vector<vector<uint64_t>> out(rows, vector<uint64_t>(slot_count, 1ULL));

        for(size_t i = 0; i < rows; i++) {
            uint64_t* row_ptr = out[i].data();
            const auto& b_row = b[i];
            for(size_t j = 0; j < b_row.size(); j++) {
                uint64_t start_idx = index_map[j];
                uint64_t theta = b_row[j];
                std::fill_n(row_ptr + start_idx, theta + 1, 0ULL);
            }
        }
        return out;
    }

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b) override {
        vector<Ciphertext> out(b.size());
        for(int i = 0 ; i < b.size(); i++){
            out[i] = lhe->encrypt(b[i]);
        }
        return out;
    }

    // input
    // a = [ a00, a01, a02 ]
    // output
    // a = [ a00, a01, a02 ]
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& raw_a) override{
        auto out = lhe->encode(raw_a[0]);
        return vector<Plaintext>{out};
    }

    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_one_zero_zero(){
        vector<uint64_t> one_zero_zero(slot_count, 0ULL);
        for(int i = 0; i < num_cmps ; i++){
            one_zero_zero[index_map[i]] = 1ULL;
        }
        return lhe->encode(one_zero_zero);
    }

    Ciphertext great_than(vector<Plaintext>& pt_a, vector<Ciphertext>& b) override {
        auto a = lhe->decode(pt_a[0]);

        vector<Ciphertext> eq(l);
        vector<Ciphertext> gt(l);

        if(l == 1){
            gt[0] = lhe->rotate_rows(b[0], a[0]);
            return gt[0];
        }else{
            for(int i = 0; i < l; ++i){
                gt[i] = lhe->rotate_rows(b[i], a[i]);
                if(a[i] < num_slots_per_element - 1){
                    eq[i] = lhe->rotate_rows(b[i], a[i] + 1);
                }else{
                    eq[i] = one_zero_zero_cipher;
                }
                lhe->sub_inplace(eq[i], gt[i]); //eq = eq - gt ;
            }
        }

        // result = b > a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * gt_0))
        // result = b >= a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * (gt_0 + eq_0)))

        if (id==0x1){
            int depth = log(l)/log(2);
            for(int i = 0; i < depth ; i++){
                int temp1 = 1<<i;
                int temp0 = 1<<(i + 1);
                //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
                for(int j = 0; j < l; j = j + temp0){                    
                    lhe->multiply_inplace(gt[j], eq[j + temp1]);
                    lhe->relinearize_inplace(gt[j]);
                    lhe->add_inplace(gt[j], gt[j + temp1]);
                    lhe->multiply_inplace(eq[j],eq[j + temp1]);
                    lhe->relinearize_inplace(eq[j]);
                }   
            }
        }else{
            for(int i = 1;i < l; i++){
                lhe->multiply_inplace(gt[0], eq[i]);
                lhe->relinearize_inplace(gt[0]);
                lhe->add_inplace(gt[0], gt[i]);
            }

        }
        

        return std::move(gt[0]);

    }

    void clear_up(Ciphertext& result) override {
        lhe->multiply_plain_inplace(result, one_zero_zero);
    }

    vector<uint64_t> decrypt(const Ciphertext& ct) override {
        auto pt = lhe->decrypt(ct);
        return lhe->decode(pt);
    }

    vector<uint64_t> decode(const std::vector<uint64_t>& res) {
        vector<uint64_t> ans(num_cmps);
        for(uint64_t i = 0; i < num_cmps ; i++){
            ans[i] = res[index_map[i]];
        }
        return ans;
    }

    vector<uint64_t> decode(const Ciphertext& ct) override {
        auto res = this->decrypt(ct);
        return this->decode(res);
    }

/*
    // out = [ a[0]>b[0], a[1]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) override {
        vector<bool> out(num_cmps, false);
        for(int i = 0; i < num_cmps ; ++i){
            for(int k = l-1; k >= 0; --k){
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
*/
    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& b) override {
        auto a = raw_a[0];
        vector<bool> out(num_cmps, false);
        for(int i = 0; i < num_cmps ; ++i){
            for(int k = l-1; k >= 0; --k){
                if(a[k] > b[k][i]){
                    out[i] = true;
                    break;
                }else if(a[k] == b[k][i]){
                    continue;
                }else{
                    break;
                }
            }
        }
        return out;
    }

/*
    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
        for(int i=0;i<num_cmps;i++){
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }
*/
/*
    bool verify(const vector<uint64_t>& a, const vector<vector<uint64_t>>& b, const vector<uint64_t>& result){
        auto actural_result = verify(a,b);

        return verify(actural_result, result);
    }
*/

    // input= [ b0,  b1,  b2 ]
    // out  = [ b00, b10, b20
    //          b01, b11, b21
    //          b02, b12, b22 ]
    // b0 = b00 + 2^m * b01 + (2^m)^2 * b02 ;
    // b1 = b10 + 2^m * b11 + (2^m)^2 * b12 ;
    // b2 = b20 + 2^m * b21 + (2^m)^2 * b22 ;
    vector<vector<uint64_t>> raw_encode_b(const vector<uint64_t>& in) override {
        vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps, 0));
        const uint64_t range = num_slots_per_element - 1;
        const size_t in_size = in.size();

        for(int i = 0; i < l; i++) {
            const auto offset = i * m;
            uint64_t* out_ptr = out[i].data();
            const uint64_t* in_ptr = in.data();
            for(size_t j = 0; j < in_size; j++) {
                out_ptr[j] = (in_ptr[j] >> offset) & range;
            }
        }
        return out;
    }

    // input = a
    // out = [a01, a02, a03, ...]
    // a = a00 + 2^m * a01 + (2^m)^2 * a02 ;
    vector<vector<uint64_t>> raw_encode_a(const vector<uint64_t>& raw_a) override {
        auto a = raw_a[0];
        vector<uint64_t> out(l, 0ULL);
        auto range = num_slots_per_element - 1;

        for(int i = 0 ; i < l; i++){
            out[i] = (a >> (i * m) ) & range;
        }
        return  vector<vector<uint64_t>>{out};
    }

    // low to high
    vector<vector<uint64_t>> random_raw_encode_b() override {
        vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps));
    
        std::uniform_int_distribution<uint64_t> dist(0, num_slots_per_element - 1);

        for (int i = 0; i < l; i++) {
            uint64_t* row_ptr = out[i].data(); // 获取原始指针，提高写入速度
            for (uint64_t j = 0; j < num_cmps; j++) {
                row_ptr[j] = dist(gen);
            }
        }
        return out; 
    }

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a() override {
        vector<uint64_t> out(l);
        std::uniform_int_distribution<uint64_t> dist(0, num_slots_per_element - 1);

        std::generate(out.begin(), out.end(), [&]() {
            return dist(gen);
        });
        
        return vector<vector<uint64_t>>{out};
    }

    void print() override {
        cout << " name                                      : " << name
             << " \n l                                         : "<< l 
             << " \n m                                         : "<< m
             << " \n bit precision (n)                         : "<< n 
             << " \n compare number                            : "<< num_cmps
             <<endl ;
    }

};