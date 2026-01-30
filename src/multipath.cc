#include"multipath.h"

MultiPath::MultiPath(){
    scheme = "pdte_multi_path";
}

TreeFlatten MultiPath::encode_tree(shared_ptr<Node> root){
    vector<uint64_t> zero(cmp->num_cmps,0); //change
    auto cmp_raw_encode_zero_b = cmp->raw_encode_b(zero);
    auto cmp_encode_zero_b = cmp->encode_b(cmp_raw_encode_zero_b);
    cmp_zero_b = cmp->encrypt(cmp_encode_zero_b);

    return raw_encode_tree(root);
}

vector<vector<Ciphertext>> MultiPath::encode_data(const vector<vector<uint64_t>>& data){
    return raw_encode_data(data);
}

// client
void MultiPath::setup_cmp(int cmp_type, int l, int m, int extra){
    if (l!=1){
        cout<<"please choose the l to 1, get the best performance."<<endl;
    }
    
    //
    int log_tree_depth = static_cast<int>(std::ceil(std::log2(tree_depth) + 1));
    extra = log_tree_depth + extra;

    //switch (cmp_type) {
    //    case 1: cmp = make_unique<DCMP>(l, m, extra, true); break;
    //}
    cmp = make_unique<DCMP>(l, m, extra, true);

    lhe = cmp->lhe;

}

vector<vector<Ciphertext>> MultiPath::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    auto extract_data = feature_extract(data_cipher, tree_flatten);
    const uint64_t new_rows = tree_flatten.index_flatten.size();

    vector<Ciphertext> cmp_raw_out(new_rows);
    for (size_t i = 0; i < new_rows; ++i){
        cmp_raw_out[i] = cmp->great_than(tree_flatten.threshold_pt[i], extract_data[i]);
        // select the left node when t > x[a].
        lhe->sub_plain_inplace(cmp_raw_out[i], tree_flatten.direction_pt[i]);
    }

    auto cmp_out = expend_compare_result(cmp_raw_out, tree_flatten);
    //cout<< "cmp_out.size(): " << cmp_out.size() <<endl;
    
    auto out = lhe->multiply_many(cmp_out);
    // pir
    lhe->multiply_plain_inplace(out, tree_flatten.leaf_values_pt);

    return vector<vector<Ciphertext>>{vector<Ciphertext>{out}};
}

vector<uint64_t> MultiPath::recover(vector<vector<Ciphertext>>& a){
    uint64_t out = 0;
    auto temp = cmp->recover(a[0][0]);
    for (size_t i = 0; i < temp.size();i++){
        if (temp[i]!=0 ){
            out = temp[i];
            if (out > (lhe->plain_modulus / 2)){
                return vector<uint64_t>{lhe->plain_modulus - out};
            }
        }
    }
    return vector<uint64_t>{out};
}