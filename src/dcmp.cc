#include"dcmp.h"

DCMP::DCMP(int l, int m, int extra, bool is_rotate, bool is_padding) {
    this->scheme = "dcmp";
    l = 1 << static_cast<int>(std::ceil(std::log2(l)));
    m = 1 << static_cast<int>(std::ceil(std::log2(m)));
    this->l = l;
    this->m = m;
    this->n = l * m;
    this->is_padding = is_padding;

    int cmp_depth_need =  static_cast<int>(std::ceil(std::log2(this->n)) + 1); 
    depth = cmp_depth_need + extra;

    std::vector<int> steps;
    for (int i = 1; i < m; i<<=1) { // 1, 2, 3, ..., 2^m
        steps.push_back(i);
    }

    this->lhe = make_unique<BFV>(depth, steps, is_rotate);

    slot_count = lhe->slot_count;
    row_count = lhe->row_count;

    num_slots_per_element = m;

    if (l!=1 && is_padding){
        num_cmps = slot_count / num_slots_per_element - 1;
    }else{
        num_cmps = slot_count / num_slots_per_element;
    }

    num_cmps_per_row = (num_cmps + 1) / 2;


    // index_map
    //idx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
    //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
    //index_map.resize(num_cmps);
    //for(uint64_t i = 0; i < num_cmps; i++) {
        //bool flag = i < num_cmps_per_row;
        //index_map[i] = flag ? (i * num_slots_per_element) : (row_count + (i - num_cmps_per_row) * num_slots_per_element);
    //    index_map[i] = i * num_slots_per_element;
    //}

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

        if(l!=1 && is_padding){
            temp[slot_count - 1] = 13; // padding.
        }
        out[i] = temp;
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
