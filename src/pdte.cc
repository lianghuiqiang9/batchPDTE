#include"pdte.h"



//server
shared_ptr<Node> PDTE::load_tree(string filename){
    auto root = std::make_shared<Node>(filename);
    tree_depth = root->get_depth(); // only for tree_depth 
    return root;
}

// client
void PDTE::setup_cmp(int cmp_type, int l, int m, int n, int extra){
    switch (cmp_type) {
        case 0: cmp = make_unique<Tecmp>(l, m, n, tree_depth, extra); break;
        case 1: cmp = make_unique<Cdcmp>(l, m, n, tree_depth, extra); break;
        case 2: cmp = make_unique<Rdcmp>(l, m, n, tree_depth, extra);break;
    }

    batch_size = cmp->num_cmps;
    lhe = cmp->lhe;

    vector<uint64_t> one(cmp->slot_count, 1ULL);
    one_one_one = lhe->encode(one);

    vector<uint64_t> zero(cmp->slot_count, 0ULL);
    zero_zero_zero = lhe->encrypt(zero);

}

// client
vector<vector<uint64_t>> PDTE::load_data(string filename, int data_rows){
    this->data_rows = data_rows;
    auto data = load_matrix(filename, data_rows);
    
    //data_m < num_cmps 
    if(batch_size < static_cast<uint64_t>(data_rows)){
        cout<<"data_rows : "<<data_rows<<" >= batch_size : "<<batch_size<<endl;
        cout<<"data_size is too large, please divide different page until the size is small than batch_size"<<endl;
        exit(0);
    }

    this->data_cols = data[0].size();

    return data;

}

// client
vector<vector<Ciphertext>> PDTE::encode_data(const vector<vector<uint64_t>>& data){
    auto data_trans = transpose(data);
    size_t len = data_trans.size();
    vector<vector<Ciphertext>> client_input(len);
    for(size_t i = 0; i < len; i++){        
        auto raw_encode_data = cmp->raw_encode_b(data_trans[i]);
        auto cmp_encode_data = cmp->encode_b(raw_encode_data);
        client_input[i] = cmp->encrypt(cmp_encode_data);
    }
    return client_input;
}

// server
LeafFlatten PDTE::raw_encode_tree(shared_ptr<Node> root){
    stack<StackFrame> stk;
    stk.push({root.get()});

    while (!stk.empty()) {
        StackFrame frame = stk.top();
        stk.pop();

        Node* node = frame.node;

        if (!node->is_leaf()) {
            //node->threshold_bitv = tecmp_encode_a(node->threshold, l, m, m_degree);
            vector<uint64_t> threshold(batch_size, node->threshold);
            node->raw_encode_threshold = cmp->raw_encode_a(threshold);
            node->cmp_encode_threshold = cmp->encode_a(node->raw_encode_threshold);
            if (node->right) stk.push({ node->right.get() }); 
            if (node->left)  stk.push({ node->left.get() });
        }
    }
    
    // handle leaf
    auto leaf_vec = root->leaf_extract();

    vector<Plaintext> leaf_vec_pt(leaf_vec.size());
    for(size_t i = 0; i<leaf_vec.size(); i++){
        vector<uint64_t> leaf(cmp->slot_count, leaf_vec[i]);
        leaf_vec_pt[i] = lhe->encode(leaf);
    }
    
    return LeafFlatten{leaf_vec, leaf_vec_pt};
}

vector<Ciphertext> PDTE::sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten){
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
            node->right->value = cmp->great_than(node->cmp_encode_threshold, data_cipher[node->index]);

            lhe->negate(node->right->value, node->left->value);
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

void PDTE::clear_up(vector<vector<Ciphertext>>& result) {
    for(size_t i = 0; i < result.size(); ++i){
        for(size_t j = 0; j < result[0].size(); ++j){
            cmp->clear_up(result[i][j]);
        }
    } 
}

long PDTE::communication_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2) {
    long comm = 0;
    for(const auto& cte : ct1){
        for(const auto& e : cte){
            comm += e.save_size();
        }
    } 
    for(const auto& cte : ct2){
        for(const auto& e : cte){
            comm += e.save_size();
        }
    } 
    return comm;
}

bool PDTE::verify(const vector<uint64_t>& result, shared_ptr<Node> root, const vector<vector<uint64_t>>& data){
    auto actural_result = root->eval(data);
    for(size_t i = 0; i < actural_result.size(); ++i){
        
        if (actural_result[i]!=(result[i]==1)){
            return false;
        }
    }
    return true;
}

bool PDTE::verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result){
    //print_vec(expect_result, expect_result.size(),         "pdte_result   : ");
    //print_vec(actural_result, actural_result.size(), "actural_result: ");
    
    for(size_t i = 0; i < actural_result.size(); ++i){
        if (actural_result[i]!=expect_result[i]){
            return false;
        }
    }
    return true;
}
