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
    uint64_t num_cmps;
    uint64_t slot_count;
    unique_ptr<LHE> lhe;

    virtual ~CMP() = default;

    // --- 核心生命周期接口 ---

    // 将 A 的原始矩阵编码为明文序列 (Tecmp 视为旋转步数，Rdcmp/Cdcmp 视为位明文)
    virtual vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a_raw) = 0;
    
    // 编码并加密 B 的原始矩阵
    virtual vector<Ciphertext> encrypt_b(const vector<vector<uint64_t>>& b_raw) = 0;

    // 核心比较逻辑：返回单一密文结果
    virtual Ciphertext great_than(vector<Plaintext>& a_pts, vector<Ciphertext>& b_cts) = 0;

    // 提取有效槽位
    virtual void clear_up(Ciphertext& result) = 0;

    // 解密并返回 0/1 数组
    virtual vector<uint64_t> decode_result(const Ciphertext& ct) = 0;

    // --- 工具接口 ---
    virtual void print() = 0;

    long communication_cost(const vector<Ciphertext>& b_cts, const Ciphertext& res_ct) {
        long comm = 0;
        for (const auto& c : b_cts) comm += c.save_size();
        comm += res_ct.save_size();
        return comm;
    }
};