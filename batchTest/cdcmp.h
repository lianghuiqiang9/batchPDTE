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

class Cdcmp : public CMP {
public:
    int n; // 位数精度
    uint64_t num_slots_per_element; 
    uint64_t row_count;
    uint64_t num_cmps_per_row;
    
    Plaintext one_zero_zero;
    vector<uint64_t> index_map;
    inline static std::mt19937 gen{ std::random_device{}() };

    Cdcmp(int n, int tree_depth = 0, int extra = 0) {
        this->name = "cdcmp";
        this->n = n;

        // 计算深度：Cdcmp 通常需要 log2(n) 次旋转和乘法
        int cmp_depth_need = static_cast<int>(std::ceil(std::log2(n)) + 1);
        int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)) + 1);
        int total_depth = cmp_depth_need + tree_depth_need + extra;

        this->lhe = make_unique<LHE>("bfv", total_depth);

        this->slot_count = lhe->slot_count;
        this->row_count = slot_count / 2;
        this->num_slots_per_element = n;
        this->num_cmps_per_row = row_count / num_slots_per_element;
        this->num_cmps = num_cmps_per_row * 2;

        // 初始化 index_map
        index_map.resize(num_cmps);
        for(uint64_t i = 0; i < num_cmps; i++) {
            bool flag = i < num_cmps_per_row;
            index_map[i] = flag ? (i * num_slots_per_element) : (row_count + (i - num_cmps_per_row) * num_slots_per_element);
        }

        one_zero_zero = init_one_zero_zero();
    }

    ~Cdcmp() override = default;

    // --- 实现 CmpBase 接口 ---

    // a_raw 格式: [n][num_cmps] 的位矩阵
    vector<Plaintext> encode_a(const vector<vector<uint64_t>>& a_raw) override {
        // Cdcmp 需要将位矩阵平铺到单个 Plaintext 的槽位中
        vector<uint64_t> flattened(slot_count, 0ULL);
        for(uint64_t i = 0; i < num_cmps; i++){
            uint64_t base_offset = index_map[i];
            for(int j = 0; j < n; j++){
                flattened[base_offset + j] = a_raw[j][i];
            }
        }
        return { lhe->encode(flattened) }; // 返回包含单个 PT 的 vector
    }

    // b_raw 格式: [n][num_cmps] 的位矩阵
    vector<Ciphertext> encrypt_b(const vector<vector<uint64_t>>& b_raw) override {
        vector<uint64_t> flattened_neg_b(slot_count, 0ULL);
        for(uint64_t i = 0; i < num_cmps; i++){
            uint64_t base_offset = index_map[i];
            for(int j = 0; j < n; j++){
                // 内部处理 1 - b
                flattened_neg_b[base_offset + j] = 1 - b_raw[j][i];
            }
        }
        return { lhe->encrypt(flattened_neg_b) }; // 返回包含单个 CT 的 vector
    }

    // 核心比较算法：利用旋转在单个密文中移动位
    Ciphertext great_than(vector<Plaintext>& a, vector<Ciphertext>& b) override {
        // a[0] 是平铺后的 A 位，b[0] 是平铺后的 1-B 位
        auto& pt_a = a[0];
        auto& ct_neg_b = b[0];

        // GT = a * (1-b)
        auto gt = lhe->multiply_plain(ct_neg_b, pt_a);
        
        // EQ = a + (1-b) - 2 * a * (1-b)
        auto eq = lhe->add(gt, gt);
        lhe->negate_inplace(eq);
        lhe->add_inplace(eq, ct_neg_b);
        lhe->add_plain_inplace(eq, pt_a);

        // 通过旋转进行位规约
        int depth = static_cast<int>(std::log2(n));
        for(int i = 0; i < depth ; i++){
            int step = 1 << i;

            auto gt_temp = lhe->rotate_rows(gt, step);
            auto eq_temp = lhe->rotate_rows(eq, step);
            
            lhe->multiply_inplace(eq, eq_temp);
            lhe->relinearize_inplace(eq);
            
            lhe->multiply_inplace(gt, eq_temp);
            lhe->relinearize_inplace(gt);
            lhe->add_inplace(gt, gt_temp);
        }
        return gt;
    }

    void clear_up(Ciphertext& result) override {
        // 只保留每个数字比较结果所在的首位 (MSB 规约后的位置)
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

    void print() override {
        cout << " [Cdcmp] Single CT mode, Precision: " << n << " bits, Parallels: " << num_cmps << endl;
    }

    // --- 辅助工具 ---

    Plaintext init_one_zero_zero(){
        vector<uint64_t> mask(slot_count, 0ULL);
        for(int i = 0; i < num_cmps ; i++){
            mask[index_map[i]] = 1ULL;
        }
        return lhe->encode(mask);
    }
};