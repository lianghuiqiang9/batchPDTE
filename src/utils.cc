#include"utils.h"

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


vector<vector<uint64_t>> transpose(const vector<vector<uint64_t>>& A) {
    if (A.empty()) return {};
    size_t rows = A.size();
    size_t cols = A[0].size();

    vector<vector<uint64_t>> B(cols);
    for (size_t i = 0; i < cols; ++i) {
        B[i].resize(rows);
    }

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            B[j][i] = A[i][j];
        }
    }
    return B;
}


std::vector<uint64_t> random_k_bit(int k, uint64_t count) {
    if (k < 1) return std::vector<uint64_t>(count, 0);
    k = std::min(k, 64);
    static std::random_device rd;
    static std::mt19937_64 gen(rd());

    std::vector<uint64_t> results(count);
    uint64_t max_val = (k == 64) ? ~0ULL : (1ULL << k) - 1;
    std::uniform_int_distribution<uint64_t> dis(0, max_val);

    for (uint64_t i = 0; i < count; ++i) {
        results[i] = dis(gen);
    }
    return results;
}

std::vector<uint64_t> random_permutation(int n) {
    if (n <= 0) {
        throw std::invalid_argument("n must be positive");
    }

    std::vector<uint64_t> a(n);
    std::iota(a.begin(), a.end(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::shuffle(a.begin(), a.end(), gen);

    return a;
}

uint64_t factorial(uint64_t n) {

    if(n==1)
        return 1;
    return n * factorial(n-1);
}

// a ^ e mod n
uint64_t mod_exp(uint64_t a, uint64_t e, uint64_t n)
{
    if (e == 0)
        return 1;

    long k = ceil(log2(e));
    uint64_t res;
    res = 1;

    for (long i = k - 1; i >= 0; i--) {
        res = (res * res) % n;
        if ((e / (uint64_t)(pow(2, i))) % 2 == 1)
            res = (res * a) % n;
    }
    return res;
}


uint64_t prime_mod_inverse(uint64_t a, uint64_t n)
{
    return mod_exp(a, n - 2, n);
}

// \theta = 1 / d! \prod ^{d}_{i = 1) (i - x )
// if i = {1,...,d}, the \theta = 0; if i = 0, the \theta = 1;    
uint64_t d_factorial_inv_with_sign(uint64_t d, uint64_t mod){
    uint64_t d_factorial_inv = prime_mod_inverse(factorial(d), mod);
    uint64_t d_factorial_inv_with_sign = (d % 2) ? mod - d_factorial_inv : d_factorial_inv;
    return d_factorial_inv_with_sign;
}