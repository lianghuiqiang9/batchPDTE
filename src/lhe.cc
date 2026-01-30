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

void LHE::multiply_plain_inplace(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->multiply_plain_inplace(ct, pt);
}

void LHE::multiply_plain_inplace(vector<Ciphertext>& cts, Plaintext& pt){
    for(auto&ct:cts){
        //mod_switch(ct, pt);
        evaluator->multiply_plain_inplace(ct, pt);
    }
}

void LHE::multiply_plain_inplace(Ciphertext& ct, vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);
    evaluator->multiply_plain_inplace(ct, pt);
}

vector<Ciphertext> LHE::multiply_plain(vector<Ciphertext> cts, Plaintext& pt){
    for(auto&ct:cts){
        //mod_switch(ct, pt);
        evaluator->multiply_plain_inplace(ct, pt);
    }
    return cts;
}

Ciphertext LHE::multiply_plain(Ciphertext ct, std::vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt); 
    evaluator->multiply_plain_inplace(ct, pt);
    return ct; 
}

Ciphertext LHE::multiply_plain(Ciphertext ct, Plaintext& pt) {
    mod_switch(ct, pt); 
    evaluator->multiply_plain_inplace(ct, pt);
    return ct; 
}


Ciphertext LHE::multiply(Ciphertext ct1, Ciphertext& ct2) {
    mod_switch(ct1, ct2); 
    evaluator->multiply_inplace(ct1, ct2);
    return ct1; 
}

Ciphertext LHE::multiply_many(vector<Ciphertext>& ct_many){
    //mod_switch(ct1, ct2);
    Ciphertext out;
    evaluator->multiply_many(ct_many, rlk, out);
    return out;
}

vector<Ciphertext> LHE::multiply_many(vector<vector<Ciphertext>>& ct_manys){
    //mod_switch(ct1, ct2);
    vector<Ciphertext> out(ct_manys.size());
    for (size_t i = 0; i < ct_manys.size(); ++i) {
        evaluator->multiply_many(ct_manys[i], rlk, out[i]);
    }

    return out;
}


void LHE::multiply_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->multiply_inplace(ct1, ct2);
}


void LHE::rotate_rows_inplace(Ciphertext& ct, int step){
    evaluator->rotate_rows_inplace(ct, step, gal_keys);
}

void LHE::rotate_rows_inplace(vector<Ciphertext>& cts, int step) {
    for (auto& ct : cts) {
        evaluator->rotate_rows_inplace(ct, step, gal_keys);
    }
}

Ciphertext LHE::rotate_rows(Ciphertext ct, int step) { 
    evaluator->rotate_rows_inplace(ct, step, gal_keys);
    return ct;
}

Ciphertext LHE::rotate_rows_global(Ciphertext ct, int step) { 
    if (step < 0){
        step = step + slot_count;
    }

    if (uint64_t(step) < row_count){
        evaluator->rotate_rows_inplace(ct, step, gal_keys);
    }else{
        evaluator->rotate_columns_inplace(ct, gal_keys);
        evaluator->rotate_rows_inplace(ct, step - row_count, gal_keys);
    }

    return ct;
}

vector<Ciphertext> LHE::rotate_rows(vector<Ciphertext> cts, int step) {
    for(auto& e:cts){
        evaluator->rotate_rows_inplace(e, step, gal_keys);
    }
    return cts;
}

void LHE::rotate_columns_inplace(Ciphertext& ct){
    evaluator->rotate_columns_inplace(ct, gal_keys);
}

void LHE::rotate_columns_inplace(vector<Ciphertext>& cts) {
    for (auto& ct : cts) {
        evaluator->rotate_columns_inplace(ct, gal_keys);
    }
}

Ciphertext LHE::rotate_columns(Ciphertext ct) { 
    evaluator->rotate_columns_inplace(ct, gal_keys);
    return ct;
}

vector<Ciphertext> LHE::rotate_columns(vector<Ciphertext> cts) {
    for(auto& e:cts){
        evaluator->rotate_columns_inplace(e, gal_keys);
    }
    return cts;
}


void LHE::add_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->add_inplace(ct1, ct2);
}

void LHE::add_inplace(vector<Ciphertext>& ct1, vector<Ciphertext>& ct2){
    if (ct1.size() != ct2.size()) {
        throw std::invalid_argument("Vector sizes do not match in add_inplace");
    }
    
    for(size_t i = 0; i < ct1.size(); i++){
        mod_switch(ct1[i], ct2[i]);
        evaluator->add_inplace(ct1[i], ct2[i]);
    }

}

void LHE::sub_inplace(Ciphertext& ct1, Ciphertext& ct2){
    mod_switch(ct1, ct2);
    evaluator->sub_inplace(ct1, ct2);
}

void LHE::sub_plain_inplace(Ciphertext& ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->sub_plain_inplace(ct, pt);
}

Ciphertext LHE::sub_plain(Ciphertext ct, Plaintext& pt) {
    mod_switch(ct, pt); 
    evaluator->sub_plain_inplace(ct, pt);
    return ct; 
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

Ciphertext LHE::add_plain(Ciphertext ct, Plaintext& pt){
    mod_switch(ct, pt);
    evaluator->add_plain_inplace(ct, pt);
    return ct;
}

Ciphertext LHE::add_plain(Ciphertext ct, const std::vector<uint64_t>& a){
    auto pt = encode(a);
    mod_switch(ct, pt);
    return add_plain(ct,pt);
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

Ciphertext LHE::negate(Ciphertext ct) { 
    evaluator->negate_inplace(ct);
    return ct;
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

void LHE::print(){
    cout << "LHE Parameters:" << endl;
    cout << "  - Scheme:        " << scheme << endl;
    cout << "  - Max Depth:     " << depth << endl;
    cout << "  - Slots:         " << slot_count << endl;
    cout << "  - Plain Modulus: " << plain_modulus << endl;
    cout << "  - Rotate Steps: [ "; 
    if(is_rotate){
        cout<<" 1 2 4 8, ... ]"<<endl;
    }else{
        for (auto e:this->steps){ cout << e << " "; } 
             cout <<"]"<< endl;
    }                         
}