#ifndef __PDTE__
#define __PDTE__

#include"node.h"
#include"cmp.h"
#include"dcmp.h"
#include"tcmp.h"
#include"utils.h"
using namespace std;

struct TreeFlatten {
    vector<vector<uint64_t>> index_matrix;
    vector<vector<Plaintext>> threshold_pt;
    Plaintext class_pt;
};

class PDTE{
    public:
    string scheme;
    int tree_depth = 8;        // set it first
    int data_cols;
    int data_rows = 2;
    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;
    uint64_t batch_size = 2;
    Plaintext one_one_one;
    Ciphertext zero_zero_zero;

    Plaintext tree_depth_factorial_inv_pt;
    vector<Plaintext> tree_depth_vec_pt;

    //
    int repeat = 1;

    shared_ptr<Node> load_tree(string filename);

    vector<vector<uint64_t>> load_data(string filename, int data_rows = 2);

    void setup_cmp(int cmp_type, int l, int m, int extra = 0);

    vector<Plaintext> init_tree_depth_vec();

    Plaintext init_d_factorial_inv_pt();

    TreeFlatten encode_tree(shared_ptr<Node> root);

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten);

};



#endif