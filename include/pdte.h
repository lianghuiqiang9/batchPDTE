#ifndef __PDTE__
#define __PDTE__

#include"node.h"
#include"cmp.h"
#include"cdcmp.h"
//#include"tecmp.h"
#include"rdcmp.h"
#include"utils.h"
using namespace std;

class PDTE{
    public:
    string scheme;
    int tree_depth = 8;        // set it first
    int data_cols;
    int data_rows;
    unique_ptr<CMP> cmp;
    shared_ptr<LHE> lhe;
    uint64_t batch_size = 2;
    Plaintext one_one_one;
    Ciphertext zero_zero_zero;

    shared_ptr<Node> load_tree(string filename);

    vector<vector<uint64_t>> load_data(string filename, int data_rows = 2);

    void setup_cmp(int cmp_type, int l, int m, int n = 0, int extra = 0);

    // client
    vector<vector<Ciphertext>> encode_data(const vector<vector<uint64_t>>& data);

};



#endif