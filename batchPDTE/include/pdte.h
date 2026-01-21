#pragma once
#include"node.h"
#include"cmp.h"
#include"serial.h"
#include"cdcmp.h"
#include"tecmp.h"
#include"rdcmp.h"
#include"utils.h"
using namespace std;

struct LeafFlatten {
    vector<uint64_t> leaf_vec;
    vector<Plaintext> leaf_vec_pt; 
};

class PDTE {
    public:

    int tree_depth = 10;        // set it first
    int data_cols;
    int data_rows;
    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;
    uint64_t batch_size = 1;
    Plaintext one_one_one;
    Ciphertext zero_zero_zero;
    Plaintext tree_depth_factorial_inv_pt;
    vector<Plaintext> tree_depth_vec_pt;


    PDTE() = default;

    //server
    shared_ptr<Node> load_tree(string filename){
        auto root = std::make_shared<Node>(filename);
        tree_depth = root->get_depth(); // only for tree_depth 
        return root;
    }

    // client
    void setup_cmp(int cmp_type, int l, int m, int n = 0, int extra = 0){
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

        tree_depth_factorial_inv_pt = init_d_factorial_inv_pt();
        tree_depth_vec_pt = init_tree_depth_vec();
    }


    // client
    vector<vector<uint64_t>> load_data(string filename, int data_size){
        this->data_rows = data_size;
        auto data = load_matrix(filename, data_size);
        
        //data_m < num_cmps 
        if(batch_size < data_size){
            cout<<"data_size : "<<data_size<<" >= batch_size : "<<batch_size<<endl;
            cout<<"data_size is too large, please divide different page until the size is small than batch_size"<<endl;
            exit(0);
        }

        this->data_cols = data[0].size();

        return data;

    }

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data){
        auto data_trans = transpose(data);

        vector<vector<Ciphertext>> client_input;
        for(int i = 0; i < data_trans.size(); i++){        
            auto raw_encode_data = cmp->raw_encode_b(data_trans[i]);
            auto cmp_encode_data = cmp->encode_b(raw_encode_data);
            auto cmp_encode_data_cipher = cmp->encrypt(cmp_encode_data);
            client_input.push_back(cmp_encode_data_cipher);
        }
        return client_input;
    }

    // server
    LeafFlatten encode_tree(shared_ptr<Node> root){
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
        for(int i = 0; i<leaf_vec.size(); i++){
            vector<uint64_t> leaf(cmp->slot_count, leaf_vec[i]);
            leaf_vec_pt[i] = lhe->encode(leaf);
        }

        return LeafFlatten{leaf_vec, leaf_vec_pt};

    }

    // server
    Ciphertext evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten, vector<vector<uint64_t>>& data){
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

        // [0, ..., d-1] --> [0,1]
        out = map_to_boolean(out);

        // auto zero_temp_pt = lhe->decrypt(zero_zero_zero);
        // auto zero_temp = lhe->decode(zero_temp_pt);
        // print(zero_temp, 100, "zero_zero_zero: ");

        // pir
        auto output = private_info_retrieval(out, leaf_flatten.leaf_vec_pt, leaf_flatten.leaf_vec);

        return output;

    }

    void clear_up(Ciphertext& result) {
        cmp->clear_up(result);
    }
    
    long communication_cost(const vector<vector<Ciphertext>>& ct1, const Ciphertext& ct2) {
        long comm = 0;
        for(const auto& cte : ct1){
            for(const auto& e : cte){
                comm += e.save_size();
            }
        } 
        comm += ct2.save_size();
        return comm;
    }

    Plaintext init_d_factorial_inv_pt(){
        auto d_factorial_inv = d_factorial_inv_with_sign(tree_depth, lhe->plain_modulus);
        vector<uint64_t> d_factorial_inv_vec(cmp->slot_count, d_factorial_inv);
        return lhe->encode(d_factorial_inv_vec);
    }

    vector<Plaintext> init_tree_depth_vec(){
        vector<Plaintext> temp(tree_depth);
        auto slot_count = lhe->slot_count;
        for(int i = 0; i < tree_depth; i++){
            vector<uint64_t> vec_temp(slot_count, i + 1);
            temp[i] = lhe->encode(vec_temp);
        }
        return temp;
    }

    // [0, ..., d-1] --> [0,1]
    vector<Ciphertext> map_to_boolean(vector<Ciphertext>& a){
        for(int i = 0; i < a.size(); i++){
            vector<Ciphertext> temp(tree_depth);
            for(int j = 0; j < tree_depth; ++j){
                temp[j] =  lhe->sub_plain(a[i], tree_depth_vec_pt[j]);
            }

            a[i] = lhe->multiply_many(temp);
            lhe->multiply_plain_inplace(a[i], tree_depth_factorial_inv_pt);

        }

        return a;
    }

    Ciphertext private_info_retrieval(vector<Ciphertext> a, vector<Plaintext> b, vector<uint64_t> b_vec){
        for(int i = 0; i < a.size(); ++i){
            if(b_vec[i] != 0){ //seal ciphertext can not mult zero
                //evaluator->multiply_plain_inplace(in_first[i],in_second[i]);
                lhe->multiply_plain_inplace(a[i], b[i]);
            }else{
                a[i] = zero_zero_zero;
            }
        }
        //evaluator->add_many(in_first, out);

        return lhe->add_many(a);
    }

    vector<uint64_t> recover(Ciphertext& a){
        return cmp->recover(a);
    }

    bool verify(const vector<uint64_t>& result, shared_ptr<Node> root, const vector<vector<uint64_t>>& data){
        auto actural_result = root->eval(data);
        for(int i = 0; i < actural_result.size(); ++i){
            
            if (actural_result[i]!=(result[i]==1)){
                return false;
            }
        }
        return true;
    }

    bool verify(const vector<uint64_t>& result, const vector<uint64_t>& actural_result){
        //print(result, actural_result.size(),         "pdte_result   : ");
        //print(actural_result, actural_result.size(), "actural_result: ");
        for(int i = 0; i < actural_result.size(); ++i){
            if (actural_result[i]!=result[i]){
                return false;
            }
        }
        return true;
    }

    void print(){
        cout << " pdte name                                : " << "pdte"
             << " \n cmp info" << endl;
        cmp->print();
        cout << endl;

    }
};
