#include"cmp.h"

bool CMP::verify(const vector<bool>& actural_result, const vector<uint64_t>& result){
    for(uint64_t i = 0;i < num_cmps; ++i){
        //cout<<"actural_result: "<<actural_result[i]<< " result: "<<result[i]<<endl;
        
        if (actural_result[i]!=(result[i]==1)){
            return false;
        }
    }
    return true;
}

long CMP::communication_cost(const vector<Ciphertext>& ct1, const Ciphertext& ct2) {
    long comm = 0;
    for(const auto& e : ct1) comm += e.save_size();
    comm += ct2.save_size();
    return comm;
}

long CMP::communication_cost(const Ciphertext& ct1, const Ciphertext& ct2){
    long comm = 0;
    comm+=ct1.save_size();
    comm+=ct2.save_size();
    return comm;
}
