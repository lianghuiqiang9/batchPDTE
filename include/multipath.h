#ifndef __MULTIPATH__
#define __MULTIPATH__

#include"pdte.h"
using namespace std;

class MultiPath : public PDTE {
    public:

    vector<vector<Plaintext>> salts;

    MultiPath();
    ~MultiPath() = default;

    void setup_cmp(int cmp_type, int l, int m, int extra = 0) override;
    
    // server
    vector<vector<Ciphertext>> evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten) override;

    vector<uint64_t> recover(vector<vector<Ciphertext>>& a) override;

};


#endif