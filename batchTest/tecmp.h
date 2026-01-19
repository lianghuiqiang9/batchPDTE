#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"
#include "cmp.h" // 包含基类定义

class Tecmp : public CMP {
public:
    uint8_t id; // 0x1 (Tree-based), 0x2 (Linear)
    int l, m, n;
    
    uint64_t num_slots_per_element; 
    uint64_t row_count;
    uint64_t num_cmps_per_row;
    
    Plaintext one_zero_zero;
    Ciphertext one_zero_zero_cipher;
    vector<uint64_t> index_map;
    inline static std::mt19937 gen{ std::random_device{}() };

    Tecmp(int l, int m, int tree_depth = 0, int extra = 0, uint8_t id = 0x1) {
        this->name = "tecmp";
        this->id = id;
        this->l = l;
        this->m = m;
        this->n = l * m;

        // 计算 HE 深度
        int cmp_depth_need = (this->id == 0x1) ? static_cast<int>(std::ceil(std::log2(l))) : l;
        if(((1 << cmp_depth_need) != l) && (this->id == 0x1)){
            throw std::invalid_argument("For tree-based Tecmp, l must be 2^x");
        }
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);
        
        // 参数初始化
        this->slot_count = lhe->slot_count;
        this->row_count = slot_count / 2;
        this->num_slots_per_element = 1 << m;
        this->num_cmps_per_row = row_count / num_slots_per_element;
        this->num_cmps = num_cmps_per_row * 2;

        // 索引映射初始化
        index_map.resize(num_cmps);
        for(uint64_t i = 0; i < num_cmps; i++) {
            bool flag = i < num_cmps_per_row;
            index_map[i] = flag ? (i * num_slots_per_element) : (row_count + (i - num_cmps_per_row) * num_slots_per_element);
        }

        one_zero_zero = init_one_zero_zero();
        one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    }

    // --- 实现 CmpBase 接口 ---

    // a_raw 格式: [l][num_cmps] 的矩阵。Tecmp 中同一批次的 A 通常是相同的
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a_raw) override {
        vector<Plaintext> out(l);
        for(int i = 0; i < l; i++) {
            // 将 A 的第 i 个分量编码到所有的槽位中，以便旋转使用
            vector<uint64_t> temp(slot_count, a_raw[i][0]); 
            out[i] = lhe->encode(temp);
        }
        return out;
    }

    vector<Ciphertext> encrypt_b(const vector<vector<uint64_t>>& b_raw) override {
        // 先进行 Thermometer Encoding (TE)
        size_t rows = b_raw.size(); // 应该是 l
        vector<vector<uint64_t>> te_encoded(rows, vector<uint64_t>(slot_count, 1ULL));

        for(size_t i = 0; i < rows; i++) {
            uint64_t* row_ptr = te_encoded[i].data();
            for(size_t j = 0; j < b_raw[i].size(); j++) {
                uint64_t start_idx = index_map[j];
                uint64_t theta = b_raw[i][j];
                std::fill_n(row_ptr + start_idx, theta + 1, 0ULL);
            }
        }

        // 加密
        vector<Ciphertext> out(rows);
        for(int i = 0; i < rows; i++) {
            out[i] = lhe->encrypt(te_encoded[i]);
        }
        return out;
    }

    Ciphertext great_than(vector<Plaintext>& a_pts, vector<Ciphertext>& b_cts) override {
        // 核心：从 Plaintext 中提取 A 的值作为旋转步数
        vector<uint64_t> a_vals(l);
        for(int i = 0; i < l; i++) {
            vector<uint64_t> decoded;
            lhe->batch_encoder->decode(a_pts[i], decoded);
            a_vals[i] = decoded[0]; // 假设本批次 A 相同
        }

        vector<Ciphertext> eq(l);
        vector<Ciphertext> gt(l);

        // 基础比较层
        for(int i = 0; i < l; ++i){
            gt[i] = lhe->rotate_rows(b_cts[i], a_vals[i]);
            if(a_vals[i] < num_slots_per_element - 1){
                eq[i] = lhe->rotate_rows(b_cts[i], a_vals[i] + 1);
            } else {
                eq[i] = one_zero_zero_cipher;
            }
            lhe->sub_inplace(eq[i], gt[i]); 
        }

        // 递归/线性组合
        if (id == 0x1) { // Tree-based
            int depth = log2(l);
            for(int i = 0; i < depth ; i++){
                int temp1 = 1 << i;
                int temp0 = 1 << (i + 1);
                for(int j = 0; j < l; j += temp0){                    
                    lhe->multiply_inplace(gt[j], eq[j + temp1]);
                    lhe->relinearize_inplace(gt[j]);
                    lhe->add_inplace(gt[j], gt[j + temp1]);
                    lhe->multiply_inplace(eq[j], eq[j + temp1]);
                    lhe->relinearize_inplace(eq[j]);
                }   
            }
        } else { // Linear
            for(int i = 1; i < l; i++){
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

    vector<uint64_t> decode_result(const Ciphertext& ct) override {
        auto pt = lhe->decrypt(ct);
        auto res = lhe->decode(pt);
        vector<uint64_t> ans(num_cmps);
        for(uint64_t i = 0; i < num_cmps; i++) {
            ans[i] = res[index_map[i]];
        }
        return ans;
    }

    // --- 其他辅助函数 ---

    Plaintext init_one_zero_zero(){
        vector<uint64_t> vec(slot_count, 0ULL);
        for(int i = 0; i < num_cmps ; i++) vec[index_map[i]] = 1ULL;
        return lhe->encode(vec);
    }

    void print() override {
        cout << " [Tecmp] l: " << l << ", m: " << m << ", prec: " << n 
             << ", parallels: " << num_cmps << endl;
    }

    // 生成测试数据
    vector<vector<uint64_t>> random_raw_encode_b() {
        vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps));
        std::uniform_int_distribution<uint64_t> dist(0, num_slots_per_element - 1);
        for (int i = 0; i < l; i++) {
            for (uint64_t j = 0; j < num_cmps; j++) out[i][j] = dist(gen);
        }
        return out; 
    }
};