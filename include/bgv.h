#ifndef __BGV__
#define __BGV__

#include"lhe.h"

class BGV :public LHE {
public:

    BGV(int depth = 3, vector<int> steps = vector<int>{}, bool is_rotate = false);

    ~BGV() = default;

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2) override;

    void mod_switch(const Ciphertext& ct, Plaintext& pt) override;

    Plaintext decrypt(const Ciphertext& ct) override;

    int get_noise_budget(const Ciphertext& ct) override;

    void print() override;

private:
    unique_ptr<Decryptor> decryptor;
};

#endif