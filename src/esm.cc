#include"esm.h"

ESM::ESM(){
    scheme = "bpdte_esm";
}

// server
LeafFlatten ESM::encode_tree(shared_ptr<Node> root){
    auto leaf = raw_encode_tree(root);
    this->salts = init_salts(2, leaf.leaf_vec.size());
    
    vector<uint64_t> one(cmp->slot_count, 1ULL);
    one_one_one = lhe->encode(one);

    vector<uint64_t> zero(cmp->slot_count, 0ULL);
    zero_zero_zero = lhe->encrypt(zero);

    return leaf;
}

// client
void ESM::setup_cmp(int cmp_type, int l, int m, int extra){
    switch (cmp_type) {
        case 0: cmp = make_unique<TCMP>(l, m, extra); break;
        case 1: cmp = make_unique<DCMP>(l, m, extra); break;
    }

    batch_size = cmp->num_cmps;
    lhe = cmp->lhe;

}

vector<vector<Plaintext>> ESM::init_salts(int row, int cols){
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(1, lhe->plain_modulus - 1);
    
    vector<vector<Plaintext>> salts(row, vector<Plaintext>(cols));
    vector<uint64_t> salt_buffer(batch_size);
    for(int i = 0; i <  row ; ++i){
        for(int j = 0; j < cols; ++j){
            for (size_t k = 0; k < batch_size; ++k) {
                salt_buffer[k] =  distrib(gen);
            }
            salts[i][j] = cmp->init_x_zero_zero(salt_buffer);
        }
    }

    return salts;
}

// server
vector<vector<Ciphertext>> ESM::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten){
    auto sum_path_result = sum_path(root, data_cipher);
    return extended_sum_path(sum_path_result, leaf_flatten);
}

vector<vector<Ciphertext>> ESM::extended_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten){
    // record the zero position
    vector<vector<Ciphertext>> out(2, vector<Ciphertext>(0));
    out[0] = sum_path_result;
    out[1] = sum_path_result;
    auto leaf_vec_pt = leaf_flatten.leaf_vec_pt;
    auto leaf_num = leaf_vec_pt.size();
    for(size_t i = 0; i < leaf_num; i++){
        lhe->multiply_plain_inplace(out[0][i], salts[0][i]);
        lhe->multiply_plain_inplace(out[1][i], salts[1][i]);
        lhe->add_plain_inplace(out[1][i], leaf_vec_pt[i]);
    }
    return shuffle(out, leaf_num);
}

vector<vector<Ciphertext>> ESM::shuffle(vector<vector<Ciphertext>> out, int leaf_num){
    auto leaf_num_perm = random_permutation(leaf_num);
    vector<Ciphertext> x(leaf_num);
    vector<Ciphertext> y(leaf_num);
    
    // shuffle the leaf position
    for(int i = 0; i < leaf_num; i++){
        x[i] = std::move(out[0][leaf_num_perm[i]]);
        y[i] = std::move(out[1][leaf_num_perm[i]]);
    }

    // shuffle the leaf in different row.
    int log_data_rows = ceil(log2(data_rows));
    int half_data_rows = (data_rows + 1) / 2;
    vector<uint64_t> W0(lhe->slot_count);
    vector<uint64_t> W1(lhe->slot_count); 

    vector<Ciphertext> x_temp0(leaf_num);
    vector<Ciphertext> y_temp0(leaf_num);
    vector<Ciphertext> x_temp1(leaf_num);
    vector<Ciphertext> y_temp1(leaf_num);

    cout<<"Shuffling, log_data_rows " <<log_data_rows<<endl;
    for(int i = 0; i < log_data_rows ; ++i){
        auto data_rows_perm = random_permutation(data_rows); //+ 1

        std::fill(W0.begin(), W0.end(), 0);
        std::fill(W1.begin(), W1.end(), 0);

        for(int j = 0; j < half_data_rows; ++j){
            W0[data_rows_perm[j]] = 1;
            if (half_data_rows + j < data_rows) {
                W1[data_rows_perm[half_data_rows + j]] = 1;
            }
        }

        Plaintext W0_pt = cmp->init_x_zero_zero(W0);
        Plaintext W1_pt = cmp->init_x_zero_zero(W1);

        auto L = random_permutation(leaf_num);

        for(int j = 0; j < leaf_num; ++j){
            
            x_temp0[j] = x[j];
            y_temp0[j] = y[j];
            x_temp1[j] = std::move(x[j]);
            y_temp1[j] = std::move(y[j]);

            lhe->multiply_plain_inplace(x_temp0[j], W0_pt);
            lhe->multiply_plain_inplace(x_temp1[j], W1_pt);
            lhe->multiply_plain_inplace(y_temp0[j], W0_pt);
            lhe->multiply_plain_inplace(y_temp1[j], W1_pt);
        }

        for(int j = 0; j < leaf_num; ++j){
            lhe->add_inplace(x_temp1[j], x_temp0[L[j]]);
            lhe->add_inplace(y_temp1[j], y_temp0[L[j]]);
            x[j] = std::move(x_temp1[j]);
            y[j] = std::move(y_temp1[j]);
        }
    }
    return {std::move(x), std::move(y)};
}

vector<uint64_t> ESM::recover(vector<vector<Ciphertext>>& a){
    size_t leaf_num = a[0].size();
    vector<vector<uint64_t>> ans0(leaf_num);
    vector<vector<uint64_t>> ans1(leaf_num);
    for(size_t j = 0; j < a[0].size(); ++j){
        ans0[j] = cmp->recover(a[0][j]);
        ans1[j] = cmp->recover(a[1][j]);
    }

    vector<uint64_t> out;
    out.reserve(data_rows);

    for(int j = 0; j < data_rows ; j++){
        for(size_t i = 0; i < ans0.size(); i++){
            if(ans0[i][j]==0){
                out.push_back(ans1[i][j]);
                break;      
            }
        }
    }

    if(out.size()<static_cast<size_t>(data_rows)){
        cout<<"depth_need_min is too small, please the params again by add the extra value."<<endl;
        //exit(0);
    }
    return out;
}
