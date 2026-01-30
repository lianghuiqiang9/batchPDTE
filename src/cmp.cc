#include"cmp.h"


vector<Ciphertext> CMP::encrypt(const vector<vector<uint64_t>>& b) {
    vector<Ciphertext> out(b.size());
    for(size_t i = 0 ; i < b.size(); i++){
        out[i] = lhe->encrypt(b[i]);
    }
    return out;
}

void CMP::clear_up(Ciphertext& result)  {
    lhe->multiply_plain_inplace(result, one_zero_zero);
}

vector<uint64_t> CMP::decrypt(const Ciphertext& ct) {
    auto pt = lhe->decrypt(ct);
    return lhe->decode(pt);
}

vector<uint64_t> CMP::decode(const std::vector<uint64_t>& res){
    vector<uint64_t> ans(num_cmps);
    for(uint64_t i = 0; i < num_cmps ; i++){
        ans[i] = res[ i * num_slots_per_element];
    }
    return ans;
}

vector<uint64_t> CMP::recover(const Ciphertext& ct) {
    auto res = this->decrypt(ct);
    return this->decode(res);
}

// [ 1,0,0,...,1,0,0,...
//   1,0,0,...,1,0,0,... ]
Plaintext CMP::init_one_zero_zero(){
    vector<uint64_t> one_zero_zero(slot_count, 0ULL);
    for(uint64_t i = 0; i < num_cmps ; i++){
        one_zero_zero[i * num_slots_per_element] = 1ULL;
    }
    return lhe->encode(one_zero_zero);
}

Plaintext CMP::init_x_zero_zero(const vector<uint64_t>& x) {
    vector<uint64_t> x_zero_zero(slot_count, 0ULL);
    auto x_size = x.size();
    auto limit = num_cmps > x_size ? x_size : num_cmps;
    for(size_t i = 0; i < limit ; i++){
        x_zero_zero[i * num_slots_per_element] = x[i];
    }
    return lhe->encode(x_zero_zero);
}


// input= [ b0,  b1,  b2 ]
// out  = [ b00, b01, b02, ..., b10, b11, b12, ..., b20, b21, b22, ... ]
// b0 = b00 + 2 * b01 + 2^m * b02 + ...;
// b1 = b10 + 2 * b11 + 2^m * b12 + ...;
// b2 = b20 + 2 * b21 + 2^m * b22 + ...;
vector<vector<uint64_t>> CMP::raw_encode_b(const vector<uint64_t>& b) {
    vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps, 0));
    const uint64_t range = (1 << m) - 1;
    const size_t b_size = b.size();

    for(int i = 0; i < l; i++) {
        const auto offset = i * m;
        uint64_t* out_ptr = out[i].data();
        const uint64_t* in_ptr = b.data();
        for(size_t j = 0; j < b_size; j++) {
            out_ptr[j] = (in_ptr[j] >> offset) & range;
        }
    }
    return out;
}

// low to high  
vector<vector<uint64_t>> CMP::random_raw_encode_b() {
    vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps));
    std::uniform_int_distribution<uint64_t> dist(0, (1<<m) - 1);

    for (int i = 0; i < l; i++) {
        uint64_t* row_ptr = out[i].data(); 
        for (uint64_t j = 0; j < num_cmps; j++) {
            row_ptr[j] = dist(gen);
        }
    }
    return out; 
}

vector<vector<uint64_t>> CMP::raw_encode_a(const vector<uint64_t>& in) {
    return raw_encode_b(in);
}

// low to high
vector<vector<uint64_t>> CMP::random_raw_encode_a() {
    return random_raw_encode_b();
}


vector<uint64_t> CMP::raw_decode_b(const vector<vector<uint64_t>>& encoded_out, size_t original_b_size) {
    vector<uint64_t> b(original_b_size, 0);
    for (size_t j = 0; j < original_b_size; j++) {
        uint64_t restored_val = 0;
        for (int i = 0; i < l; i++) {
            const auto offset = i * m;
            uint64_t slice = encoded_out[i][j];
            restored_val |= (slice << offset);
        }
        b[j] = restored_val;
    }
    
    return b;
}

