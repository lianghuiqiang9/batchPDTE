
#include"pdte.h"

//server
shared_ptr<Node> PDTE::load_tree(string filename){
    auto root = std::make_shared<Node>(filename);
    tree_depth = root->get_depth(); // only for tree_depth 
    return root;
}


// client
vector<vector<uint64_t>> PDTE::load_data(string filename, int data_rows){
    this->data_rows = data_rows;
    auto data = load_matrix(filename, data_rows);
    
    //data_m > num_cmps 
    if(static_cast<uint64_t>(data_rows) > batch_size){
        cout<<"data_rows : "<<data_rows<<" > batch_size : "<<batch_size<<endl;
        cout<<"data_size is too large, please divide different page until the size is small than batch_size"<<endl;
        exit(0);
    }

    this->data_cols = data[0].size();

    return data;

}

// client
void PDTE::setup_cmp(int cmp_type, int l, int m, int n, int extra){
    
    int log_tree_depth = (tree_depth <= 1) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)));
   
    switch (cmp_type) {
        // tecmp is not suitable for this case.
        //case 0: cmp = make_unique<Tecmp>(l, m, n, log_tree_depth + extra); break;
        case 1: cmp = make_unique<Cdcmp>(l, m, n, log_tree_depth + extra); break;
        case 2: cmp = make_unique<Rdcmp>(l, m, n, log_tree_depth + extra);break;
    }

    batch_size = cmp->num_cmps;
    lhe = cmp->lhe;

    vector<uint64_t> one(cmp->slot_count, 1ULL);
    one_one_one = lhe->encode(one);

    vector<uint64_t> zero(cmp->slot_count, 0ULL);
    zero_zero_zero = lhe->encrypt(zero);
}

// client
vector<vector<Ciphertext>> PDTE::encode_data(const vector<vector<uint64_t>>& raw_data){
    auto data = raw_data[0];

    auto num_cmps = cmp->num_cmps;
    auto num_cmps_per_row = cmp->num_cmps / 2;
    auto repeat = num_cmps_per_row / data_cols ; // bfv plaintext structure.

    vector<uint64_t> new_data(cmp->num_cmps, 0);
    if(raw_data.size()==1){
        for (int i = 0; i< new_data.size(); i++){
            new_data[i] = data[ i / repeat];
        }
    }else{


    }

    print_vec(new_data, new_data.size(), "new_data: ");

    auto raw_encode_data = cmp->raw_encode_b(new_data);
    auto cmp_encode_data = cmp->encode_b(raw_encode_data);
    auto client_input = cmp->encrypt(cmp_encode_data);


    return vector<vector<Ciphertext>>{client_input};
}