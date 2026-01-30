#include"tcmp.h"

TCMP::TCMP(int l, int m, int extra, bool is_rotate, uint8_t id) {
    this->scheme = "tcmp";
    this->id = id;
    l = 1 << static_cast<int>(std::ceil(std::log2(l)));
    this->l = l;
    this->m = m;
    this->n = l * m;

    int cmp_depth_need = (this->id == 0x1) ? static_cast<int>(std::ceil(std::log2(l))) : l;
    depth = cmp_depth_need + extra;

    if (m <= 4){
        std::vector<int> steps;
        for (int i = 1; i <= pow(2,m); i++) { // 1, 2, 3, ..., 2^m
            steps.push_back(i);
        }
        this->lhe = make_unique<BFV>(depth, steps, is_rotate);        
    }else{
        this->lhe = make_unique<BFV>(depth, vector<int>(0), true);
    }

    slot_count = lhe->slot_count;
    row_count = lhe->row_count;

    if (m > int(this->lhe->log_poly_mod_degree)) {
        cout<< "m: " << m <<" log_poly_mod_degree: " << this->lhe->log_poly_mod_degree<<endl;
        cout << "Error: params m is too large." << endl;
        exit(0);
    }else 

    num_slots_per_element = 1 << m;
    num_cmps = slot_count / num_slots_per_element;
    num_cmps_per_row = num_cmps / 2;

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
    one_zero_zero_cipher = lhe->encrypt(one_zero_zero);
}

// input
// num_cmps 
// b = [ b00, b10, b20
//       b01, b11, b21
//       b02, b12, b22 ]
// b0 = b00 + 2^m * b01 + (2^m)^2 * b02 ;
// b1 = b10 + 2^m * b11 + (2^m)^2 * b12 ;
// b2 = b20 + 2^m * b21 + (2^m)^2 * b22 ;
// output
// b = [ te(b00), te(b10), te(b20)
//       te(b01), te(b11), te(b21)
//       te(b02), te(b12), te(b22) ]
vector<vector<uint64_t>> TCMP::encode_b(const vector<vector<uint64_t>>& b) {
    size_t rows = b.size();
    vector<vector<uint64_t>> out(rows, vector<uint64_t>(slot_count, 1ULL));

    for(size_t i = 0; i < rows; i++) {
        uint64_t* row_ptr = out[i].data();
        const auto& b_row = b[i];
        for(size_t j = 0; j < b_row.size(); j++) {
            uint64_t start_idx = j * num_slots_per_element;
            // te encoding
            uint64_t theta = b_row[j];
            std::fill_n(row_ptr + start_idx, theta + 1, 0ULL); 

            //if (theta + 1 == slot_count){
            //    std::cerr << "[Warning] Reached slot_count limit. theta: " << theta 
            //            << ", slot_count: " << slot_count << std::endl;
            //    throw std::logic_error("Comparison range exceeds slot_count boundary.");
            //}
        }
    }
    return out;
}

vector<vector<uint64_t>> TCMP::decode_b(const vector<Ciphertext>& cts) {
    vector<vector<uint64_t>> decrypted_data(l);
    for (int i = 0; i < l; i++) {
        decrypted_data[i] = decrypt(cts[i]); 
    }
    vector<vector<uint64_t>> raw_b(l, vector<uint64_t>(num_cmps, 0));
    uint64_t max_search = (1ULL << m);

    for (int i = 0; i < l; i++) {
        const uint64_t* row_ptr = decrypted_data[i].data();
        for (uint64_t j = 0; j < num_cmps; j++) {
            uint64_t start_idx = j * num_slots_per_element;
            uint64_t theta = 0;

            while (theta < max_search && start_idx + theta < slot_count && row_ptr[start_idx + theta] == 0ULL) {
                theta++;
            }
            raw_b[i][j] = theta - 1; 
        }
    }

    return raw_b;
}

// input
// a = [ a00, a01, a02 ]
// output
// a = [ a00, a01, a02 ]
vector<Plaintext> TCMP::encode_a(const vector<vector<uint64_t>>& raw_a) {
    vector<uint64_t> a(l);
    for(int i = 0; i < l; ++i){
        a[i] = raw_a[i][0];
    }
    auto out = lhe->encode(a);
    return vector<Plaintext>{out};
}


Ciphertext TCMP::great_than(vector<Plaintext>& pt_a, vector<Ciphertext>& b)  {
    auto a = lhe->decode(pt_a[0]);

    vector<Ciphertext> eq(l);
    vector<Ciphertext> gt(l);

    if(l == 1){
        gt[0] = lhe->rotate_rows_global(b[0], a[0]);
        return gt[0];
    }else{
        for(int i = 0; i < l; ++i){
            gt[i] = lhe->rotate_rows_global(b[i], a[i]);
            if(a[i] < num_slots_per_element - 1){
                eq[i] = lhe->rotate_rows_global(b[i], a[i] + 1);
            }else{
                eq[i] = one_zero_zero_cipher;
            }
            lhe->sub_inplace(eq[i], gt[i]); //eq = eq - gt ;
        }
    }

    // result = b > a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * gt_0))
    // result = b >= a  = gt_n-1 + eq_n-1 * (... gt_2 + eq_2 * (gt_1 + eq_1 * (gt_0 + eq_0)))

    if (id==0x1){
        int depth = log(l)/log(2);
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
    }else{
        for(int i = 1;i < l; i++){
            lhe->multiply_inplace(gt[0], eq[i]);
            lhe->relinearize_inplace(gt[0]);
            lhe->add_inplace(gt[0], gt[i]);
        }

    }

    return gt[0];
}

// out = [ a[0]>b[0], a[0]>b[1], ... ]
vector<bool> TCMP::verify(const vector<vector<uint64_t>>& raw_a, const vector<vector<uint64_t>>& b)  {
    vector<uint64_t> a(l);
    for(int i = 0; i < l; ++i){
        a[i] = raw_a[i][0];
    }
    vector<bool> out(num_cmps, false);
    for(uint64_t i = 0; i < num_cmps ; ++i){
        for(int k = l-1; k >= 0; --k){
            if(a[k] > b[k][i]){
                out[i] = true;
                break;
            }else if(a[k] == b[k][i]){
                continue;
            }else{
                break;
            }
        }
    }
    return out;
}