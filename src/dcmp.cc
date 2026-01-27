#include"dcmp.h"

DCMP::DCMP(int l, int m, int extra, bool is_rotate) {
    this->scheme = "dcmp";
    l = 1 << static_cast<int>(std::ceil(std::log2(l)));
    m = 1 << static_cast<int>(std::ceil(std::log2(m)));
    this->l = l;
    this->m = m;
    this->n = l * m;

    int cmp_depth_need =  static_cast<int>(std::ceil(std::log2(this->n)) + 1); 
    depth = cmp_depth_need + extra;

    std::vector<int> steps;
    for (int i = 1; i < m; i<<=1) { // 1, 2, 3, ..., 2^m
        steps.push_back(i);
    }

    this->lhe = make_unique<BFV>(depth, steps, is_rotate);

    slot_count = lhe->slot_count;
    row_count = slot_count / 2;
    num_slots_per_element = m;
    num_cmps_per_row = row_count / num_slots_per_element;

    if (l!=1){
        num_cmps = num_cmps_per_row * 2 - 1;
    }else{
        num_cmps = num_cmps_per_row * 2;
    }

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
// b = [ b00, b01, b02, ..., b10, b11, b12, ...
//       b03, b04, b05, ..., b13, b14, b15, ... ]
// output
// neg_b = [ 1 - b000, 1 - b001, 1 - b002, ..., 1 - b100, 1 - b101, 1 - b102, ...
//           1 - b030, 1 - b031, 1 - b032, ..., 1 - b130, 1 - b131, 1 - b132, ... ]
vector<vector<uint64_t>> DCMP::encode_b(const vector<vector<uint64_t>>& raw_b) {
    vector<vector<uint64_t>> out(l);
    vector<uint64_t> temp(slot_count, 0);
    for(int i = 0; i < l;i++){
        std::fill(temp.begin(), temp.end(), 0ULL);
         for(uint64_t j = 0; j < num_cmps; j++){
            auto offset = j * m;
            //uint64_t start_idx = index_map[j];
            for(int k = 0; k < m; k++) {
                temp[offset + k] = 1 - ((raw_b[i][j]>>k)&1);
            }
         }

        if(l!=1){
            temp[slot_count - 1] = 13; // padding.
        }
        out[i] = temp;
    }
    return out;
}

vector<Ciphertext> DCMP::encrypt(const vector<vector<uint64_t>>& raw_b) {
    vector<Ciphertext> out(l);
    for(int i = 0 ; i < l; i++){
        out[i] = lhe->encrypt(raw_b[i]);
    }

    return out;
}

vector<vector<uint64_t>> DCMP::decode_b(const vector<Ciphertext>& cts) {
    vector<vector<uint64_t>> decrypted_data(l);

    for (int i = 0; i < l; i++) {
        decrypted_data[i] = decrypt(cts[i]); 
    }
    vector<vector<uint64_t>> raw_b(l, vector<uint64_t>(num_cmps, 0));

    for (int i = 0; i < l; i++) {
        for (uint64_t j = 0; j < num_cmps; j++) {
            auto offset = j * m;
            uint64_t val = 0;

            for (int k = 0; k < m; k++) {
                uint64_t slot_val = decrypted_data[i][offset + k];
                uint64_t bit = 1 - (slot_val & 1); 
                val |= (bit << k);
            }
            raw_b[i][j] = val;
        }
    }

    return raw_b;
}

// input
// a = [ a00, a01, a02 ]
// output
// a = [ a00, a01, a02 ]
vector<Plaintext> DCMP::encode_a(const vector<vector<uint64_t>>& raw_a){
    vector<Plaintext> out(l);
    vector<uint64_t> temp(slot_count, 0);
    for(int i = 0; i < l;i++){
        std::fill(temp.begin(), temp.end(), 0ULL);
         for(uint64_t j = 0; j < num_cmps; j++){
            auto offset = j * m;
            //uint64_t start_idx = index_map[j];
            for(int k = 0; k < m; k++) {
                temp[offset + k] = (raw_a[i][j]>>k)&1;
            }
         }

        if(l!=1){
            temp[slot_count - 1] =13; // padding.
        }

        out[i] = lhe->encode(temp);
    }
    return out;
}

// [ 1,0,0,...,1,0,0,...
//   1,0,0,...,1,0,0,... ]
Plaintext DCMP::init_one_zero_zero(){
    vector<uint64_t> one_zero_zero(slot_count, 0ULL);
    for(size_t i = 0; i < num_cmps ; i++){
        one_zero_zero[index_map[i]] = 1ULL;
    }
    return lhe->encode(one_zero_zero);
}

Plaintext DCMP::init_x_zero_zero(const vector<uint64_t>& x) {
    vector<uint64_t> x_zero_zero(slot_count, 0ULL);
    auto x_size = x.size();
    auto limit = num_cmps > x_size ? x_size : num_cmps;
    for(size_t i = 0; i < limit ; i++){
        x_zero_zero[index_map[i]] = x[i];
    }
    return lhe->encode(x_zero_zero);
}

// a>E(b);
Ciphertext DCMP::great_than(vector<Plaintext>& a, vector<Ciphertext>& b) {
    vector<Ciphertext> eq(l);
    vector<Ciphertext> gt(l);

    // in cols
    for(int i = 0; i < l; ++i){
        gt[i] = lhe->multiply_plain(b[i], a[i]);
        eq[i] = lhe->add(gt[i], gt[i]);
        lhe->negate_inplace(eq[i]);
        lhe->add_inplace(eq[i], b[i]);
        lhe->add_plain_inplace(eq[i], a[i]);
        
        int depth = static_cast<int>(std::log2(m));
        for(int j = 0; j < depth ; j++){
            int step = 1<<j;
            auto gt_temp = lhe->rotate_rows(gt[i], step);
            auto eq_temp = lhe->rotate_rows(eq[i], step);
            lhe->multiply_inplace(eq[i], eq_temp);
            lhe->relinearize_inplace(eq[i]);
            lhe->multiply_inplace(gt[i], eq_temp);
            lhe->relinearize_inplace(gt[i]);
            lhe->add_inplace(gt[i], gt_temp);
        }

    }

    // in rows
    int depth = static_cast<int>(std::log2(l));
    for(int i = 0; i < depth ; i++){
        int temp1 = 1<<i;
        int temp0 = 1<<(i + 1);
        //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
        for(int j = 0; j < l; j = j + temp0){                    
            lhe->multiply_inplace(gt[j], eq[j + temp1]);
            lhe->relinearize_inplace(gt[j]);
            lhe->add_inplace(gt[j], gt[j + temp1]);
            lhe->multiply_inplace(eq[j],eq[j + temp1]);
            lhe->relinearize_inplace(eq[j]);
        }   
    }
    return std::move(gt[0]);
}

void DCMP::clear_up(Ciphertext& result) {
    lhe->multiply_plain_inplace(result, one_zero_zero);
}

vector<uint64_t> DCMP::decrypt(const Ciphertext& ct) {
    auto pt = lhe->decrypt(ct);
    return lhe->decode(pt);
}

vector<uint64_t> DCMP::decode(const std::vector<uint64_t>& res){
    vector<uint64_t> ans(num_cmps);
    for(uint64_t i = 0; i < num_cmps ; i++){
        ans[i] = res[index_map[i]];
    }
    return ans;
}

vector<uint64_t> DCMP::recover(const Ciphertext& ct) {
    auto res = this->decrypt(ct);
    return this->decode(res);
}

// out = [ a[0]>b[0], a[0]>b[1], ... ]
vector<bool> DCMP::verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) {
    vector<bool> out(num_cmps, false);
    for(uint64_t i = 0;i < num_cmps;i++){
        for(int k = l-1;k>=0;k--){
            if(a[k][i] > b[k][i]){
                out[i] = true;
                break;
            }else if(a[k][i] == b[k][i]){
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
// b0 = b00 + 2 * b01 + 2^m * b02 + ...;
// b1 = b10 + 2 * b11 + 2^m * b12 + ...;
// b2 = b20 + 2 * b21 + 2^m * b22 + ...;
vector<vector<uint64_t>> DCMP::raw_encode_b(const vector<uint64_t>& b) {
    vector<vector<uint64_t>> out(l, vector<uint64_t>(slot_count, 0));
    const uint64_t range = (1 << num_slots_per_element) - 1;
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

vector<uint64_t> DCMP::raw_decode_b(const vector<vector<uint64_t>>& encoded_out, size_t original_b_size) {
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

vector<vector<uint64_t>> DCMP::raw_encode_a(const vector<uint64_t>& in) {
    return raw_encode_b(in);
}


// low to high  
vector<vector<uint64_t>> DCMP::random_raw_encode_b() {
    vector<vector<uint64_t>> out(l, vector<uint64_t>(num_cmps));
    std::uniform_int_distribution<uint64_t> dist(0, num_slots_per_element - 1);

    for (int i = 0; i < l; i++) {
        uint64_t* row_ptr = out[i].data(); 
        for (uint64_t j = 0; j < num_cmps; j++) {
            row_ptr[j] = dist(gen);
        }
    }
    return out; 
}

// low to high
vector<vector<uint64_t>> DCMP::random_raw_encode_a() {
    return random_raw_encode_b();
}


Plaintext DCMP::get_one_hot(uint64_t start, uint64_t width) {
    vector<uint64_t> onehot(lhe->slot_count, 0); 

    uint64_t end_idx = std::min((start + width) * m, (uint64_t)onehot.size());
    std::fill(onehot.begin() + start * m, onehot.begin() + end_idx, 1);
    
    return lhe->encode(onehot);
}

vector<Ciphertext> DCMP::rotate_m_rows(const vector<Ciphertext>& b, const vector<Ciphertext>& b_inv_rows, int step) {

    int64_t absolute_step = (static_cast<int64_t>(step) * m) % slot_count;

    if (absolute_step < 0) {
        absolute_step += slot_count;
    }

    if (absolute_step < row_count) {
        if (absolute_step == 0) return b;
        return lhe->rotate_rows(b, static_cast<int>(absolute_step));
    } 
    else {
        int64_t physical_shift = absolute_step - row_count;
        if (physical_shift == 0) return b_inv_rows;
        return lhe->rotate_rows(b_inv_rows, static_cast<int>(physical_shift));
    }
}

vector<Ciphertext> DCMP::rotate_m_rows(const vector<Ciphertext>& b, int step) {

    int64_t absolute_step = (static_cast<int64_t>(step) * m) % slot_count;
    if (absolute_step < 0) {
        absolute_step += slot_count;
    }

    if (absolute_step < row_count) {
        if (absolute_step == 0) return b;
        return lhe->rotate_rows(b, static_cast<int>(absolute_step));
    } 
    else {
        int64_t physical_shift = absolute_step - row_count;
        auto b_inv_rows = lhe->rotate_columns(b);
        if (physical_shift == 0) return b_inv_rows;
        return lhe->rotate_rows(b_inv_rows, static_cast<int>(physical_shift));
    }
}

Ciphertext DCMP::rotate_m_rows(const Ciphertext& b, const Ciphertext& b_inv_rows, int step) {

    int64_t absolute_step = (static_cast<int64_t>(step) * m) % slot_count;

    if (absolute_step < 0) {
        absolute_step += slot_count;
    }

    if (absolute_step < row_count) {
        if (absolute_step == 0) return b;
        return lhe->rotate_rows(b, static_cast<int>(absolute_step));
    } 
    else {
        int64_t physical_shift = absolute_step - row_count;
        if (physical_shift == 0) return b_inv_rows;
        return lhe->rotate_rows(b_inv_rows, static_cast<int>(physical_shift));
    }
}

Ciphertext DCMP::rotate_m_rows(const Ciphertext& b, int step) {

    int64_t absolute_step = (static_cast<int64_t>(step) * m) % slot_count;

    if (absolute_step < 0) {
        absolute_step += slot_count;
    }

    if (absolute_step < row_count) {
        if (absolute_step == 0) return b;
        return lhe->rotate_rows(b, static_cast<int>(absolute_step));
    } 
    else {
        int64_t physical_shift = absolute_step - row_count;
        auto b_inv_rows = lhe->rotate_columns(b);
        if (physical_shift == 0) return b_inv_rows;
        return lhe->rotate_rows(b_inv_rows, static_cast<int>(physical_shift));
    }
}

void DCMP::print() {
    lhe->print();
    cout << " name                                     : " << scheme 
        << " \n depth                                    : "<< depth    
        << " \n l                                        : "<< l 
        << " \n m                                        : "<< m
        << " \n bit precision (n)                        : "<< n 
        << " \n max batch size                           : "<< num_cmps
        << endl;
}