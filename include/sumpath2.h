#ifndef __SUMPATH2__
#define __SUMPATH2__

#include"pdte.h"
using namespace std;

class SumPath2 : public PDTE {
    public:

    Plaintext salt1_pt;
    Plaintext salt2_pt;

    Plaintext one_one_one;
    Ciphertext zero_zero_zero;
    Plaintext onehot_pt;

    SumPath2();
    ~SumPath2() = default;

    TreeFlatten encode_tree(shared_ptr<Node> root) override;

    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data) override;

    void setup_cmp(int cmp_type, int l, int m, int extra = 0) override;
    
    vector<Ciphertext> sum_path(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher);
    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten) override;

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a) override;

};


#endif