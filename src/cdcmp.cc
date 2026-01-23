#include"cdcmp.h"

Cdcmp::Cdcmp(int l, int m, int n, int extra) {
    this->scheme = "cdcmp";
    this->n = n;

    int cmp_depth_need =  static_cast<int>(std::ceil(std::log2(n)) + 1); 
    depth = cmp_depth_need + extra;

    std::vector<int> steps;
    for (int i = 1; i < n; i<<=1) { // 1, 2, 3, ..., 2^m
        steps.push_back(i);
    }
    
    cout<<" steps.size(): "<< steps.size() << endl;
    for(auto e:steps){
        cout<<e<<" ";
    }
    cout<<"steps"<<endl;

    this->lhe = make_unique<BFV>(depth, steps);

    slot_count = lhe->slot_count;
    row_count = slot_count / 2;
    num_slots_per_element = n;
    num_cmps_per_row = row_count / num_slots_per_element;
    num_cmps = num_cmps_per_row * 2;

    // index_map
    //idx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
    //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
    index_map.resize(num_cmps);
    for(uint64_t i = 0; i < num_cmps; i++) {
        bool flag = i < num_cmps_per_row;
        index_map[i] = flag ? (i * num_slots_per_element) : (row_count + (i - num_cmps_per_row) * num_slots_per_element);
    }

    one_zero_zero = init_one_zero_zero();
}


// input
// b = [ b00, b01, b02, ..., b10, b11, b12, ...]
// output
// neg_b = [ 1 - b00, 1 - b01, 1 - b02, ..., 1 - b10, 1 - b11, 1 - b12, ...]
vector<vector<uint64_t>> Cdcmp::encode_b(const vector<vector<uint64_t>>& raw_b) {
    auto b = raw_b[0]; 
    vector<uint64_t> out(slot_count, 0ULL); //_encode_process(raw_b);
    for(size_t i = 0 ; i < out.size(); i++){
        out[i] = 1 - b[i];
    }
    return vector<vector<uint64_t>>{out};
}

/*
vector<uint64_t> Cdcmp::_encode_process(const vector<vector<uint64_t>>& raw_b){
    auto b = raw_b[0];
    vector<uint64_t> out(slot_count, 0ULL);     
    size_t num_elements = std::min((size_t)num_cmps, b.size() / n);
    for (size_t i = 0; i < num_elements; ++i) {
        size_t target_base_pos = index_map[i];
        size_t source_base_pos = i * n;
        for (int j = 0; j < n; ++j) {
            out[target_base_pos + j] = b[source_base_pos + j];
        }
    }
    return out;
}
*/

vector<Ciphertext> Cdcmp::encrypt(const vector<vector<uint64_t>>& raw_b) {
    auto out = lhe->encrypt(raw_b[0]);
    return vector<Ciphertext>{out};
}

// input
// a = [ a00, a01, a02 ]
// output
// a = [ a00, a01, a02 ]
vector<Plaintext> Cdcmp::encode_a(const vector<vector<uint64_t>>& raw_a){
    //auto temp = _encode_process(raw_a);
    auto out = lhe->encode(raw_a[0]);
    return vector<Plaintext>{out};
}

// [ 1,0,0,...,1,0,0,...
//   1,0,0,...,1,0,0,... ]
Plaintext Cdcmp::init_one_zero_zero(){
    vector<uint64_t> one_zero_zero(slot_count, 0ULL);
    for(size_t i = 0; i < num_cmps ; i++){
        one_zero_zero[index_map[i]] = 1ULL;
    }
    return lhe->encode(one_zero_zero);
}

Plaintext Cdcmp::init_x_zero_zero(const vector<uint64_t>& salt) {
    vector<uint64_t> salt_zero_zero(slot_count, 0ULL);
    auto salt_size = salt.size();
    auto limit = num_cmps > salt_size ? salt_size : num_cmps;
    for(size_t i = 0; i < limit ; i++){
        salt_zero_zero[index_map[i]] = salt[i];
    }
    return lhe->encode(salt_zero_zero);
}

