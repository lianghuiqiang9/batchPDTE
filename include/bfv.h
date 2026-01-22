#ifndef __BFV__
#define __BFV__

#include"lhe.h"

using namespace std;
using namespace seal;

class BFV : public LHE{
public:

    BFV(int depth = 3, vector<int> steps = vector<int>{});

    ~BFV() = default;

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2) override;

    void mod_switch(const Ciphertext& ct, Plaintext& pt) override;

    void print() override;

};

#endif