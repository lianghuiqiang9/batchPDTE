#ifndef __ASM__
#define __ASM__

#include"bpdte.h"
using namespace std;

class ASM : public BPDTE {
    public:

    //asm
    Plaintext tree_depth_factorial_inv_pt;
    vector<Plaintext> tree_depth_vec_pt;

    ASM();
    ~ASM() = default;

    // server
    LeafFlatten encode_tree(shared_ptr<Node> root) override ;

    void setup_cmp(int cmp_type, int l, int m, int extra = 0) override;
    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, LeafFlatten& leaf_flatten) override ;

    vector<vector<Ciphertext>> adapted_sum_path(vector<Ciphertext>& sum_path_result, LeafFlatten& leaf_flatten);
    
    vector<Plaintext> init_tree_depth_vec();

    Plaintext init_d_factorial_inv_pt();

    // [0, ..., d-1] --> [0,1]
    vector<Ciphertext> map_to_boolean(vector<Ciphertext>& a);

    Ciphertext private_info_retrieval(vector<Ciphertext> a, vector<Plaintext> b, vector<uint64_t> b_vec);

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a) override;

};


#endif