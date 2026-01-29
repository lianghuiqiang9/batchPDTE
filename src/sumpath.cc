#include"sumpath.h"

SumPath::SumPath(){
    scheme = "pdte_sum_path";
}

// client
void SumPath::setup_cmp(int cmp_type, int l, int m, int extra){
    if (l!=1){
        cout<<"please choose the l to 1, get the best performance."<<endl;
    }
    
    //
    //int log_tree_depth = static_cast<int>(std::ceil(std::log2(tree_depth) + 1));
    //extra = log_tree_depth + extra;

    switch (cmp_type) {
        case 1: cmp = make_unique<DCMP>(l, m, extra, true); break;
    }

    lhe = cmp->lhe;

    // 
    vector<uint64_t> zero(cmp->num_cmps,0); 
    auto cmp_raw_encode_zero_b = cmp->raw_encode_b(zero);
    auto cmp_encode_zero_b = cmp->encode_b(cmp_raw_encode_zero_b);
    cmp_zero_b = cmp->encrypt(cmp_encode_zero_b);

    one_zero_zero = cmp->init_one_zero_zero();

    vector<uint64_t> neg_one(cmp->num_cmps, cmp->lhe->plain_modulus - 1);
    neg_one_zero_zero = cmp->init_x_zero_zero(neg_one);

    vector<uint64_t> neg_two(cmp->num_cmps, cmp->lhe->plain_modulus - 2);
    neg_two_zero_zero = cmp->init_x_zero_zero(neg_two);

    // salt
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(1, lhe->plain_modulus - 1);
    vector<uint64_t> salt1(cmp->num_cmps);
    vector<uint64_t> salt2(cmp->num_cmps);
    for (uint64_t i = 0; i < cmp->num_cmps; ++i) {
        salt1[i] = distrib(gen);
        salt2[i] = distrib(gen);
    }
    salt1_pt = cmp->init_x_zero_zero(salt1);
    salt2_pt = cmp->init_x_zero_zero(salt2);

}

vector<vector<Ciphertext>> SumPath::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    
    auto extract_data = feature_extract(data_cipher, tree_flatten);
    const uint64_t new_rows = tree_flatten.index_flatten.size();
    
    vector<Ciphertext> cmp_raw_out(new_rows);

    for (size_t i = 0; i < new_rows; ++i){
        // select the left node when t > x[a].
        cmp_raw_out[i] = cmp->great_than(tree_flatten.threshold_pt[i], extract_data[i]);

        auto right_temp =lhe->multiply_plain(cmp_raw_out[i], neg_two_zero_zero);
        lhe->add_plain_inplace(right_temp, one_zero_zero);
        lhe->multiply_plain_inplace(right_temp, tree_flatten.direction_pt[i]);
        
        lhe->add_plain_inplace(cmp_raw_out[i], neg_one_zero_zero);
        lhe->add_inplace(cmp_raw_out[i], right_temp);
    }

    auto cmp_out = expend_compare_result(cmp_raw_out, tree_flatten);  
    //cout<< "cmp_out.size(): " << cmp_out.size() <<endl;

    auto out1 = lhe->add_many(cmp_out); // zero position
    auto out2 = lhe->multiply_plain(out1, salt2_pt); // leaf position
    lhe->add_plain_inplace(out2, tree_flatten.leaf_values_pt);

    lhe->multiply_plain_inplace(out1, salt1_pt);

    return vector<vector<Ciphertext>>{vector<Ciphertext>{out1, out2}}; // be careful the sequence.

}

vector<uint64_t> SumPath::recover(vector<vector<Ciphertext>>& a){
    auto temp1 = cmp->recover(a[0][0]);
    auto temp2 = cmp->recover(a[0][1]);
    for (int i = 0; i < leaf_nums; i++){
        if (temp1[i]== 0){
            return vector<uint64_t>{temp2[i]};
        }
    }
    return vector<uint64_t>{0};
}