bool CMP::verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
    for(uint64_t i = 0;i < num_cmps; ++i){
        //cout<<"actural_result: "<<actural_result[i]<< " result: "<<result[i]<<endl;
        if (actural_result[i]!=(result[i]==1)){
            return false;
        }
    }
    return true;
}

Plaintext CMP::get_one_hot(uint64_t start, uint64_t width) {
    vector<uint64_t> onehot(lhe->slot_count, 0); 
    uint64_t end_idx = std::min((start + width) * num_slots_per_element, (uint64_t)onehot.size());
    std::fill(onehot.begin() + start * num_slots_per_element, onehot.begin() + end_idx, 1);
    
    return lhe->encode(onehot);
}

Plaintext CMP::get_double_one_hot(uint64_t start, uint64_t width) {
    vector<uint64_t> onehot(lhe->slot_count, 0); 
    uint64_t end_idx = std::min((start + width) * num_slots_per_element, (uint64_t)onehot.size());
    std::fill(onehot.begin() + start * num_slots_per_element, onehot.begin() + end_idx, 1);
    std::fill(onehot.begin() + start * num_slots_per_element + row_count, onehot.begin() + end_idx + row_count, 1);

    return lhe->encode(onehot);
}

vector<Ciphertext> CMP::exchange(const vector<Ciphertext>& b, uint64_t end_index, uint64_t start_index) {
    int64_t start_pos = static_cast<int64_t>(start_index * num_slots_per_element);
    int64_t end_pos = static_cast<int64_t>(end_index * num_slots_per_element);
    int64_t rc = static_cast<int64_t>(row_count);
    int step = static_cast<int>(((start_pos - end_pos) % rc + rc) % rc);
    
    vector<Ciphertext> result(b.size());
    bool start_in_row0 = start_pos < rc;
    bool end_in_row0 = end_pos < rc;

    for (size_t i = 0; i < b.size(); ++i) {
        if (start_in_row0 == end_in_row0) {
            result[i] = (step == 0) ? b[i] : lhe->rotate_rows(b[i], step);
        } else {
            auto temp = lhe->rotate_columns(b[i]);
            result[i] = (step == 0) ? temp : lhe->rotate_rows(temp, step);
        }
    }
    return result;
}

Ciphertext CMP::exchange(const Ciphertext& b, uint64_t end_index, uint64_t start_index) {
    int64_t start_pos = static_cast<int64_t>(start_index * num_slots_per_element);
    int64_t end_pos = static_cast<int64_t>(end_index * num_slots_per_element);
    int64_t rc = static_cast<int64_t>(row_count);

    int step = static_cast<int>(((start_pos - end_pos) % rc + rc) % rc);
    
    Ciphertext result;
    bool start_in_row0 = start_pos < rc;
    bool end_in_row0 = end_pos < rc;

    if (start_in_row0 == end_in_row0) {
        result = (step == 0) ? b : lhe->rotate_rows(b, step);
    } else {
        auto temp = lhe->rotate_columns(b);
        result = (step == 0) ? temp : lhe->rotate_rows(temp, step);
    }
    
    return result;
}

// start --> end
vector<Ciphertext> CMP::exchange(const vector<Ciphertext>& b, const vector<Ciphertext>& b_rot_columns, uint64_t end_index, uint64_t start_index) {
    int64_t start_pos = static_cast<int64_t>(start_index * num_slots_per_element);
    int64_t end_pos = static_cast<int64_t>(end_index * num_slots_per_element);
    int64_t rc = static_cast<int64_t>(row_count);
    
    int step = static_cast<int>(((start_pos - end_pos) % rc + rc) % rc);
    
    vector<Ciphertext> result(b.size());
    bool start_in_row0 = start_pos < rc;
    bool end_in_row0 = end_pos < rc;

    for (size_t i = 0; i < b.size(); ++i) {
        if (start_in_row0 == end_in_row0) {
            result[i] = (step == 0) ? b[i] : lhe->rotate_rows(b[i], step);
        } else {
            result[i] = (step == 0) ? b_rot_columns[i] : lhe->rotate_rows(b_rot_columns[i], step);
        }
    }
    return result;
}

