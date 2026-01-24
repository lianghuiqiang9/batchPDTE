
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
void PDTE::setup_cmp(int cmp_type, int l, int m, int extra){
    
    int log_tree_depth = (tree_depth <= 1) ? 0 : static_cast<int>(std::ceil(std::log2(tree_depth)));
    
    switch (cmp_type) {
        case 1: cmp = make_unique<DCMP>(l, m, log_tree_depth + extra); break;
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
    auto num_cmps = cmp->num_cmps;
    auto slot_count = cmp->slot_count;
    auto num_cmps_per_row = cmp->num_cmps_per_row;

    repeat = num_cmps / 2 / data_cols ; // bfv plaintext structure.
    repeat = get_nearest_power_of_two(repeat);

    cout<<"data_cols       : "<< data_cols <<endl;
    cout<<"num_cmps        : "<< num_cmps <<endl;
    cout<<"slot_count      : "<< slot_count <<endl;
    cout<<"num_cmps_per_row: "<< num_cmps_per_row <<endl;
    cout<<"repeat          : "<< repeat <<endl;

    vector<uint64_t> data(num_cmps, 0);

    for (size_t i = 0; i < repeat * data_cols; i++){
        data[i] = raw_data[0][ i / repeat];
        if(data_rows==2){
            data[num_cmps_per_row + i ] = raw_data[1][ i/ repeat];
        }
    }

    print_vec(data, data.size(), "new_data: ");

    auto raw_encode_data = cmp->raw_encode_b(data);
    auto cmp_encode_data = cmp->encode_b(raw_encode_data);
    auto client_input = cmp->encrypt(cmp_encode_data);


    return vector<vector<Ciphertext>>{client_input};
}

vector<Plaintext> PDTE::init_tree_depth_vec(){
    vector<Plaintext> temp(tree_depth);
    auto slot_count = lhe->slot_count;
    for(int i = 0; i < tree_depth; i++){
        vector<uint64_t> vec_temp(slot_count, i + 1);
        temp[i] = lhe->encode(vec_temp);
    }
    return temp;
}

Plaintext PDTE::init_d_factorial_inv_pt(){
    auto d_factorial_inv = d_factorial_inv_with_sign(tree_depth, lhe->plain_modulus);
    vector<uint64_t> d_factorial_inv_vec(cmp->slot_count, d_factorial_inv);
    return lhe->encode(d_factorial_inv_vec);
}

// server
TreeFlatten PDTE::encode_tree(shared_ptr<Node> root){
    auto node_matrix = extract_aligned_layers(root);
    print_aligned_matrix(node_matrix);
    auto width = node_matrix[0].size();
    if (width > (cmp->num_cmps / 2)){
        cout<<" the m is to biger, choose a small one"<<endl;
    }

    auto num_cmps = cmp->num_cmps;
    auto num_cmps_per_row = cmp->num_cmps_per_row;

    vector<vector<uint64_t>> index_matrix;
    vector<vector<uint64_t>> threshold_matrix;
    vector<uint64_t> class_vec;
    extract_matrices(node_matrix, index_matrix, threshold_matrix, class_vec);
    
    //print_vec(index_matrix, index_matrix[0].size(), "index_matrix: ");
    //print_vec(threshold_matrix, threshold_matrix[0].size(), "threshold_matrix: ");
    //print_vec(class_vec, class_vec.size(), "class_vec: ");

    cout<<"data_rows: "<<data_rows<<endl;

    if (data_rows == 2) {

        //vector<vector<uint64_t>> raw_index_matrix(index_matrix.size(), vector<uint64_t>(num_cmps, 0));
        vector<vector<uint64_t>> raw_threshold_matrix(threshold_matrix.size(), vector<uint64_t>(num_cmps, 0));
        vector<uint64_t> raw_class_vec(num_cmps, 0);

        // width 通常是单棵树对齐后的宽度 (如 8)
        size_t width = threshold_matrix[0].size();

        // 填充决策层矩阵
        for (size_t i = 0; i < threshold_matrix.size(); i++) {
            for (size_t j = 0; j < width; j++) {
                //raw_index_matrix[i][j] = index_matrix[i][j];
                raw_threshold_matrix[i][j] = threshold_matrix[i][j];

                //raw_index_matrix[i][num_cmps_per_row + j] = index_matrix[i][j];
                raw_threshold_matrix[i][num_cmps_per_row + j] = threshold_matrix[i][j];
            }
        }

        for (size_t j = 0; j < width; j++) {
            raw_class_vec[j] = class_vec[j];
            raw_class_vec[num_cmps_per_row + j] = class_vec[j];
        }

        //index_matrix = std::move(raw_index_matrix);
        threshold_matrix = std::move(raw_threshold_matrix);
        class_vec = std::move(raw_class_vec);
    }

    // for cmp
    vector<vector<Plaintext>> threshold_pt(threshold_matrix.size());
    for (size_t i = 0; i < threshold_matrix.size(); i++){
        auto raw_encode_a = cmp->raw_encode_a(threshold_matrix[i]);
        threshold_pt[i] = cmp->encode_a(raw_encode_a);
    }

    // for pir
    auto class_pt = lhe->encode(class_vec);

    //print_vec(index_matrix, num_cmps, "*index_matrix: ");
    //print_vec(threshold_matrix, num_cmps, "*threshold_matrix: ");
    //print_vec(class_vec, num_cmps, "*class_vec: ");
        
    // adapted
    tree_depth_factorial_inv_pt = init_d_factorial_inv_pt();
    tree_depth_vec_pt = init_tree_depth_vec();  

    return TreeFlatten{index_matrix, threshold_pt, class_pt};
}

vector<vector<Ciphertext>> PDTE::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    auto data = data_cipher[0]; // vector 

    // flatten
    auto data_pt = lhe->decrypt(data[0]);
    auto data_vec = lhe->decode(data_pt);
    print_vec(data_vec, data_vec.size(), "data_vec: ");

    auto index_matrix = tree_flatten.index_matrix;
    auto threshold_pt = tree_flatten.threshold_pt;
    auto class_pt = tree_flatten.class_pt;

    print_vec(index_matrix, index_matrix[0].size(), "index_matrix: ");
    cout<<"repeat: "<<repeat<<" m: "<<cmp->m<<" repeat*m: "<<repeat * cmp->m<<endl;

    auto width = index_matrix[0].size();
    for(size_t i = 0; i < index_matrix.size(); i++){
        cout<<"width: "<< width << endl;

        if (width > repeat){
            auto rot = width / repeat;

            for(size_t j = 0; j < index_matrix[0].size(); j = j + width){
                cout<<"j: "<<j<<" index_matrix "<<index_matrix[i][j]<<endl;


            }
            cout<<endl;

        }else{
            Ciphertext temp;
            for(size_t j = 0; j < index_matrix[0].size(); j = j + width){
                cout<<"j: "<<j<<" index_matrix "<<index_matrix[i][j]<<endl;
                // left rotate (index_matrix[i][j] - j) * repeat * m;
                auto step =  (index_matrix[i][j] - j) * repeat * m;

                cout<<"step: "<<step<<endl;

                vector <Ciphertext> rot_data(l);
                for(int k=0;k<l;k++){
                    rot_data[k] = lhe->rotate_rows(data[k],step);
                }

                // onehot = j,j+width
                vector<uint64_t> onehot(lhe->slot_count,0);
                for (int k=j * repeat * m;k<(j+width) * repeat * m;k++){
                    onehot[k] = 1;
                    onehot[k] = 1;
                }
                auto onehot_pt= lhe->encode(onehot);


                // left rotate * onehot

                // temp = temp + (left rotate * onehot)

            }
            
            cout<<endl;

        }


        width = width/2;

    }

    // compare


    // sum


    // pir

    return data_cipher;
}