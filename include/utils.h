#ifndef __UTILS__
#define __UTILS__
#include<iostream>
#include<vector>
#include<random>
#include<algorithm>
#include<chrono>
using namespace std;

template <typename Func>
auto profile(string name, Func&& func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << name << ": " << duration.count() << " us" << endl;
    return (float)duration.count();
}


template <typename T>
void print_vec(const std::vector<T> &vec, size_t k, std::string name = "Resutl: ") {
    if (vec.size()==0){
        return;
    }

    k = std::min(k, vec.size());
    cout << name << " [ ";
    for (size_t i = 0; i < k; i++) {
        cout << vec[i] << (i == k - 1 ? "" : ", ");

        if ((i + 1) % k == 0 ) {
            cout << " ] (first " << k << " slots):" << endl;
        }
    }
}

template <typename T>
void print_vec(const std::vector<std::vector<T>> &matrix, size_t k, std::string name = "Resutl: ") {
    if (matrix.size()==0){
        return;
    }
    auto limit = matrix.size()>100? 100 : matrix.size();
    cout << name;
    for (size_t i = 0;i < limit;i++){
        print_vec(matrix[i], k, "");
    }
    if (matrix.size()>100){
        cout<< "..., "<<matrix.size()<<" rows." <<endl;
    }
}

vector<uint64_t> add(const vector<uint64_t> &a, const vector<uint64_t> &b, uint64_t mod);

vector<uint64_t> mul(const vector<uint64_t> &a, const vector<uint64_t> &b, uint64_t mod);

vector<uint64_t> scalar_mul(const vector<uint64_t> &a, uint64_t s, uint64_t mod);

vector<uint64_t> rotate(const vector<uint64_t> &a, int k);

vector<vector<uint64_t>> transpose(const vector<vector<uint64_t>>& A);

std::vector<uint64_t> random_k_bit(int k, uint64_t count);

std::vector<uint64_t> random_permutation(int n);

uint64_t factorial(uint64_t n);

uint64_t mod_exp(uint64_t a, uint64_t e, uint64_t n);

uint64_t prime_mod_inverse(uint64_t a, uint64_t n);

uint64_t d_factorial_inv_with_sign(uint64_t d, uint64_t mod);

uint64_t get_nearest_power_of_two(uint64_t n);

#endif