#include"cwcmp.h"

CWEncoder::CWEncoder(uint64_t h, uint64_t n) : h(h), n(n) {
    this->l = find_best_l(h, n);
    precompute_combinations(l);
}

void CWEncoder::precompute_combinations(uint64_t max_l) {
    combo_table.assign(max_l + 1, std::vector<uint64_t>(max_l + 1, 0));
    for (uint64_t i = 0; i <= max_l; i++) {
        combo_table[i][0] = 1;
        for (uint64_t j = 1; j <= i; j++) {
            combo_table[i][j] = combo_table[i - 1][j - 1] + combo_table[i - 1][j];
        }
    }
}

uint64_t CWEncoder::nCr(uint64_t n, uint64_t r) {
    if (r > n) return 0;
    if (r == 0 || r == n) return 1;
    if (r > n / 2) r = n - r;
    uint64_t res = 1;
    for (uint64_t i = 1; i <= r; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res;
}

uint64_t CWEncoder::find_best_l(uint64_t h, uint64_t n) {
    if (n >= 64) {
        std::cerr << "Warning: n >= 64 may cause overflow in target calculation." << std::endl;
    }
    uint64_t target = 1ULL << n;
    uint64_t l_val = h;
    // 线性搜索最小的 l
    while (nCr(l_val, h) < target && l_val < 1000) {
        l_val++;
    }
    return l_val;
}

std::vector<uint64_t> CWEncoder::encode(int64_t x) const {
    if (x < 0) return std::vector<uint64_t>(l, 0);

    int64_t r = x;
    uint64_t h_prime = h;
    std::vector<uint64_t> y(l, 0);

    for (uint64_t l_prime = l; l_prime > 0; --l_prime) {
        uint64_t idx = l_prime - 1; 
        if (h_prime == 0) break;

        if (idx >= h_prime) {
            uint64_t threshold = combo_table[idx][h_prime];
            if ((uint64_t)r >= threshold) {
                y[idx] = 1;
                r -= threshold;
                h_prime--;
            }
        } else {
            y[idx] = 1;
            h_prime--;
        }
    }
    return y;
}


CWCMP::CWCMP(int n, int mh, int extra, bool is_rotate) {
    /////////////////////////////////
    this->scheme = "cwcmp";
    
    //l = 1 << static_cast<int>(std::ceil(std::log2(l)));
    //m = 1 << static_cast<int>(std::ceil(std::log2(m)));

    if(n>=64){std::cout<< "n: " << n << ", make sure the n < 64."<<std::endl;} // due to the constant-weight encode limit in uint64.
    
    //int h = 4;  // h = 2
    CWEncoder encoder(mh, n);
    this->encoder = encoder;

    this->l = encoder.l;
    this->m = 1;
    this->n = n;
    this->h = mh;

    auto d = (mh + 4)*(mh - 1)/2 + 1;
    int cmp_depth_need = static_cast<int>(std::ceil(std::log2(d)) + 1); 
    /////////////////////////////////

    depth = cmp_depth_need + extra;

    std::vector<int> steps;
    for (int i = 1; i < m; i<<=1) { // 1, 2, 3, ..., 2^m
        steps.push_back(i);
    }

    this->lhe = make_unique<BFV>(depth, steps, is_rotate);

    slot_count = lhe->slot_count;
    row_count = lhe->row_count;

    num_slots_per_element = m;

    num_cmps = slot_count / num_slots_per_element;

    num_cmps_per_row = (num_cmps + 1) / 2;

    one_zero_zero = init_one_zero_zero();
    zero_zero_zero_cipher = init_zero_zero_zero();
    one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
    h_factorial_inv_pt = init_h_factorial_inv_pt();
    h_vec_pt = init_h_vec();

}

vector<vector<uint64_t>> CWCMP::raw_encode_b(const vector<uint64_t>& b){
    return vector<vector<uint64_t>>{b};
}

vector<vector<uint64_t>> CWCMP::raw_encode_a(const vector<uint64_t>& a){
    return vector<vector<uint64_t>>{a};
}

vector<vector<uint64_t>> CWCMP::encode_b(const vector<vector<uint64_t>>& raw_b) {
    vector<vector<uint64_t>> out(l);
    for (int j = 0; j < l; ++j) {
        out[j].resize(num_cmps);
    }

    const vector<uint64_t>& input_vec = raw_b[0];
    for (uint64_t i = 0; i < num_cmps; ++i) {

        std::vector<uint64_t> a = encoder.encode(input_vec[i]);
        for (int j = 0; j < l; ++j) {
            out[j][i] = a[j];
        }
    }

    return out;
}

vector<vector<uint64_t>> CWCMP::decode_b(const vector<Ciphertext>& cts) {
    vector<vector<uint64_t>> decrypted_data(l);

    for (int i = 0; i < l; i++) {
        decrypted_data[i] = decrypt(cts[i]); 
    }
    vector<vector<uint64_t>> raw_b(l, vector<uint64_t>(num_cmps, 0));
    /**/
    return raw_b;
}

vector<vector<uint64_t>> CWCMP::encode_a(const vector<vector<uint64_t>>& raw_a){
    std::vector<uint64_t> out = encoder.encode(raw_a[0][0]);
    return vector<vector<uint64_t>>{out};;
}

Ciphertext CWCMP::piecewise_comp_rec(vector<Ciphertext>& b, vector<uint64_t>& a, uint64_t h) {
    int64_t c0 = -1;
    for (int64_t i = (int64_t)a.size() - 1; i >= 0; --i) {
        if (a[i] == 1) {
            c0 = i;
            break;
        }
    }

    if (c0 == -1){
        Ciphertext res;
        res = zero_zero_zero_cipher; 
        return res;
    } 

    Ciphertext alpha;
    if (c0 + 1 < (int64_t)b.size()) {
        std::vector<Ciphertext> higher_bits(b.begin() + c0 + 1, b.end());
        alpha = lhe->add_many(higher_bits);
        alpha = map_to_boolean(alpha, h); 
    } else {
        alpha = zero_zero_zero_cipher; 
    }

    if (h == 1) {
        return alpha;
    }

    std::vector<uint64_t> a_next(a.begin(), a.begin() + c0);
    std::vector<Ciphertext> b_next(b.begin(), b.begin() + c0);

    Ciphertext recursive_part = piecewise_comp_rec(b_next, a_next, h - 1);

    Ciphertext out = lhe->negate(alpha);
    lhe->add_plain_inplace(out, one_zero_zero); 

    lhe->multiply_inplace(out, b[c0]); 
    lhe->relinearize_inplace(out);
    
    lhe->multiply_inplace(out, recursive_part); 
    lhe->relinearize_inplace(out);
    
    lhe->add_inplace(out, alpha); // + alpha

    return out;
}

// a > E(b); // error
Ciphertext CWCMP::piecewise_comp_rec_GT_ab(vector<Ciphertext>& b, vector<uint64_t>& a, uint64_t h) {

    int64_t c0 = -1;
    for (int64_t i = (int64_t)a.size() - 1; i >= 0; --i) {
        if (a[i] == 1) { c0 = i; break; }
    }

    if (c0 == -1) {
        Ciphertext res = zero_zero_zero_cipher; 
        return res;
    }

    Ciphertext b_is_zero_higher;
    if (c0 + 1 < (int64_t)b.size()) {
        std::vector<Ciphertext> higher_bits(b.begin() + c0 + 1, b.end());
        Ciphertext sum_b_higher = lhe->add_many(higher_bits);
        b_is_zero_higher = map_to_boolean_zero(sum_b_higher, h); 
    } else {
        b_is_zero_higher = one_zero_zero_cipher; 
    }

    if (h == 1) return b_is_zero_higher;

    std::vector<uint64_t> a_next(a.begin(), a.begin() + c0);
    std::vector<Ciphertext> b_next(b.begin(), b.begin() + c0);
    Ciphertext recursive_part = piecewise_comp_rec_GT_ab(b_next, a_next, h - 1);

    Ciphertext out = lhe->negate(b_is_zero_higher);
    lhe->add_plain_inplace(out, one_zero_zero); // (1 - alpha)

    lhe->multiply_inplace(out, b[c0]); 
    lhe->relinearize_inplace(out);
    
    lhe->multiply_inplace(out, recursive_part); 
    lhe->relinearize_inplace(out);
    
    lhe->add_inplace(out, b_is_zero_higher); 

    return out;
}

// a < E(b)
Ciphertext CWCMP::great_than(vector<vector<uint64_t>>& raw_a, vector<Ciphertext>& b) {
    auto a = raw_a[0];
    //print_vector(a, l, "a:");

    return piecewise_comp_rec(b, a, h);

}

// 
vector<bool> CWCMP::verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) {
    vector<bool> out(num_cmps, false);
    for(uint64_t i = 0;i < num_cmps;i++){
        if (b[0][i] > a[0][0]){
            out[i] = true;
        }
    }
    return out;
}


