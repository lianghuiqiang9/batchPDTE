#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"

class Tecmp {
public:
    string name;
    uint8_t id; // 0x1, 0x2
    int l;  // 
    int m;
    int n;
    unique_ptr<LHE> lhe; 
    //uint64_t m_degree;
    uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t slot_count;
    uint64_t row_count;
    uint64_t num_cmps_per_row;
    uint64_t num_cmps;
    Plaintext one_zero_zero;
    Ciphertext one_zero_zero_cipher;

    Tecmp(int l, int m, int tree_depth = 0, int extra = 0, uint8_t id = 0x1) {
        this->name = "tecmp";
        this->id = id;
        this->l = l;
        this->m = m;
        this->n = l * m;

        int cmp_depth_need = (this->id == 0x1) ? static_cast<int>(std::ceil(std::log2(l))) : l;
        if((1<<cmp_depth_need !=l) && (this->id == 0x1)){
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

        // for cmp
        one_zero_zero = init_one_zero_zero();
        one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    }


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
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b){

        vector<vector<uint64_t>> out;
        for(int i = 0 ; i < b.size(); i++){
            vector<uint64_t> temp(lhe->slot_count, 1ULL);
            for(int j = 0; j < b[i].size(); j++){
                auto jdx = get_index(j);
                uint64_t theta = b[i][j];
                //cout<<"encrypted_op_encode[" <<j<< "]"<<encrypted_op_encode[j]<<" b = "<< b <<endl;
                for(int k = 0; k <= theta; k++){
                    temp[jdx + k] = 0ULL;     
                }
            }
            out.push_back(temp);
        }
        return out;
    }

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b_te){
        vector<Ciphertext> out(b_te.size());
        for(int i = 0 ; i < b_te.size(); i++){
            out[i] = lhe->encrypt(b_te[i]);
        }
        return out;
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

    Ciphertext great_than(const vector<uint64_t>& a, const vector<Ciphertext>& b){

        vector<Ciphertext> eq(l);
        vector<Ciphertext> gt(l);

        if(l == 1){

            //evaluator->rotate_rows(b[0], a[0], *gal_keys_server, gt[0]);
            gt[0] = lhe->rotate_rows(b[0], a[0]);
            return gt[0];
        }else{
            for(int i = 0; i < l; ++i){
                
                //evaluator->rotate_rows(b[i], a[i], *gal_keys_server, gt[i]);
                gt[i] = lhe->rotate_rows(b[i], a[i]);

                if(a[i] < num_slots_per_element - 1){
                    //evaluator->rotate_rows(b[i], a[i] + 1, *gal_keys_server, eq[i]);
                    eq[i] = lhe->rotate_rows(b[i], a[i] + 1);
                }else{
                    eq[i] = one_zero_zero_cipher;
                }
                //evaluator->sub_inplace(eq[i], gt[i]); //eq = eq - gt ;
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
                    //evaluator->multiply_inplace(gt[j],eq[ j + temp1]);
                    //evaluator->relinearize_inplace(gt[j],*rlk_server);
                    //evaluator->add_inplace(gt[j], gt[j + temp1]);
                    //evaluator->multiply_inplace(eq[j],eq[j + temp1]);
                    //evaluator->relinearize_inplace(eq[j],*rlk_server);
                    
                    lhe->multiply_inplace(gt[j], eq[j + temp1]);
                    lhe->relinearize_inplace(gt[j]);
                    lhe->add_inplace(gt[j], gt[j + temp1]);
                    lhe->multiply_inplace(eq[j],eq[j + temp1]);
                    lhe->relinearize_inplace(eq[j]);

                }   
            }
        }else{
            for(int i = 1;i < l; i++){
                //evaluator->multiply_inplace(comparisonResult,eq[i]); //l-1
                //evaluator->relinearize_inplace(comparisonResult,*rlk_server);
                //evaluator->add_inplace(comparisonResult,gt[i]);

                lhe->multiply_inplace(gt[0], eq[i]);
                lhe->relinearize_inplace(gt[0]);
                lhe->add_inplace(gt[0], gt[i]);
            }

        }
        

        return gt[0];

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

    // out = [ a[0]>b[0], a[1]>b[1], ... ]
    vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b){
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

    // out = [ a[0]>b[0], a[0]>b[1], ... ]
    vector<bool> verify(const vector<uint64_t>& a, const vector<vector<uint64_t>>& b){
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

    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
        for(int i=0;i<num_cmps;i++){
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }

    bool verify(const vector<uint64_t>& a, const vector<vector<uint64_t>>& b, const vector<uint64_t>& result){
        auto actural_result = verify(a,b);

        return verify(actural_result, result);
    }



    // input= [ b0,  b1,  b2 ]
    // out  = [ b00, b10, b20
    //          b01, b11, b21
    //          b02, b12, b22 ]
    // b0 = b00 + 2^m * b01 + (2^m)^2 * b02 ;
    // b1 = b10 + 2^m * b11 + (2^m)^2 * b12 ;
    // b2 = b20 + 2^m * b21 + (2^m)^2 * b22 ;
    vector<vector<uint64_t>> raw_encode(vector<uint64_t> in){
        vector<vector<uint64_t>> out(l);
        auto range = num_slots_per_element - 1;
        for(int i = 0 ; i < l; i++){
            auto offset = i * m;
            vector<uint64_t> temp(num_cmps, 0);
            for(int j = 0; j < in.size(); j++){
                temp[j] = (in[j] >> offset ) & range;
            }
            out[i] = temp;
        }
        return out;
    }

    // input = a
    // out = [a01, a02, a03, ...]
    // a = a00 + 2^m * a01 + (2^m)^2 * a02 ;
    vector<uint64_t> raw_encode(uint64_t a){
        vector<uint64_t> out(l, 0ULL);
        auto range = num_slots_per_element - 1;

        for(int i = 0 ; i < l; i++){
            out[i] = (a >> (i * m) ) & range;
        }
        return out;
    }

    // low to high
    vector<vector<uint64_t>> random_raw_encode_b(){
        //srand(time(NULL));
        vector<vector<uint64_t>> out(l);
        for(int i = 0; i < l ;i++){
            vector<uint64_t> temp(num_cmps, 0);
            for(int j = 0;j < num_cmps;j++){
                temp[j] = rand() % num_slots_per_element;  //rand 0  2^31-1
            }
            out[i] = temp;
        }
        return out;
    }

    // low to high
    vector<uint64_t> random_raw_encode_a(){
        srand(time(NULL));
        vector<uint64_t> out(l);
        for(int i = 0; i < l ;i++){
            out[i] = rand() % num_slots_per_element;  //rand 0  2^31-1
        }
        return out;
    }

    long communication_cost(const vector<Ciphertext>& ct1, const Ciphertext& ct2){
        stringstream send;
        long comm = 0;
        for(auto e:ct1){
            comm+=e.save(send);
        }
        comm+=ct2.save(send);
        return comm;
    }
    void print(){
        cout << " name                                      : " << name
             << " \n l                                         : "<< l 
             << " \n m                                         : "<< m
             << " \n bit precision                             : "<< n 
             << " \n compare number                            : "<< num_cmps
             <<endl ;
    }


};