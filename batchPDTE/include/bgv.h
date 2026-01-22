#ifndef __BGV__
#define __BGV__

#include"lhe.h"

class BGV :public LHE {
public:

    BGV(int depth = 3, vector<int> steps = vector<int>{});

    ~BGV() = default;

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2) override;

    void mod_switch(const Ciphertext& ct, Plaintext& pt) override;

    void print() override;

};

#endif