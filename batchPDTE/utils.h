#pragma once
#include<iostream>
#include<vector>
using namespace std;

template <typename Func>
auto profile(string name, Func&& func) {
    auto start = chrono::high_resolution_clock::now();
    func();
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << name << ": " << duration.count() << " us" << endl;
    return duration.count();
}

template <typename T>
void print(const std::vector<T> &vec, size_t k, std::string name = "Result") {
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
void print(const std::vector<std::vector<T>> &matrix, size_t k, std::string name = "Result") {
    if (matrix.size()==0){
        return;
    }
    cout << name;
    for (int i = 0;i<matrix.size();i++){
        print(matrix[i], k, "");
    }

}
/*
template <typename T>
void print(const T &a, size_t k = 1, std::string name = "Result") {
    
    cout << "[ "<< a << " ] (first " << k << " slots):" << endl;

}
*/
vector<uint64_t> add(const vector<uint64_t> &a, const vector<uint64_t> &b, uint64_t mod) {
    if (a.size() != b.size()) throw invalid_argument("Vector sizes must match.");
    
    vector<uint64_t> result(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        // (a + b) % mod
        result[i] = (a[i] + b[i]) % mod;
    }
    return result;
}

vector<uint64_t> mul(const vector<uint64_t> &a, const vector<uint64_t> &b, uint64_t mod) {
    if (a.size() != b.size()) throw invalid_argument("Vector sizes must match.");
    
    vector<uint64_t> result(a.size());
    for (size_t i = 0; i < a.size(); i++) {
        unsigned __int128 temp = static_cast<unsigned __int128>(a[i]) * b[i];
        result[i] = static_cast<uint64_t>(temp % mod);
    }
    return result;
}

vector<uint64_t> scalar_mul(const vector<uint64_t> &a, uint64_t s, uint64_t mod) {
    vector<uint64_t> result(a.size());

    uint64_t scalar = s % mod;

    for (size_t i = 0; i < a.size(); i++) {
        unsigned __int128 temp = static_cast<unsigned __int128>(a[i]) * scalar;
        result[i] = static_cast<uint64_t>(temp % mod);
    }
    return result;
}

vector<uint64_t> rotate(const vector<uint64_t> &a, int k) {
    if (a.empty()) return a;
    size_t n = a.size();
    size_t half = n / 2;
    vector<uint64_t> result(n);

    int shift = k % (int)half;
    if (shift < 0) shift += half; 

    size_t s = static_cast<size_t>(shift);

    for (size_t i = 0; i < half; i++) {
        result[(i + half - s) % half] = a[i];
        
        result[((i + half - s) % half) + half] = a[i + half];
    }
    
    return result;
}