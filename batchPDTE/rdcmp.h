#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"

class Rdcmp {
public:
    string name;
    int n;
    unique_ptr<LHE> lhe; 
    //uint64_t num_slots_per_element; // m_degree = (1<<m)
    uint64_t slot_count;
    uint64_t row_count;
    //uint64_t num_cmps_per_row;
    uint64_t num_cmps;
    Plaintext zero_zero_one;
    //Ciphertext one_zero_zero_cipher;

    Rdcmp(int n, int tree_depth = 0, int extra = 0) {
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


    // input
    // b = [ b00, b10, ...
    //       b01, b11, ...
    //       b02, b12, ...]
    // output
    // b = [ 1 - b00, 1 - b10, ...
    //       1 - b01, 1 - b11, ...
    //       1 - b02, 1 - b12, ...]
    vector<vector<uint64_t>> encode_b(const vector<vector<uint64_t>>& b){
        vector<vector<uint64_t>> out(n);
        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps, 0);
            for(int j = 0; j<num_cmps; j++){
                temp[j] = 1 - b[i][j]; 
            }
            out[i] = temp;
        }
        return out;
    }

    vector<Ciphertext> encrypt(const vector<vector<uint64_t>>& b){
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
    vector<vector<uint64_t>> encode_a(const vector<vector<uint64_t>>& a){
        return a;
    }

    
    // [ 1,0,0,...,1,0,0,...
    //   1,0,0,...,1,0,0,... ]
    Plaintext init_zero_zero_one(){
        vector<uint64_t> zero_zero_one(slot_count,1);
        zero_zero_one[num_cmps] = 0;
        return lhe->encode(zero_zero_one);
    }
    

    Ciphertext great_than(const vector<vector<uint64_t>>& a, vector<Ciphertext>& b){
        vector<Ciphertext> eq(n);
        vector<Ciphertext> gt(n);

        for(int i = 0; i < n; ++i){
            gt[i] = lhe->multiply_plain(b[i], a[i]);
        }
        for(int i = 0; i < n; ++i){
            eq[i] = lhe->add(gt[i], gt[i]);
            lhe->negate_inplace(eq[i]);
            lhe->add_inplace(eq[i], b[i]);
            lhe->add_plain_inplace(eq[i], a[i]);
        }

        int depth = log(n)/log(2);
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
        return gt[0];

    }

    void clear_up(Ciphertext& result){
        lhe->multiply_plain_inplace(result, zero_zero_one);
    }

    vector<uint64_t> decrypt(const Ciphertext& ct){
        auto pt = lhe->decrypt(ct);
        return lhe->decode(pt);
    }

    vector<uint64_t> decode(const std::vector<uint64_t>& res){
        return res;
    }

    vector<uint64_t> decode(const Ciphertext& ct){
        auto res = this->decrypt(ct);
        return this->decode(res);
    }

    // 
    vector<bool> verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b){
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

    bool verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
        for(int i=0;i<num_cmps;i++){
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }

    bool verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b, const vector<uint64_t>& result){
        auto actural_result = verify(a,b);
        return verify(actural_result, result);
    }


    // input= [ b0,  b1,  b2, ... ]
    // out  = [ b00, b10, b20, ...
    //          b01, b11, b21, ...
    //          b02, b12, b22, ... ]
    // b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
    // b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
    // b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
    vector<vector<uint64_t>> raw_encode(vector<uint64_t> in){
        vector<vector<uint64_t>> out;
        for(int i = 0 ; i < n; i++){
            vector<uint64_t> temp(slot_count, 0ULL);
            for(int j = 0; j < num_cmps; j++){
                temp[ j ] = (in[j] >> i ) & 1; 
            }
            temp[ num_cmps ] = 13; //Make sure plaintext is not padded with 0 , otherwise transparent error.
            out.push_back(temp);
        }
        return out;
    }


    // low to high  
    vector<vector<uint64_t>> random_raw_encode_b(){
        //srand(time(NULL));
        vector<vector<uint64_t>> out(n);
        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps, 0);
            for(int j = 0; j<num_cmps; j++){
                temp[j] = rand()%2; 
            }
            out[i] = temp;
        }
        return out;
    }

    // low to high
    vector<vector<uint64_t>> random_raw_encode_a(){
        srand(time(NULL));
        vector<vector<uint64_t>> out(n);
        for(int i = 0; i < n;i++){
            vector<uint64_t> temp(num_cmps,0);
            for(int j = 0; j<num_cmps; j++){
                temp[j] = rand()%2;  
            }
            out[i] = temp;
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
             << "\n bit precision                             : "<< n 
             << "\n compare number                            : "<< num_cmps
             << "\n";
    }


};