vector<Ciphertext> CMP::fill_width_hot(vector<Ciphertext>& data, uint64_t start, uint64_t width, uint64_t repeat){
    if (width <= repeat) return data;
    auto onehot = get_one_hot(start, repeat);
    auto out = lhe->multiply_plain(data, onehot);

    if (start < num_cmps_per_row && (start + width) > num_cmps_per_row && start !=0){
        //cout<< " start: " << start << " width: " << width << endl;

        uint64_t rot_times1 = (num_cmps_per_row - start + repeat - 1) / repeat;
        uint64_t rot_times2 = (width + start - num_cmps_per_row + repeat - 1) / repeat;
        //cout<< " start: " << start << " repeat: " << repeat << " rot_times1: " << rot_times1<< " rot_times2: " << rot_times2;
        
        auto temp2 = exchange(out, num_cmps_per_row, start);

        for(uint64_t k = 1; k < rot_times1; k<<=1 ){
            //cout<< " rot_times1: "<< rot_times1 <<" k: "<<k;
            auto temp = exchange(out, k * repeat + start, start);
            lhe->add_inplace(out, temp);

        }

        for(uint64_t k = 1; k < rot_times2; k<<=1 ){
            //cout<< " rot_times2: "<< rot_times2 <<" k: "<<k;
            auto temp = exchange(temp2, k * repeat + num_cmps_per_row, num_cmps_per_row);
            lhe->add_inplace(temp2, temp);

        }
        lhe->add_inplace(out, temp2);
    }else{
        uint64_t rot_times = (width + repeat - 1) / repeat;
        //cout<< " start: " << start << " repeat: " << repeat << " rot_times: " << rot_times;
        for(uint64_t k = 1; k < rot_times; k <<= 1 ){
            auto temp = exchange(out, k * repeat + start, start);
            lhe->add_inplace(out, temp);
        }
    }

    return out;
}


vector<Ciphertext> CMP::fill_double_width_hot(vector<Ciphertext>& data, uint64_t start, uint64_t width, uint64_t repeat){
    if (width <= repeat) return data;
    auto onehot = get_double_one_hot(start, repeat);
    auto out = lhe->multiply_plain(data, onehot);

    uint64_t rot_times = (width + repeat - 1) / repeat;
    //cout<< " start: " << start << " repeat: " << repeat << " rot_times: " << rot_times;
    for(uint64_t k = 1; k < rot_times; k <<= 1 ){
        auto temp = exchange(out, k * repeat + start, start);
        lhe->add_inplace(out, temp);
    }
    return out;
}

long CMP::keys_size(){
    return lhe->rlk_size() + lhe->pk_size() + lhe->glk_size();
}

long CMP::comm_cost(const vector<Ciphertext>& ct1, const Ciphertext& ct2) {
    std::stringstream data_stream;
    long comm = 0;
    for(const auto& e : ct1) comm += e.save(data_stream);
    comm += ct2.save(data_stream);
    return comm;
}

long CMP::comm_cost(const Ciphertext& ct1, const Ciphertext& ct2){
    std::stringstream data_stream;
    long comm = 0;
    comm+=ct1.save(data_stream);
    comm+=ct2.save(data_stream);
    return comm;
}

long CMP::comm_cost_estimate(const vector<Ciphertext>& ct1, const Ciphertext& ct2) {
    long comm = 0;
    for(const auto& e : ct1) comm += e.save_size();
    comm += ct2.save_size();
    return comm;
}

long CMP::comm_cost_estimate(const Ciphertext& ct1, const Ciphertext& ct2){
    long comm = 0;
    comm+=ct1.save_size();
    comm+=ct2.save_size();
    return comm;
}

void CMP::print() {
    lhe->print();
    cout << " name                                     : " << scheme 
        << " \n depth                                    : "<< depth    
        << " \n l                                        : "<< l 
        << " \n m                                        : "<< m
        << " \n bit precision (n)                        : "<< n 
        << " \n max batch nums                           : "<< num_cmps
        << endl;
}