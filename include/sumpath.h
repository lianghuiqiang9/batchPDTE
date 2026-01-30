#ifndef __SUMPATH__
#define __SUMPATH__

#include"pdte.h"
using namespace std;

class SumPath : public PDTE {
    public:

    Plaintext one_zero_zero;
    Plaintext neg_one_zero_zero;
    Plaintext neg_two_zero_zero;
    Plaintext salt1_pt;
    Plaintext salt2_pt;

    SumPath();
    ~SumPath() = default;

    TreeFlatten encode_tree(shared_ptr<Node> root) override;

    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data) override;

    void setup_cmp(int cmp_type, int l, int m, int extra = 0) override;
    
    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten) override;

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a) override;

};


#endif