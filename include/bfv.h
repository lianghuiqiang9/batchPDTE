#ifndef __BFV__
#define __BFV__

#include"lhe.h"

using namespace std;
using namespace seal;

class BFV : public LHE{
public:

    BFV(int depth = 3, vector<int> steps = vector<int>{}, bool is_rotate = true);

    ~BFV() = default;

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2) override;

    void mod_switch(const Ciphertext& ct, Plaintext& pt) override;

    Plaintext decrypt(const Ciphertext& ct) override;

    int get_noise_budget(const Ciphertext& ct) override;

    void print() override;

private:
    unique_ptr<Decryptor> decryptor;
};

#endif