#include"bgv.h"

BGV::BGV(int depth, vector<int> steps, bool is_rotate) {

    this->parms = EncryptionParameters(scheme_type::bgv);
    this->scheme = "bgv";
    this->steps = steps;
/////////////////////////////////////
    uint64_t log_poly_mod_degree = 13;
    uint64_t prime_bitlength = 17;
    vector<int> bits;
    if (depth <= 4) {
        this->depth = 4;
        bits = {50, 30, 30, 30, 30, 40}; 
    } 
    else if (depth <= 8) {
        this->depth = 8;
        log_poly_mod_degree = 14;
        bits = vector<int>(depth + 2, 40);
        bits.front() = 50;
        bits.back() = 50; 
    } 
    else if (depth <= 12) {
        this->depth = 12;
        log_poly_mod_degree = 15;
        bits = vector<int>(depth + 2, 45);
        bits.front() = 55;
        bits.back() = 55; 
    } else{
        cout<<" the max depth is large than 12, should choose params manually"<<endl;
        exit(0);
    }
////////////////////////////////

    auto coeff_modulus = CoeffModulus::Create(1 << log_poly_mod_degree, bits);

    this->log_poly_mod_degree = log_poly_mod_degree;
    parms.set_poly_modulus_degree(1 << log_poly_mod_degree);
    parms.set_plain_modulus(PlainModulus::Batching(1 << log_poly_mod_degree, prime_bitlength));
    parms.set_coeff_modulus(coeff_modulus);

    context = make_shared<SEALContext>(parms);
    KeyGenerator keygen(*context);
    keygen.create_public_key(pk);
    keygen.create_relin_keys(rlk);

    if (is_rotate){
        if(steps.size() == 0){
            keygen.create_galois_keys(gal_keys);
        }else{
            auto elts = context->key_context_data()->galois_tool()->get_elts_from_steps(steps);
            keygen.create_galois_keys(elts, gal_keys);
        }
    }

    encryptor = make_unique<Encryptor>(*context, pk);
    decryptor = make_unique<Decryptor>(*context, keygen.secret_key());
    evaluator = make_unique<Evaluator>(*context);
    batch_encoder = make_unique<BatchEncoder>(*context);

    plain_modulus = parms.plain_modulus().value();
    slot_count = batch_encoder->slot_count();

}

void BGV::mod_switch(Ciphertext& ct1, Ciphertext& ct2) {
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

void BGV::mod_switch(const Ciphertext& ct, Plaintext& pt) {
    if ( ct.parms_id() != pt.parms_id()) {
        if (pt.parms_id() == parms_id_zero) {
            evaluator->transform_to_ntt_inplace(pt, context->first_parms_id());
        }
        if (pt.parms_id() != ct.parms_id()) {
            evaluator->mod_switch_to_inplace(pt, ct.parms_id());
        }
    }
}

Plaintext BGV::decrypt(const Ciphertext& ct){
    Plaintext pt;
    decryptor->decrypt(ct, pt);
    return pt;
}

int BGV::get_noise_budget(const Ciphertext& ct) {
    return decryptor->invariant_noise_budget(ct);
}

void BGV::print(){
    cout << "LHE Parameters:" << endl;
    cout << "  - Scheme:        " << scheme << endl;
    cout << "  - Max Depth:     " << depth << endl;
    cout << "  - Slots:         " << slot_count << endl;
    cout << "  - Plain Modulus: " << plain_modulus << endl;
    cout << "  - Rotation Steps: [ "; 
                            for (auto e:this->steps){ cout << e << " "; } 
                                cout <<" ]"<< endl;
}
