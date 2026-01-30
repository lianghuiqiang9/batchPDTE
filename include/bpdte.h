#ifndef __BPDTE__
#define __BPDTE__

#include"node.h"
#include"cmp.h"
#include"dcmp.h"
#include"tcmp.h"
#include"utils.h"
using namespace std;

struct LeafFlatten {
    vector<uint64_t> leaf_vec;
    vector<Plaintext> leaf_vec_pt; 
};

class BPDTE {
    public:
    string scheme;
    int tree_depth = 8;        // set it first
    int data_cols;
    int data_rows;
    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;
    uint64_t batch_size = 1;
    Plaintext one_one_one;
    Ciphertext zero_zero_zero;

    // BPDTE() = default;
    virtual ~BPDTE() = default;

    //server
    shared_ptr<Node> load_tree(string filename);

    // client
    virtual void setup_cmp(int cmp_type, int l, int m, int extra = 0) = 0;

    // client
    vector<vector<uint64_t>> load_data(string filename, int data_rows = 1);

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

    // server
    LeafFlatten raw_encode_tree(shared_ptr<Node> root);

    virtual LeafFlatten encode_tree(shared_ptr<Node> root) = 0;

    // server
    virtual vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten) = 0;

    vector<Ciphertext> sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher);

    void clear_up(vector<vector<Ciphertext>>& result);

    long keys_size();
    
    long comm_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2);

    long comm_cost_estimate(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2);

    virtual vector<uint64_t> recover(vector<vector<Ciphertext>>& a) = 0;

    bool verify(const vector<uint64_t>& result, shared_ptr<Node> root, const vector<vector<uint64_t>>& data);

    bool verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result);

    void print();
};


#endif