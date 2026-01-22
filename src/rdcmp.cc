#include"rdcmp.h"

Rdcmp::Rdcmp(int l, int m, int n, int tree_depth, int extra) {
    this->scheme = "rdcmp";
    this->n = n;

    int cmp_depth_need = static_cast<int>(std::floor(std::log2(n)) + 1); // plus one for additive operation
    int tree_depth_need = (tree_depth == 0) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)));
    depth = cmp_depth_need + tree_depth_need + extra;

    this->lhe = make_unique<BFV>(depth);

    slot_count = lhe->slot_count;
    row_count = slot_count / 2;
    
    num_cmps = slot_count - 1; //Reserve a padding bit to prevent the plaintext from being 0

    one_one_zero = init_one_one_zero();
}

// input
// b = [ b00, b10, ...
//       b01, b11, ...
//       b02, b12, ...]
// output
// b = [ 1 - b00, 1 - b10, ...
//       1 - b01, 1 - b11, ...
//       1 - b02, 1 - b12, ...]
vector<vector<uint64_t>> Rdcmp::encode_b(const vector<vector<uint64_t>>& b) {
    vector<vector<uint64_t>> out(n);
    for(int i = 0; i < n;i++){
        vector<uint64_t> temp(num_cmps, 0);
        for(uint64_t j = 0; j < num_cmps; j++){

            // negitive
            temp[j] = 1 - b[i][j]; 
        }
        out[i] = temp;
    }
    return out;
}

vector<Ciphertext> Rdcmp::encrypt(const vector<vector<uint64_t>>& b) {
    vector<Ciphertext> out(n);
    for(int i = 0 ; i < n; i++){
        out[i] = lhe->encrypt(b[i]);
    }

    return out;
}

// input
// a = [ a00, a01, a02 ]
// output
// a = [ a00, a01, a02 ]
vector<Plaintext> Rdcmp::encode_a(const vector<vector<uint64_t>>& a) {
    vector<Plaintext> out(n);
    for(int i = 0; i < n; ++i) {
        out[i] = lhe->encode(a[i]);
    }
    return out;
}


// [ 1,0,0,...,1,0,0,...
//   1,0,0,...,1,0,0,... ]
Plaintext Rdcmp::init_one_one_zero(){
    vector<uint64_t> one_one_zero(slot_count,1);
    one_one_zero[num_cmps] = 0;
    return lhe->encode(one_one_zero);
}

Plaintext Rdcmp::init_x_zero_zero(const vector<uint64_t>& salt) {
    return lhe->encode(salt);
}

Ciphertext Rdcmp::great_than(vector<Plaintext>& a, vector<Ciphertext>& b) {
    vector<Ciphertext> eq(n);
    vector<Ciphertext> gt(n);

    for(int i = 0; i < n; ++i){
        gt[i] = lhe->multiply_plain(b[i], a[i]);
        eq[i] = lhe->add(gt[i], gt[i]);
        lhe->negate_inplace(eq[i]);
        lhe->add_inplace(eq[i], b[i]);
        lhe->add_plain_inplace(eq[i], a[i]);
    }

    int depth = static_cast<int>(log2(n));
    for(int i = 0; i < depth ; ++i){
        int temp1 = 1<<i;
        int temp0 = 1<<(i + 1);
        //cout<<"temp0 : "<<temp0<<" temp1 : "<<temp1<<endl;
        for(int j = 0; j < n; j = j + temp0){
            lhe->multiply_inplace(gt[j], eq[j + temp1]);
            lhe->relinearize_inplace(gt[j]);
            lhe->add_inplace(gt[j], gt[j + temp1]);
            lhe->multiply_inplace(eq[j], eq[j + temp1]);
            lhe->relinearize_inplace(eq[j]);

        }   
    }
    return std::move(gt[0]);

}

void Rdcmp::clear_up(Ciphertext& result) {
    lhe->multiply_plain_inplace(result, one_one_zero);
}

vector<uint64_t> Rdcmp::decrypt(const Ciphertext& ct) {
    auto pt = lhe->decrypt(ct);
    return lhe->decode(pt);
}

vector<uint64_t> Rdcmp::decode(const std::vector<uint64_t>& res) {
    return res;
}

vector<uint64_t> Rdcmp::recover(const Ciphertext& ct) {
    auto res = this->decrypt(ct);
    return this->decode(res);
}

// raw_a, raw_b. 
vector<bool> Rdcmp::verify(const vector<vector<uint64_t>>& a, const vector<vector<uint64_t>>& b) {
    vector<bool> out(num_cmps, false);
    for(uint64_t i = 0;i < num_cmps;i++){
        for(int k = n-1;k>=0;k--){
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

// input= [ b0,  b1,  b2, ... ]
// out  = [ b00, b10, b20, ...
//          b01, b11, b21, ...
//          b02, b12, b22, ... ]
// b0 = b00 + 2 * b01 + 2^2 * b02 + ...;
// b1 = b10 + 2 * b11 + 2^2 * b12 + ...;
// b2 = b20 + 2 * b21 + 2^2 * b22 + ...;
vector<vector<uint64_t>> Rdcmp::raw_encode_b(const vector<uint64_t>& b) {
    vector<vector<uint64_t>> out(n, vector<uint64_t>(slot_count, 0ULL));
    
    for(int i = 0; i < n; i++) {
        uint64_t* out_ptr = out[i].data();
        for(size_t j = 0; j < b.size(); j++) {
            out_ptr[j] = (b[j] >> i) & 1; 
        }
        out_ptr[num_cmps] = 13; // Padding bit
    }
    return out;
}
vector<vector<uint64_t>> Rdcmp::raw_encode_a(const vector<uint64_t>& in) {
    return raw_encode_b(in);
}

// low to high  
vector<vector<uint64_t>> Rdcmp::random_raw_encode_b() {
    vector<vector<uint64_t>> out(n, vector<uint64_t>(num_cmps));
    std::uniform_int_distribution<uint64_t> dist(0, 1);

    for (int i = 0; i < n; i++) {
        uint64_t* row_ptr = out[i].data();
        for (uint64_t j = 0; j < num_cmps; j++) {
            row_ptr[j] = dist(gen);
        }
    }
    return out;
}

// low to high
vector<vector<uint64_t>> Rdcmp::random_raw_encode_a() {
    return random_raw_encode_b();
}

void Rdcmp::print() {
    cout << " name                                     : " << scheme
         << " \n lhe name                                 : "<< lhe->scheme          
         << " \n max depth                                : "<< depth
            << " \n bit precision (n)                        : "<< n 
            << " \n compare number                           : "<< num_cmps
            << " \n";
}
