#pragma once

#include<seal/seal.h>
#include<vector>
#include<string>

using namespace std;
using namespace seal;

class LHE {
public:
    string scheme;
    uint8_t scheme_id; // 
    uint64_t depth;
    vector<int> steps;

    EncryptionParameters parms;
    shared_ptr<SEALContext> context; 
    
    Encryptor *encryptor = nullptr;
    Evaluator *evaluator = nullptr;
    BatchEncoder *batch_encoder = nullptr;
    RelinKeys rlk;
    GaloisKeys gal_keys;
    
    uint64_t log_poly_mod_degree;
    uint64_t plain_modulus;
    uint64_t slot_count;
    

    LHE(string scheme, int depth = 3, vector<int> steps = vector<int>{1}) : parms((scheme == "bfv") ? scheme_type::bfv : scheme_type::bgv) {
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
        keygen.create_galois_keys(steps, gal_keys);
        
        encryptor = new Encryptor(*context, pk);
        decryptor = new Decryptor(*context, keygen.secret_key());
        evaluator = new Evaluator(*context);
        batch_encoder = new BatchEncoder(*context);

        plain_modulus = parms.plain_modulus().value();
        slot_count = batch_encoder->slot_count();

    }

    ~LHE() {
        delete encryptor;
        delete decryptor;
        delete evaluator;
        delete batch_encoder;
    }

    Plaintext encode(const std::vector<uint64_t>& a){
        Plaintext pt;
        batch_encoder->encode(a, pt);
        return pt;
    }

    Ciphertext encrypt(const Plaintext& pt){
        Ciphertext ct;
        encryptor->encrypt(pt, ct);
        return ct;
    }

    Ciphertext encrypt(const std::vector<uint64_t>& a){
        auto pt = encode(a);
        auto ct = encrypt(pt);
        return ct;
    }

    std::vector<uint64_t> decode(const Plaintext& pt){
        std::vector<uint64_t> a;
        batch_encoder->decode(pt, a);
        return a;
    }

    Plaintext decrypt(const Ciphertext& ct){
        Plaintext pt;
        decryptor->decrypt(ct, pt);
        return pt;
    }

    void mod_switch(Ciphertext& ct1, Ciphertext& ct2) {
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

    void mod_switch(const Ciphertext& ct, Plaintext& pt) {
        if (scheme_id == 0x3 && ct.parms_id() != pt.parms_id()) {
            if (pt.parms_id() == parms_id_zero) {
                evaluator->transform_to_ntt_inplace(pt, context->first_parms_id());
            }
            if (pt.parms_id() != ct.parms_id()) {
                evaluator->mod_switch_to_inplace(pt, ct.parms_id());
            }
        }
    }

    void print(){
        cout << "System Parameters:" << endl;
        cout << "  - Scheme:        " << scheme << endl;
        cout << "  - Max Depth:     " << depth << endl;
        cout << "  - Slots:         " << slot_count << endl;
        cout << "  - Plain Modulus: " << plain_modulus << endl;
        cout << "  - Rotation Steps: [ "; 
                                for (auto e:this->steps){ cout << e << " "; } 
                                    cout <<" ]"<< endl;
    }

    Ciphertext multiply_plain(const Ciphertext& ct, Plaintext& pt){
        mod_switch(ct, pt);

        Ciphertext out;
        evaluator->multiply_plain(ct, pt, out);
        return out;
    }

    Ciphertext multiply_plain(const Ciphertext& ct, const std::vector<uint64_t>& a){
        auto pt = encode(a);
        mod_switch(ct, pt);

        return multiply_plain(ct,pt);
    }

    Ciphertext multiply(Ciphertext& ct1, Ciphertext& ct2){
        mod_switch(ct1, ct2);
        Ciphertext out;
        evaluator->multiply(ct1, ct2, out);
        return out;
    }

    Ciphertext rotate_rows(const Ciphertext& ct, int step){
        Ciphertext out;
        evaluator->rotate_rows(ct, step, gal_keys, out);
        return out;
    }

    void add_inplace(Ciphertext& ct1, Ciphertext& ct2){
        mod_switch(ct1, ct2);
        evaluator->add_inplace(ct1, ct2);
    }

    Ciphertext add(Ciphertext& ct1, Ciphertext& ct2){
        mod_switch(ct1, ct2);
        Ciphertext out;
        evaluator->add(ct1, ct2, out);
        return out;
    }

    void add_plain_inplace(Ciphertext& ct, Plaintext& pt){
        mod_switch(ct, pt);
        evaluator->add_plain_inplace(ct, pt);
    }

    void add_plain_inplace(Ciphertext& ct, const std::vector<uint64_t>& a){
        auto pt = encode(a);
        mod_switch(ct, pt);
        add_plain_inplace(ct, pt);
    }

    Ciphertext add_plain(const Ciphertext& ct, Plaintext& pt){
        mod_switch(ct, pt);
        Ciphertext out;
        evaluator->add_plain(ct, pt, out);
        return out;
    }

    Ciphertext add_plain(const Ciphertext& ct, const std::vector<uint64_t>& a){
        auto pt = encode(a);
        mod_switch(ct, pt);
        return add_plain(ct,a);
    }

    void relinearize_inplace(Ciphertext& ct){
        evaluator->relinearize_inplace(ct, rlk);
    }

private:
    Decryptor *decryptor = nullptr;

};