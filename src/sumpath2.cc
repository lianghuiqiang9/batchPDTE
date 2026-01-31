#include "sumpath2.h"

SumPath2::SumPath2(){
    scheme = "pdte_sumpath2";
}

TreeFlatten SumPath2::encode_tree(shared_ptr<Node> root){
    
    // raw_encode_tree2
    vector<uint64_t> leaf_vec;
    stack<StackFrame> stk;
    stk.push({root.get()});

    while (!stk.empty()) {
        StackFrame frame = stk.top();
        stk.pop();

        Node* node = frame.node;

        if (!node->is_leaf()) {
            vector<uint64_t> threshold(cmp->num_cmps, node->threshold);
            auto raw_encode_threshold = cmp->raw_encode_a(threshold);
            node->cmp_encode_threshold = cmp->encode_a(raw_encode_threshold);
            if (node->right) stk.push({ node->right.get() }); 
            if (node->left)  stk.push({ node->left.get() });
        }else{
            leaf_vec.push_back(node->leaf_value);
        }
    }
    // print_vector(leaf_vec, leaf_vec.size(), " leaf_vec: ");
    auto leaf_values_pt = lhe->encode(leaf_vec);
    TreeFlatten out;
    out.leaf_values_pt = std::move(leaf_values_pt);

    // salt
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(1, lhe->plain_modulus - 1);
    vector<uint64_t> salt1(cmp->slot_count);
    vector<uint64_t> salt2(cmp->slot_count);
    for (uint64_t i = 0; i < cmp->slot_count; ++i) {
        salt1[i] = distrib(gen);
        salt2[i] = distrib(gen);
    }
    salt1_pt = lhe->encode(salt1); // different with sumpath
    salt2_pt = lhe->encode(salt2);

    // sumpath2 info.
    vector<uint64_t> one(cmp->slot_count, 1ULL);
    one_one_one = lhe->encode(one);
    vector<uint64_t> zero(cmp->slot_count, 0ULL);
    zero_zero_zero = lhe->encrypt(zero);
    vector<uint64_t> onehot(cmp->slot_count, 0);
    onehot[0] = 1;
    onehot_pt = lhe->encode(onehot);

    return out;
}

vector<vector<Ciphertext>> SumPath2::encode_data(const vector<vector<uint64_t>>& raw_data){
    uint64_t num_cmps = cmp->num_cmps;
    data_cols = raw_data[0].size();
    uint64_t rows = (data_cols + num_cmps - 1) / num_cmps;

    vector<vector<uint64_t>> data(rows, vector<uint64_t>(num_cmps, 0));

    for (uint64_t i = 0; i < data_cols; ++i) {
        uint64_t r = i / num_cmps;
        uint64_t c = i % num_cmps; 
        data[r][c] = raw_data[0][i];
    }
    // print_matrix(data, rows, num_cmps, "data: ");

    vector<vector<Ciphertext>> client_input(rows);
    for(uint64_t i = 0; i < rows; ++i){
        auto raw_encode_data = cmp->raw_encode_b(data[i]);
        auto cmp_encode_data = cmp->encode_b(raw_encode_data);
        client_input[i] = cmp->encrypt(cmp_encode_data);
    }
    return client_input;

}

// client
void SumPath2::setup_cmp(int cmp_type, int l, int m, int extra){
    if (l!=1){
        cout<<"please choose the l to 1, get the best performance."<<endl;
    }

    switch (cmp_type) {
        case 0: cmp = make_unique<TCMP>(l, m, extra, true); break;
        case 1: cmp = make_unique<DCMP>(l, m, extra, true); break;
    }

    lhe = cmp->lhe;
}

vector<vector<Ciphertext>> SumPath2::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    uint64_t num_cmps = cmp->num_cmps;
    vector<vector<Ciphertext>> extract_data(data_cols);
    for(uint64_t i = 0; i < data_cols; ++i){
        uint64_t r = i / num_cmps; 
        uint64_t c = i % num_cmps; 
        extract_data[i] = cmp->exchange(data_cipher[r], 0, c);
    }

    auto sum_path_result = sum_path(root, extract_data);

    Ciphertext out1;
    for(size_t i = 0; i < sum_path_result.size(); ++i){
        lhe->multiply_plain_inplace(sum_path_result[i], onehot_pt);
        lhe->rotate_rows_inplace(sum_path_result[i], -i);
        if (i == 0){
            out1 = std::move(sum_path_result[i]);
        }else{
            lhe->add_inplace(out1, sum_path_result[i]);
        }
    }
    auto out2 = lhe->multiply_plain(out1, salt2_pt);
    lhe->add_plain_inplace(out2, tree_flatten.leaf_values_pt);
    lhe->multiply_plain_inplace(out1, salt1_pt);

    return vector<vector<Ciphertext>>{vector<Ciphertext>{out1, out2}};
}

// same with BPDTE::sum_path
vector<Ciphertext> SumPath2::sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher){
    vector<Ciphertext> out;
    root->value = zero_zero_zero;
    stack<StackFrame> stk;
    stk.push({root.get(), false});

    while (!stk.empty()) {
        StackFrame frame = stk.top();
        stk.pop();

        Node* node = frame.node;

        if (node->is_leaf()) {
            out.push_back(node->value);
            continue;
        }

        if (!frame.visited) {
            //      c
            //  r      1-r
            //  r+c    1-r+c
            if (node->threshold == 0){
                node->right->value = zero_zero_zero;
            }else{
                node->right->value = cmp->great_than(node->cmp_encode_threshold, data_cipher[node->index]);
            }
            node->left->value = lhe->negate(node->right->value);
            lhe->add_plain_inplace(node->left->value, one_one_one);
            lhe->add_inplace(node->left->value, node->value);
            lhe->add_inplace(node->right->value, node->value);

            stk.push(StackFrame{ node, true });
            stk.push(StackFrame{ node->right.get(), false });
            stk.push(StackFrame{ node->left.get(), false });
        }
    }
    return out;
}

vector<uint64_t> SumPath2::recover(vector<vector<Ciphertext>>& a){
    auto temp1_pt = lhe->decrypt(a[0][0]);
    auto temp2_pt = lhe->decrypt(a[0][1]);
    auto temp1 = lhe->decode(temp1_pt);
    auto temp2 = lhe->decode(temp2_pt);
    //print_vector(temp1, 10, " temp1: ");
    //print_vector(temp2, 10, " temp2: ");
    for (size_t i = 0; i < temp1.size(); i++){
        if (temp1[i]== 0){
            return vector<uint64_t>{temp2[i]};
        }
    }
    return vector<uint64_t>{0};
}