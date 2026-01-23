#include"lhe.h"

Plaintext LHE::encode(const std::vector<uint64_t>& a){
    Plaintext pt;
    batch_encoder->encode(a, pt);
    return pt;
}

Ciphertext LHE::encrypt(const Plaintext& pt){
    Ciphertext ct;
    encryptor->encrypt(pt, ct);
    return ct;
}

Ciphertext LHE::encrypt(const std::vector<uint64_t>& a){
    auto pt = encode(a);
    auto ct = encrypt(pt);
    return ct;
}

std::vector<uint64_t> LHE::decode(const Plaintext& pt){
    std::vector<uint64_t> a;
    batch_encoder->decode(pt, a);
    return a;
}

Ciphertext LHE::multiply_plain(const Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);

    Ciphertext out;
    evaluator->multiply_plain(ct, pt, out);
    return out;
}

void LHE::multiply_plain_inplace(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->multiply_plain_inplace(ct, pt);
}
void LHE::multiply_plain_inplace(Ciphertext& ct, vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);
    evaluator->multiply_plain_inplace(ct, pt);
}

Ciphertext LHE::multiply_plain(const Ciphertext& ct, const std::vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);

    return multiply_plain(ct,pt);
}

Ciphertext LHE::multiply(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    Ciphertext out;
    evaluator->multiply(ct1, ct2, out);
    return out;
}

Ciphertext LHE::multiply_many(vector<Ciphertext>& ct_many){
    //mod_switch(ct1, ct2);
    Ciphertext out;
    evaluator->multiply_many(ct_many, rlk, out);
    return out;
}


void LHE::multiply_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->multiply_inplace(ct1, ct2);
}

Ciphertext LHE::rotate_rows(const Ciphertext& ct, int step){
    Ciphertext out;
    evaluator->rotate_rows(ct, step, gal_keys, out);
    return out;
}

void LHE::add_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->add_inplace(ct1, ct2);
}

void LHE::sub_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->sub_inplace(ct1, ct2);
}

void LHE::sub_plain_inplace(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->sub_plain_inplace(ct, pt);
}

Ciphertext LHE::sub_plain(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    Ciphertext out;
    evaluator->sub_plain(ct, pt, out);
    return out;
}

Ciphertext LHE::add(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    Ciphertext out;
    evaluator->add(ct1, ct2, out);
    return out;
}

void LHE::add_plain_inplace(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->add_plain_inplace(ct, pt);
}

void LHE::add_plain_inplace(Ciphertext& ct, const std::vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);
    add_plain_inplace(ct, pt);
}

Ciphertext LHE::add_plain(const Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    Ciphertext out;
    evaluator->add_plain(ct, pt, out);
    return out;
}

Ciphertext LHE::add_plain(const Ciphertext& ct, const std::vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);
    return add_plain(ct,a);
}

Ciphertext LHE::add_many(vector<Ciphertext>& ct_many){
    //mod_switch(ct1, ct2);
    Ciphertext out;
    evaluator->add_many(ct_many, out);
    return out;
}

void LHE::relinearize_inplace(Ciphertext& ct){
    evaluator->relinearize_inplace(ct, rlk);
}

void LHE::negate_inplace(Ciphertext& ct){
    evaluator->negate_inplace(ct);
}

void LHE::negate(Ciphertext& ct1, Ciphertext& ct2){
    evaluator->negate(ct1, ct2);
}

long LHE::rlk_size() {
    std::stringstream data_stream;
    return rlk.save(data_stream);
}

long LHE::pk_size() {
    std::stringstream data_stream;
    return pk.save(data_stream);
}

long LHE::glk_size() {
    std::stringstream data_stream;
    return gal_keys.save(data_stream);
}