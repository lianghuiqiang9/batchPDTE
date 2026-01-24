#ifndef __ESM__
#define __ESM__

#include"bpdte.h"
using namespace std;

class ESM : public BPDTE {
    public:

    vector<vector<Plaintext>> salts;

    ESM();
    ~ESM() = default;

    // server
    LeafFlatten encode_tree(shared_ptr<Node> root) override ;

    void setup_cmp(int cmp_type, int l, int m, int n, int extra = 0);
    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten) override ;

    vector<vector<Plaintext>> init_salts(int row, int cols);

    vector<vector<Ciphertext>> extended_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten);

    vector<vector<Ciphertext>> shuffle(vector<vector<Ciphertext>> out, int leaf_num);

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a) override;

};


#endif