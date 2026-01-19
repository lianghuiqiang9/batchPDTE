#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <iomanip>

// 包含所有算法头文件
#include "tecmp.h"
#include "rdcmp.h"
#include "cdcmp.h"
#include "cmp.h"
using namespace std;

/**
 * 辅助函数：生成模拟数据
 * 返回格式为 [维度][比较数量] 的位矩阵或系数矩阵
 */
vector<vector<uint64_t>> generate_test_data(uint64_t dim, uint64_t num_cmps, uint64_t max_val) {
    vector<vector<uint64_t>> data(dim, vector<uint64_t>(num_cmps));
    std::mt19937 gen(12345); // 固定种子以便复现
    std::uniform_int_distribution<uint64_t> dist(0, max_val);

    for (uint64_t i = 0; i < dim; ++i) {
        for (uint64_t j = 0; j < num_cmps; ++j) {
            data[i][j] = dist(gen);
        }
    }
    return data;
}

/**
 * 性能测试核心逻辑
 */
void run_benchmark(unique_ptr<CMP>& engine, int dim, uint64_t max_val) {
    cout << "\n" << string(50, '=') << endl;
    engine->print();
    cout << string(50, '-') << endl;

    uint64_t num_cmps = engine->num_cmps;

    // 1. 准备原始数据
    auto raw_a = generate_test_data(dim, num_cmps, max_val);
    auto raw_b = generate_test_data(dim, num_cmps, max_val);

    // 2. 编码与加密阶段
    auto start_enc = chrono::high_resolution_clock::now();
    
    auto a_pts = engine->encode_a(raw_a);
    auto b_cts = engine->encrypt_b(raw_b);
    
    auto end_enc = chrono::high_resolution_clock::now();
    auto dur_enc = chrono::duration_cast<chrono::milliseconds>(end_enc - start_enc);

    // 3. 密文比较阶段 (核心性能指标)
    auto start_cmp = chrono::high_resolution_clock::now();
    
    Ciphertext result_ct = engine->great_than(a_pts, b_cts);
    engine->clear_up(result_ct);
    
    auto end_cmp = chrono::high_resolution_clock::now();
    auto dur_cmp = chrono::duration_cast<chrono::milliseconds>(end_cmp - start_cmp);

    // 4. 解密与验证阶段
    auto decrypted_res = engine->decode_result(result_ct);
    
    int errors = 0;
    for (uint64_t j = 0; j < num_cmps; ++j) {
        // 明文验证逻辑 (从高位到低位比较)
        bool expected = false;
        for (int i = dim - 1; i >= 0; --i) {
            if (raw_b[i][j] > raw_a[i][j]) { expected = true; break; }
            if (raw_b[i][j] < raw_a[i][j]) { expected = false; break; }
        }
        if (decrypted_res[j] != (expected ? 1 : 0)) errors++;
    }

    // 5. 输出结果
    long comm_size = engine->communication_cost(b_cts, result_ct);

    cout << left << setw(25) << "Encryption Time:" << dur_enc.count() << " ms" << endl;
    cout << left << setw(25) << "Computation Time:" << dur_cmp.count() << " ms" << endl;
    cout << left << setw(25) << "Communication Cost:" << fixed << setprecision(2) << comm_size / 1024.0 << " KB" << endl;
    cout << left << setw(25) << "Amortized (per cmp):" << (double)dur_cmp.count() / num_cmps << " ms" << endl;
    cout << left << setw(25) << "Verification:" << (errors == 0 ? "PASSED" : "FAILED (" + to_string(errors) + " errors)") << endl;
    cout << string(50, '=') << endl;
}

int main(int argc, char* argv[]) {
    // 默认参数
    int n = 16;      // 位精度 (Rdcmp, Cdcmp 使用)
    int l = 4, m = 4; // 分解参数 (Tecmp 使用, l*m = 16)
    
    string choice = "all";
    if (argc > 1) choice = argv[1];

    try {
        // 测试 Rdcmp
        if (choice == "all" || choice == "rdcmp") {
            unique_ptr<CMP> rdcmp = make_unique<Rdcmp>(n);
            run_benchmark(rdcmp, n, 1); // 位分解 max_val 为 1
        }

        // 测试 Cdcmp
        if (choice == "all" || choice == "cdcmp") {
            unique_ptr<CMP> cdcmp = make_unique<Cdcmp>(n);
            run_benchmark(cdcmp, n, 1); // 位分解 max_val 为 1
        }

        // 测试 Tecmp
        if (choice == "all" || choice == "tecmp") {
            unique_ptr<CMP> tecmp = make_unique<Tecmp>(l, m);
            run_benchmark(tecmp, l, (1 << m) - 1); // 系数分解 max_val 为 2^m - 1
        }

    } catch (const std::exception& e) {
        cerr << "Runtime Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}