#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  // 必须包含
#include <cmath>   // 为了使用 log2
#include "lhe.h"

class Tecmp {
public:
    uint8_t id;
    int l, m;
    unique_ptr<LHE> lhe; 
    uint64_t m_degree;
    uint64_t num_slots_per_element;
    uint64_t row_count;
    uint64_t num_cmps_per_row;
    uint64_t num_cmps;

    Tecmp(int l, int m, int tree_depth = 0, int extra = 0, uint8_t id = 0x1) {
        this->id = id;
        this->l = l;
        this->m = m;

        int cmp_depth_need = (id == 0x1) ? static_cast<int>(std::ceil(std::log2(l))) : l;
        if(1<<cmp_depth_need !=l){
            cout<<" l must be 2^x, error"<<endl;
            exit(0);
        }

        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;


        this->lhe = make_unique<LHE>("bfv", total_depth);


        if (m >= this->lhe->log_poly_mod_degree - 1) {
            cout << "Error: params m is too large." << endl;
            exit(0);
        }
        num_slots_per_element = 1 << m;
        row_count = lhe->slot_count / 2;
        num_cmps_per_row = row_count / num_slots_per_element;
        num_cmps = num_cmps_per_row * 2;
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
    vector<vector<uint64_t>> encode_b(vector<vector<uint64_t>> b){

        vector<vector<uint64_t>> out;
        for(int i = 0 ; i < b.size(); i++){
            vector<uint64_t> temp(lhe->slot_count, 1ULL);
            for(int j = 0; j < b[i].size(); j++){
                //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
                //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
                bool flag = j < num_cmps_per_row; 
                uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
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

    // output
    // a = [ a00, 
    //       a01, 
    //       a02 ]
    // a = a00 + 2^m * a01 + (2^m)^2 * a02 ;

    vector<uint64_t> encode_a(){

    }

    

    // 调用时使用 this->lhe->encrypt(...)
};