Ciphertext Cdcmp::great_than(vector<Plaintext>& raw_a, vector<Ciphertext>& raw_b) {
    auto a = raw_a[0];
    auto b = raw_b[0];

    auto gt = lhe->multiply_plain(b, a);
    auto eq = lhe->add(gt, gt);
    lhe->negate_inplace(eq);
    lhe->add_inplace(eq, b);
    lhe->add_plain_inplace(eq, a);


    int depth = static_cast<int>(std::log2(n));
    for(int i = 0; i < depth ; i++){
        int step = 1<<i;

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

void Cdcmp::clear_up(Ciphertext& result) {
    lhe->multiply_plain_inplace(result, one_zero_zero);
}

vector<uint64_t> Cdcmp::decrypt(const Ciphertext& ct) {
    auto pt = lhe->decrypt(ct);
    return lhe->decode(pt);
}

vector<uint64_t> Cdcmp::decode(const std::vector<uint64_t>& res){
    vector<uint64_t> ans(num_cmps);
    for(uint64_t i = 0; i < num_cmps ; i++){
        ans[i] = res[index_map[i]];
    }
    return ans;
}

vector<uint64_t> Cdcmp::recover(const Ciphertext& ct) {
    auto res = this->decrypt(ct);
    return this->decode(res);
}

// out = [ a[0]>b[0], a[0]>b[1], ... ]
vector<bool> Cdcmp::verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& raw_b) {
    auto a = raw_a[0];
    auto b = raw_b[0];

    vector<bool> out(num_cmps, false);
    for(size_t i = 0; i < num_cmps ; ++i){
        int offset = i*n;
        for(int k = offset + n - 1;k >= offset; --k){
            if(a[k] > b[k]){
                out[i] = true;
                break;
            }else if(a[k] == b[k]){
                continue;
            }else{
                break;
            }
        }
    }
    return out;
}

// input= [ b0,  b1,  b2 ]
// out  = [ b00, b01, b02, ..., b10, b11, b12, ..., b20, b21, b22, ... ]
// b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
// b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
// b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
vector<vector<uint64_t>> Cdcmp::raw_encode_b(const vector<uint64_t>& b) {
    vector<uint64_t> out(slot_count, 0ULL);
    size_t actual_process_size = std::min(b.size(), num_cmps);

    for (size_t i = 0; i < actual_process_size; ++i) {
        uint64_t val = b[i];
        size_t base_offset = i * n;
        for (int j = 0; j < n; ++j) {
            out[base_offset + j] = (val >> j) & 1;
        }
    }
    
    return vector<vector<uint64_t>>{out};
}

vector<vector<uint64_t>> Cdcmp::raw_encode_a(const vector<uint64_t>& in) {
    return raw_encode_b(in);
}


// low to high  
vector<vector<uint64_t>> Cdcmp::random_raw_encode_b() {
    static std::uniform_int_distribution<uint64_t> dist(0, 1);
    vector<uint64_t> out(slot_count);
    std::generate(out.begin(), out.end(), [&]() { return dist(gen); });
    return vector<vector<uint64_t>>{out};
}

// low to high
vector<vector<uint64_t>> Cdcmp::random_raw_encode_a() {
    static std::uniform_int_distribution<uint64_t> dist(0, 1);
    vector<uint64_t> out(slot_count);
    std::generate(out.begin(), out.end(), [&]() { return dist(gen); });
    return vector<vector<uint64_t>>{out};
}

void Cdcmp::print() {
    cout << " name                                     : " << scheme
            << " \n lhe name                                 : "<< lhe->scheme          
            << " \n max depth                                : "<< depth
            << " \n bit precision (n)                        : "<< n 
            << " \n compare number                           : "<< num_cmps
            << " \n";
}