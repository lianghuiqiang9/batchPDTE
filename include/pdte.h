#ifndef __PDTE__
#define __PDTE__

#include"node.h"
#include"cmp.h"
#include"dcmp.h"
#include"tcmp.h"
#include"utils.h"
using namespace std;

struct IndexPos{
    uint64_t index;
    uint64_t start;
    uint64_t width;
};

struct TreeFlatten {
    vector<vector<IndexPos>> index_flatten;
    vector<vector<Plaintext>> threshold_pt;
    vector<Plaintext> direction_pt;
    Plaintext leaf_values_pt;
    int remiander;
    int new_cols; // original index_matrix cols.
    int aligned_cols; // the thresholds in one level.
};

class PDTE{
    public:
    string scheme = "pdte";
    int tree_depth = -1;        // set it first
    int data_cols;
    //int data_rows = 1;

    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;
    //Plaintext one_one_one;
    //Ciphertext zero_zero_zero;

    vector<Ciphertext> cmp_zero_b;

    //Plaintext tree_depth_factorial_inv_pt;
    //vector<Plaintext> tree_depth_vec_pt;

    //
    int repeat = 1;
    bool rot_flag = false;

    shared_ptr<Node> load_tree(string filename);

    vector<vector<uint64_t>> load_data(string filename, int data_rows = 1);

    void setup_cmp(int cmp_type, int l, int m, int extra = 0);

    //vector<Plaintext> init_tree_depth_vec();

    //Plaintext init_d_factorial_inv_pt();
    uint64_t get_optimized_aligned_cols(uint64_t cols, uint64_t rows, uint64_t num_cmps);

    vector<vector<IndexPos>> get_index_flatten(vector<vector<uint64_t>> index_matrix, uint64_t cols,  uint64_t aligned_cols); 
    TreeFlatten encode_tree(shared_ptr<Node> root);

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten);

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a);

    long keys_size();

    void print();

    bool verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result);

    long comm_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2);
};



#endif