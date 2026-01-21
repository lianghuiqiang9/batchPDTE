#ifndef __PDTE__
#define __PDTE__

#include"node.h"
#include"cmp.h"
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
    PDTE(uint8_t shuffle);

    //server
    shared_ptr<Node> load_tree(string filename);

    // client
    void setup_cmp(int cmp_type, int l, int m, int n = 0, int extra = 0);

    // client
    vector<vector<uint64_t>> load_data(string filename, int data_rows);

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

    // server
    LeafFlatten encode_tree(shared_ptr<Node> root);

    vector<vector<Plaintext>> init_salts(int row, int cols);

    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten);


    vector<vector<Ciphertext>> extended_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten);

    vector<vector<Ciphertext>> shuffle_result(vector<vector<Ciphertext>> out, int leaf_num);

    vector<vector<Ciphertext>> adapted_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten);

    vector<Ciphertext> sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten);

    void clear_up(vector<vector<Ciphertext>>& result);
    
    long communication_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2);

    Plaintext init_d_factorial_inv_pt();

    vector<Plaintext> init_tree_depth_vec();

    // [0, ..., d-1] --> [0,1]
    vector<Ciphertext> map_to_boolean(vector<Ciphertext>& a);

    Ciphertext private_info_retrieval(vector<Ciphertext> a, vector<Plaintext> b, vector<uint64_t> b_vec);

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a);

    vector<uint64_t> recover_shuffle_result(vector<vector<Ciphertext>>& a);

    bool verify(const vector<uint64_t>& result, shared_ptr<Node> root, const vector<vector<uint64_t>>& data);


    bool verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result);

    void print();
};


#endif