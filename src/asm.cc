#include"asm.h"

ASM::ASM(){
    scheme = "bpdte_asm";
}
// server
LeafFlatten ASM::encode_tree(shared_ptr<Node> root){
    auto leaf = raw_encode_tree(root);
  
    tree_depth_factorial_inv_pt = init_d_factorial_inv_pt();
    tree_depth_vec_pt = init_tree_depth_vec();  

    return leaf;
}

// client
void ASM::setup_cmp(int cmp_type, int l, int m, int extra){
    
    int log_tree_depth = (tree_depth <= 1) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)));
   
    switch (cmp_type) {
        case 0: cmp = make_unique<TCMP>(l, m, log_tree_depth + extra); break;
        case 1: cmp = make_unique<DCMP>(l, m, log_tree_depth + extra); break;
    }

    batch_size = cmp->num_cmps;
    lhe = cmp->lhe;

    vector<uint64_t> one(cmp->slot_count, 1ULL);
    one_one_one = lhe->encode(one);

    vector<uint64_t> zero(cmp->slot_count, 0ULL);
    zero_zero_zero = lhe->encrypt(zero);
}

// server

vector<vector<Ciphertext>> ASM::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten){
    
    auto sum_path_result = sum_path(root, data_cipher, leaf_flatten);
    
    return adapted_sum_path(sum_path_result, leaf_flatten);
    
}

vector<vector<Ciphertext>> ASM::adapted_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten){

    // [0, ..., d-1] --> [0,1]
    auto out_temp = map_to_boolean(sum_path_result);

    // pir
    auto out_temp1 = private_info_retrieval(out_temp, leaf_flatten.leaf_vec_pt, leaf_flatten.leaf_vec);
    
    return vector<vector<Ciphertext>>{vector<Ciphertext>{out_temp1}};

}

vector<Plaintext> ASM::init_tree_depth_vec(){
    vector<Plaintext> temp(tree_depth);
    auto slot_count = lhe->slot_count;
    for(int i = 0; i < tree_depth; i++){
        vector<uint64_t> vec_temp(slot_count, i + 1);
        temp[i] = lhe->encode(vec_temp);
    }
    return temp;
}

Plaintext ASM::init_d_factorial_inv_pt(){
    auto d_factorial_inv = d_factorial_inv_with_sign(tree_depth, lhe->plain_modulus);
    vector<uint64_t> d_factorial_inv_vec(cmp->slot_count, d_factorial_inv);
    return lhe->encode(d_factorial_inv_vec);
}


// [0, ..., d-1] --> [0,1]
vector<Ciphertext> ASM::map_to_boolean(vector<Ciphertext>& a){
    for(size_t i = 0; i < a.size(); i++){
        vector<Ciphertext> temp(tree_depth);
        for(int j = 0; j < tree_depth; ++j){
            temp[j] =  lhe->sub_plain(a[i], tree_depth_vec_pt[j]);
        }

        a[i] = lhe->multiply_many(temp);
        lhe->multiply_plain_inplace(a[i], tree_depth_factorial_inv_pt);

    }

    return a;
}

Ciphertext ASM::private_info_retrieval(vector<Ciphertext> a, vector<Plaintext> b, vector<uint64_t> b_vec){
    for(size_t i = 0; i < a.size(); ++i){
        if(b_vec[i] != 0){ //seal ciphertext can not mult zero
            lhe->multiply_plain_inplace(a[i], b[i]);
        }else{
            a[i] = zero_zero_zero;
        }
    }

    return lhe->add_many(a);
}

vector<uint64_t> ASM::recover(vector<vector<Ciphertext>>& a){

    return cmp->recover(a[0][0]);
}

