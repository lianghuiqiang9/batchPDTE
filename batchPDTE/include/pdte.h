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

    // shuffle
    uint8_t shuffle = 0;
    vector<vector<Plaintext>> salts;


    PDTE() = default;
    PDTE(uint8_t shuffle){
        this->shuffle = shuffle;
    };

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
    vector<vector<uint64_t>> load_data(string filename, int data_rows){
        this->data_rows = data_rows;
        auto data = load_matrix(filename, data_rows);
        
        //data_m < num_cmps 
        if(batch_size < data_rows){
            cout<<"data_rows : "<<data_rows<<" >= batch_size : "<<batch_size<<endl;
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

        // shuffle
        if (shuffle != 0){
            this->salts = init_salts(2, leaf_vec.size());
        }
        
        return LeafFlatten{leaf_vec, leaf_vec_pt};

    }

    vector<vector<Plaintext>> init_salts(int row, int cols){
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

    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten){
        
        auto sum_path_result = sum_path(root, data_cipher, leaf_flatten);
        
        if (shuffle != 0){
            return extended_sum_path(sum_path_result, leaf_flatten);
        }

        return adapted_sum_path(sum_path_result, leaf_flatten);
        

    }


    vector<vector<Ciphertext>> extended_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten){

        // record the zero position
        vector<vector<Ciphertext>> out(2, vector<Ciphertext>(0));
        out[0] = sum_path_result;
        out[1] = sum_path_result;
        auto leaf_vec_pt = leaf_flatten.leaf_vec_pt;
        auto leaf_num = leaf_vec_pt.size();
        for(int i = 0; i < leaf_num; i++){
            //evaluator->multiply_plain_inplace(out[0][i],salts[0][i]);
            //evaluator->multiply_plain_inplace(out[1][i],salts[1][i]);
            //evaluator->add_plain_inplace(out[1][i], leaf_flatten.leaf_vec_pt[i]);

            lhe->multiply_plain_inplace(out[0][i], salts[0][i]);
            lhe->multiply_plain_inplace(out[1][i], salts[1][i]);
            lhe->add_plain_inplace(out[1][i], leaf_vec_pt[i]);
        }

        //this->clear_up(out);    // ?

        return shuffle_result(out, leaf_num);
    }

    vector<vector<Ciphertext>> shuffle_result(vector<vector<Ciphertext>> out, int leaf_num){
        auto leaf_num_perm = random_permutation(leaf_num);
        vector<Ciphertext> x(leaf_num);
        vector<Ciphertext> y(leaf_num);
        
        // shuffle the leaf position
        for(int i = 0; i < leaf_num; i++){
            x[i] = std::move(out[0][leaf_num_perm[i]]);
            y[i] = std::move(out[1][leaf_num_perm[i]]);
        }

        // shuffle the leaf in different row.
        int log_data_rows = log2(data_rows + 1);
        int half_data_rows = (data_rows + 1) / 2;
        vector<uint64_t> W0(lhe->slot_count);
        vector<uint64_t> W1(lhe->slot_count); 
    
        vector<Ciphertext> x_temp0(leaf_num);
        vector<Ciphertext> y_temp0(leaf_num);
        vector<Ciphertext> x_temp1(leaf_num);
        vector<Ciphertext> y_temp1(leaf_num);

        cout<<"log_data_rows " <<log_data_rows<<endl;
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

    vector<vector<Ciphertext>> adapted_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten){

        // [0, ..., d-1] --> [0,1]
        auto out_temp = map_to_boolean(sum_path_result);

        // auto zero_temp_pt = lhe->decrypt(zero_zero_zero);
        // auto zero_temp = lhe->decode(zero_temp_pt);
        // print(zero_temp, 100, "zero_zero_zero: ");

        // pir
        auto out_temp1 = private_info_retrieval(out_temp, leaf_flatten.leaf_vec_pt, leaf_flatten.leaf_vec);
        
        return vector<vector<Ciphertext>>{vector<Ciphertext>{out_temp1}};

    }

    vector<Ciphertext> sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten){
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

    void clear_up(vector<vector<Ciphertext>>& result) {
        for(int i = 0; i < result.size(); ++i){
            for(int j = 0; j < result[0].size(); ++j){
                cmp->clear_up(result[i][j]);
            }
        } 
    }
    
    long communication_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2) {
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

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a){
        if (shuffle !=0){

            vector<vector<uint64_t>> ans0;
            vector<vector<uint64_t>> ans1;
            for(int j=0;j<a[0].size();j++){
                ans0.push_back(cmp->recover(a[0][j]));
                ans1.push_back(cmp->recover(a[1][j]));
            }

            vector<uint64_t> out;
            for(int j = 0; j < data_rows ; j++){
                for(int i = 0; i < ans0.size(); i++){
                    if(ans0[i][j]==0){
                        out.push_back(ans1[i][j]);
                        break;      
                    }
                }
            }
            
            print_vec(ans0, 255, "ans0: ");
            print_vec(ans1, 255, "ans1: ");
            print_vec(out, out.size(), "out: ");

            if(out.size()<data_rows){
                cout<<"depth_need_min is too small, please the params again by add the extra value."<<endl;
                //exit(0);
            }

            return out;

        }

        return cmp->recover(a[0][0]);
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

    bool verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result){
        print_vec(expect_result, expect_result.size(),         "pdte_result   : ");
        print_vec(actural_result, actural_result.size(), "actural_result: ");
        for(int i = 0; i < actural_result.size(); ++i){
            if (actural_result[i]!=expect_result[i]){
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
