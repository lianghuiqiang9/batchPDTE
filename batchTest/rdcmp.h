#pragma once
#include <seal/seal.h>
#include <vector>
#include <string>
#include <memory>  
#include <cmath> 
#include "lhe.h"
#include "cmp.h"

using namespace std;
using namespace seal;

class Rdcmp : public CMP {
public:
    int n; // 位数精度
    Plaintext zero_zero_one;
    inline static std::mt19937 gen{ std::random_device{}() };

    Rdcmp(int n, int tree_depth = 0, int extra = 0) {
        this->name = "rdcmp";
        this->n = n;

        // 计算所需的 HE 方案深度
        // 比较逻辑深度约为 log2(n) + 1
        int cmp_depth_need = static_cast<int>(std::ceil(std::log2(n)) + 1);
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);

        this->slot_count = lhe->slot_count;
        // 保留一个位置作为 padding 位，防止明文全为 0
        this->num_cmps = slot_count - 1; 

        // 初始化结果提取用的 mask
        zero_zero_one = init_zero_zero_one();
    }

    ~Rdcmp() override = default;

    // --- 实现 CmpBase 接口 ---

    // a_raw: [n][num_cmps] 的位矩阵
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a_raw) override {
        vector<Plaintext> out(n);
        for(int i = 0; i < n; ++i) {
            // a_raw[i] 已经是第 i 位的向量
            out[i] = lhe->encode(a_raw[i]);
        }
        return out;
    }

    // b_raw: [n][num_cmps] 的位矩阵
    vector<Ciphertext> encrypt_b(const vector<vector<uint64_t>>& b_raw) override {
        vector<Ciphertext> out(n);
        for(int i = 0; i < n; i++) {
            // Rdcmp 内部逻辑通常处理 1 - b
            vector<uint64_t> neg_b(slot_count, 0);
            for(size_t j = 0; j < num_cmps; j++) {
                neg_b[j] = 1 - b_raw[i][j];
            }
            out[i] = lhe->encrypt(neg_b);
        }
        return out;
    }

    // 核心比较算法实现
    Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) override {
        vector<Ciphertext> eq(n);
        vector<Ciphertext> gt(n);

        // 第一层：计算每一位的 GT 和 EQ
        // GT_i = a_i * (1 - b_i)
        // EQ_i = a_i + (1 - b_i) - 2 * a_i * (1 - b_i)
        for(int i = 0; i < n; ++i){
            gt[i] = lhe->multiply_plain(b[i], a[i]); // 这里 b 已经是加密的 (1-b)
            eq[i] = lhe->add(gt[i], gt[i]);
            lhe->negate_inplace(eq[i]);
            lhe->add_inplace(eq[i], b[i]);
            lhe->add_plain_inplace(eq[i], a[i]);
        }

        // 树状规约计算最终结果
        // Result = GT_{n-1} + EQ_{n-1} * (GT_{n-2} + EQ_{n-2} * (...))
        int depth = static_cast<int>(log2(n));
        for(int i = 0; i < depth ; ++i){
            int temp1 = 1 << i;
            int temp0 = 1 << (i + 1);
            for(int j = 0; j < n; j = j + temp0){
                // gt[j] = gt[j] * eq[j+temp1] + gt[j+temp1]
                lhe->multiply_inplace(gt[j], eq[j + temp1]);
                lhe->relinearize_inplace(gt[j]);
                lhe->add_inplace(gt[j], gt[j + temp1]);
                
                // eq[j] = eq[j] * eq[j+temp1]
                lhe->multiply_inplace(eq[j], eq[j + temp1]);
                lhe->relinearize_inplace(eq[j]);
            }   
        }
        return std::move(gt[0]);
    }

    void clear_up(Ciphertext& result) override {
        // 过滤掉 padding 位，只保留 num_cmps 个结果
        lhe->multiply_plain_inplace(result, zero_zero_one);
    }

    vector<uint64_t> decode_result(const Ciphertext& ct) override {
        auto pt = lhe->decrypt(ct);
        auto res = lhe->decode(pt);
        // 只返回前 num_cmps 个有效结果
        res.resize(num_cmps);
        return res;
    }

    void print() override {
        cout << " [Rdcmp] Precision: " << n << " bits, Parallels: " << num_cmps << endl;
    }

    // --- 辅助函数 ---

    Plaintext init_zero_zero_one() {
        vector<uint64_t> mask(slot_count, 1);
        mask[num_cmps] = 0; // Padding 位置零
        return lhe->encode(mask);
    }

    // 随机生成位矩阵数据 [n][num_cmps]
    vector<vector<uint64_t>> random_raw_encode_matrix() {
        vector<vector<uint64_t>> out(n, vector<uint64_t>(num_cmps));
        std::uniform_int_distribution<uint64_t> dist(0, 1);
        for (int i = 0; i < n; i++) {
            for (uint64_t j = 0; j < num_cmps; j++) {
                out[i][j] = dist(gen);
            }
        }
        return out;
    }
};