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
    int remainder;
    int new_cols; // original index_matrix cols.
    int aligned_cols; // the thresholds in one level.
};

class PDTE{
    public:
    string scheme = "pdte";
    int tree_depth = 16;        // set it first
    uint64_t data_cols;

    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;

    vector<Ciphertext> cmp_zero_b;

    uint64_t repeat = 1;

    virtual ~PDTE() = default;

    shared_ptr<Node> load_tree(string filename);

    vector<vector<uint64_t>> load_data(string filename, int data_rows = 1);

    virtual void setup_cmp(int cmp_type, int l, int m, int extra = 0) = 0;

    TreeFlatten encode_tree(shared_ptr<Node> root);

    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

    virtual vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten) = 0;

    virtual vector<uint64_t> recover(vector<vector<Ciphertext>>& a) = 0;

    vector<vector<IndexPos>> get_index_flatten(vector<vector<uint64_t>> index_matrix, uint64_t cols,  uint64_t aligned_cols); 
    
    vector<vector<Ciphertext>> feature_extract(vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten);

    vector<Ciphertext> expend_compare_result(vector<Ciphertext>& cmp_raw_out, TreeFlatten& tree_flatten);

    long keys_size();

    void print();

    bool verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result);

    long comm_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2);
};



#endif