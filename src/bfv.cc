#include"bfv.h"

BFV::BFV( int depth, vector<int> steps, bool is_rotate) {

    this->parms = EncryptionParameters(scheme_type::bfv);
    this->scheme = "bfv";
    this->steps = steps;
/////////////////////////////////
    uint64_t log_poly_mod_degree = 13;
    uint64_t prime_bitlength = 17;
    vector<int> bits;
    if (depth <=1){
        this->depth = 1;
        log_poly_mod_degree = 12;
        prime_bitlength = 16;
        bits = vector<int>{36, 36, 37}; 
    }else if (depth <= 4) {
        this->depth = 4;
        log_poly_mod_degree = 13;
        prime_bitlength = 17;
        bits = vector<int>{43, 43, 44, 44, 44}; 

    } else if (depth <= 8) {
        this->depth = 8;
        log_poly_mod_degree = 14;
        prime_bitlength = 17;
        bits = vector<int>{ 48, 48, 48, 48, 48, 48, 48 }; 

    } else if (depth <= 12){
        this->depth = 12;
        log_poly_mod_degree = 14;
        prime_bitlength = 17;
        bits = vector<int>{48, 48, 48, 49, 49, 49, 49, 49, 49}; 
    } else{
        cout<<" the max depth is large than 12, should choose params manually"<<endl;
        exit(0);
    }
/////////////////////////////////
    this->log_poly_mod_degree = log_poly_mod_degree;
    auto poly_degree = 1 << log_poly_mod_degree;
    auto coeff_modulus = CoeffModulus::Create(poly_degree, bits);

    parms.set_poly_modulus_degree(poly_degree);
    parms.set_plain_modulus(PlainModulus::Batching(poly_degree, prime_bitlength));
    parms.set_coeff_modulus(coeff_modulus);

    context = make_shared<SEALContext>(parms);
    KeyGenerator keygen(*context);
    keygen.create_public_key(pk);
    keygen.create_relin_keys(rlk);
           
    if (is_rotate){
        keygen.create_galois_keys(gal_keys);
    }else{
        auto elts = context->key_context_data()->galois_tool()->get_elts_from_steps(steps);
        
        // rotate_columns key, maybe unnecessary.
        uint32_t conj_elt = 2 * poly_degree - 1;
        elts.push_back(conj_elt);

        keygen.create_galois_keys(elts, gal_keys);
    }
            
    
    encryptor = make_unique<Encryptor>(*context, pk);
    decryptor = make_unique<Decryptor>(*context, keygen.secret_key());
    evaluator = make_unique<Evaluator>(*context);
    batch_encoder = make_unique<BatchEncoder>(*context);

    plain_modulus = parms.plain_modulus().value();
    slot_count = batch_encoder->slot_count();

}


void BFV::mod_switch(Ciphertext& ct1, Ciphertext& ct2) {

}

void BFV::mod_switch(const Ciphertext& ct, Plaintext& pt) {

}

Plaintext BFV::decrypt(const Ciphertext& ct){
    Plaintext pt;
    decryptor->decrypt(ct, pt);
    return pt;
}

int BFV::get_noise_budget(const Ciphertext& ct) {
    return decryptor->invariant_noise_budget(ct);
}

void BFV::print(){
    cout << "LHE Parameters:" << endl;
    cout << "  - Scheme:        " << scheme << endl;
    cout << "  - Max Depth:     " << depth << endl;
    cout << "  - Slots:         " << slot_count << endl;
    cout << "  - Plain Modulus: " << plain_modulus << endl;
    cout << "  - Rotate Steps: [ "; 
                            for (auto e:this->steps){ cout << e << " "; } 
                                cout <<" ]"<< endl;
}
