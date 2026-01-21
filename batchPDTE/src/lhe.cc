#include"lhe.h"




LHE::LHE(string scheme, int depth, vector<int> steps) : parms((scheme == "bfv") ? scheme_type::bfv : scheme_type::bgv) {
        this->scheme = scheme;
        this->scheme_id = (scheme == "bfv") ? 0x1 : 0x3;
        this->depth = depth;
        this->steps = steps;

        uint64_t log_poly_mod_degree = 13;
        uint64_t prime_bitlength = 17;
        vector<int> bits;

        if (scheme == "bfv"){

            if (depth <= 4) {
                bits = vector<int>{43, 43, 44, 44, 44}; 

            } else if (depth <= 8) {
                bits = vector<int>{ 48, 48, 48, 48, 48, 48, 48 }; 

            } else if (depth <= 12){
                log_poly_mod_degree = 14;
                bits = vector<int>{48, 48, 48, 49, 49, 49, 49, 49, 49}; 

            } else{
                cout<<" the max depth is large than 12, should choose params manually"<<endl;
                exit(0);
            }
        }else{
            // this depth is for fresh ciphertext.
            if (depth <= 4) {
                bits = {50, 30, 30, 30, 30, 40}; 
            } 
            else if (depth <= 8) {
                log_poly_mod_degree = 14;
                bits = vector<int>(depth + 2, 40);
                bits.front() = 50;
                bits.back() = 50; 
            } 
            else if (depth <= 12) {
                log_poly_mod_degree = 15;
                bits = vector<int>(depth + 2, 45);
                bits.front() = 55;
                bits.back() = 55; 
            } else{
                cout<<" the max depth is large than 12, should choose params manually"<<endl;
                exit(0);
            }
        }

        auto coeff_modulus = CoeffModulus::Create(1 << log_poly_mod_degree, bits);

        this->log_poly_mod_degree = log_poly_mod_degree;
        parms.set_poly_modulus_degree(1 << log_poly_mod_degree);
        parms.set_plain_modulus(PlainModulus::Batching(1 << log_poly_mod_degree, prime_bitlength));
        parms.set_coeff_modulus(coeff_modulus);

        context = make_shared<SEALContext>(parms);
        KeyGenerator keygen(*context);
        PublicKey pk;
        keygen.create_public_key(pk);
        keygen.create_relin_keys(rlk);
        
        // vector<uint32_t> elts = { 3, 9, 27 }; // for steps: 1, 2, 3
        steps.size() == 0 ? keygen.create_galois_keys(gal_keys) : keygen.create_galois_keys(steps, gal_keys);
        
        encryptor = make_unique<Encryptor>(*context, pk);
        decryptor = make_unique<Decryptor>(*context, keygen.secret_key());
        evaluator = make_unique<Evaluator>(*context);
        batch_encoder = make_unique<BatchEncoder>(*context);

        plain_modulus = parms.plain_modulus().value();
        slot_count = batch_encoder->slot_count();

    }

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

Plaintext LHE::decrypt(const Ciphertext& ct){
    Plaintext pt;
    decryptor->decrypt(ct, pt);
    return pt;
}

int LHE::get_noise_budget(const Ciphertext& ct) {
    return decryptor->invariant_noise_budget(ct);
}

void LHE::mod_switch(Ciphertext& ct1, Ciphertext& ct2) {
    if (scheme_id == 0x3) { // BGV Scheme
        auto context_data1 = context->get_context_data(ct1.parms_id());
        auto context_data2 = context->get_context_data(ct2.parms_id());

        if (!context_data1 || !context_data2) return;

        size_t level1 = context_data1->chain_index();
        size_t level2 = context_data2->chain_index();

        if (level1 > level2) {
            evaluator->mod_switch_to_inplace(ct1, ct2.parms_id());
        } else if (level2 > level1) {
            evaluator->mod_switch_to_inplace(ct2, ct1.parms_id());
        }
    }
}

void LHE::mod_switch(const Ciphertext& ct, Plaintext& pt) {
    if (scheme_id == 0x3 && ct.parms_id() != pt.parms_id()) {
        if (pt.parms_id() == parms_id_zero) {
            evaluator->transform_to_ntt_inplace(pt, context->first_parms_id());
        }
        if (pt.parms_id() != ct.parms_id()) {
            evaluator->mod_switch_to_inplace(pt, ct.parms_id());
        }
    }
}

void LHE::print(){
    cout << "System Parameters:" << endl;
    cout << "  - Scheme:        " << scheme << endl;
    cout << "  - Max Depth:     " << depth << endl;
    cout << "  - Slots:         " << slot_count << endl;
    cout << "  - Plain Modulus: " << plain_modulus << endl;
    cout << "  - Rotation Steps: [ "; 
                            for (auto e:this->steps){ cout << e << " "; } 
                                cout <<" ]"<< endl;
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