vector<Plaintext> CWCMP::init_h_factorial_inv_pt(){
    vector<Plaintext> out(h);
    for(int i = 0; i < h; i++){
        auto h_factorial_inv = d_factorial_inv_with_sign(i+1, lhe->plain_modulus);
        //std::cout<<"h_factorial_inv: " << h_factorial_inv <<std::endl;
        vector<uint64_t> d_factorial_inv_vec(slot_count, h_factorial_inv);
        out[i] = lhe->encode(d_factorial_inv_vec);
    }
    return out;
}

vector<Plaintext> CWCMP::init_h_vec(){
    vector<Plaintext> temp(h);
    for(int i = 0; i < h; i++){
        vector<uint64_t> vec_temp(slot_count, i + 1);
        temp[i] = lhe->encode(vec_temp);
    }
    return temp;
}

//{1,...,h} -> 1, {0} -> 0
Ciphertext CWCMP::map_to_boolean(Ciphertext& a, uint64_t h){
    vector<Ciphertext> temp(h);
    for(uint64_t j = 0; j < h; ++j){
        temp[j] =  lhe->sub_plain(a, h_vec_pt[j]);
    }

    a = lhe->multiply_many(temp);
    lhe->multiply_plain_inplace(a, h_factorial_inv_pt[h-1]);
    lhe->negate_inplace(a);
    lhe->add_plain_inplace(a, one_zero_zero);

    return a;
}

//{1,...,h} -> 1, {0} -> 0
Ciphertext CWCMP::map_to_boolean_zero(Ciphertext& a, uint64_t h){
    vector<Ciphertext> temp(h);
    for(uint64_t j = 0; j < h; ++j){
        temp[j] =  lhe->sub_plain(a, h_vec_pt[j]);
    }

    a = lhe->multiply_many(temp);
    lhe->multiply_plain_inplace(a, h_factorial_inv_pt[h-1]);
    //lhe->negate_inplace(a);
    //lhe->add_plain_inplace(a, one_zero_zero);

    return a;
}

void CWCMP::print() {
    lhe->print();
    cout << " name                                     : " << scheme 
        << " \n depth                                    : "<< depth    
        << " \n CW.l                                     : "<< l
        << " \n CW.h                                     : "<< h
        << " \n bit precision (n)                        : "<< n 
        << " \n max batch nums                           : "<< num_cmps
        << endl;
}