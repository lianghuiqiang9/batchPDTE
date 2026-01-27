
#include"pdte.h"

//server
shared_ptr<Node> PDTE::load_tree(string filename){
    auto root = std::make_shared<Node>(filename);
    tree_depth = root->get_depth(); // only for tree_depth 
    return root;
}


// client
vector<vector<uint64_t>> PDTE::load_data(string filename, int data_rows){
    //this->data_rows = data_rows;
    auto data = load_matrix(filename, data_rows);
    
    //data_m > num_cmps 
    //if(static_cast<uint64_t>(data_rows) > 1){
    //    cout<<"data_rows : "<<data_rows<<" > 1, data_size should be one"<<endl;
    //    //exit(0);
    //}

    this->data_cols = data[0].size();

    return data;

}

// client
void PDTE::setup_cmp(int cmp_type, int l, int m, int extra){
    if (l!=1){
        cout<<"please choose the l to 1, get the best performance."<<endl;
    }
    
    int log_tree_depth = (tree_depth < 1) ? 5 : static_cast<int>(std::ceil(std::log2(tree_depth) + 1));
    extra = log_tree_depth + extra;

    switch (cmp_type) {
        case 1: cmp = make_unique<DCMP>(l, m, extra, true); break;
    }

    lhe = cmp->lhe;


    vector<uint64_t> zero(cmp->num_cmps,0); //change
    auto cmp_raw_encode_zero_b = cmp->raw_encode_b(zero);
    auto cmp_encode_zero_b = cmp->encode_b(cmp_raw_encode_zero_b);
    cmp_zero_b = cmp->encrypt(cmp_encode_zero_b);
}

// client
vector<vector<Ciphertext>> PDTE::encode_data(const vector<vector<uint64_t>>& raw_data){
    auto num_cmps = cmp->num_cmps;

    repeat = num_cmps / data_cols ; // bfv plaintext structure.
    repeat = 1 << static_cast<int>(std::log2(repeat)); // maybe useful, we can test later.

    cout<<"data_cols       : "<< data_cols <<endl;
    cout<<"num_cmps        : "<< num_cmps <<endl;
    cout<<"repeat          : "<< repeat <<endl;

    vector<uint64_t> data(num_cmps, 0);
    for (int i = 0; i < repeat * data_cols; i++){
        data[i] = raw_data[0][ i / repeat];   
    }

    //print_vector(data, data.size(), "data: ");

    auto raw_encode_data = cmp->raw_encode_b(data);
    auto cmp_encode_data = cmp->encode_b(raw_encode_data);
    auto client_input = cmp->encrypt(cmp_encode_data);

    return vector<vector<Ciphertext>>{client_input};
}

// server
// for perfect binary tree with lower depth.
TreeFlatten PDTE::encode_tree(shared_ptr<Node> root){
    auto paths = get_raw_paths(root);
    //print_paths(paths);

    vector<vector<uint64_t>> index_matrix;
    vector<vector<uint64_t>> threshold_matrix;
    vector<vector<uint64_t>> direction_matrix;
    vector<uint64_t> leaf_values;
    extract_matrices_from_paths(paths, index_matrix, threshold_matrix, direction_matrix, leaf_values);

    auto rows = index_matrix.size(); 
    auto cols = index_matrix[0].size();
    auto num_cmps = cmp->num_cmps;

    if (cols >= num_cmps){
        cout<<"cols: "<< cols << " num_cmps: "<< num_cmps <<" the leaves is too much."<<endl;
        exit(0);
    }

    if (cols >= num_cmps / 2){
        rot_flag = true;
    }

    /**/
    cout<< "index_matrix rows    : "<< index_matrix.size() << ", cols: " << index_matrix[0].size() << endl;
    cout<< "threshold_matrix rows: "<< threshold_matrix.size() << ", cols: " << threshold_matrix[0].size() << endl;
    cout<< "leaf_values rows     : "<< leaf_values.size() << endl;
    print_matrix(index_matrix, index_matrix.size(), index_matrix[0].size(), "index_matrix: ");
    print_matrix(threshold_matrix, threshold_matrix[0].size(), threshold_matrix[0].size(), "threshold_matrix: ");
    print_matrix(direction_matrix, direction_matrix[0].size(), direction_matrix[0].size(), "direction_matrix: ");
    print_vector(leaf_values, leaf_values.size(), "leaf_values: ");
    
    // new 
    
    auto aligned_cols = 1 << static_cast<int>(std::ceil(std::log2(cols)));
    //get_optimized_aligned_cols(cols, rows, num_cmps);

    int max_batch_rows = num_cmps / aligned_cols ;  // 1024 / 5 = 
    auto new_rows = (rows + max_batch_rows - 1) / max_batch_rows; 
    auto new_cols = (rows + new_rows - 1) / new_rows * aligned_cols;
    auto cols_nums_per_new_cols = new_cols / aligned_cols;
    auto remiander = (new_rows == 1) ?  cols_nums_per_new_cols : rows / ((new_rows - 1) *  cols_nums_per_new_cols);


    cout << "max_batch_rows: "<< max_batch_rows 
        << ", new_rows: "<< new_rows
        << ", new_cols: "<< new_cols
        << ", aligned_cols: "<< aligned_cols
        << ", cols_nums_per_new_cols: "<< cols_nums_per_new_cols
        << ", remiander: "<< remiander
         <<endl;

    vector<vector<uint64_t>> tiled_index(new_rows, vector<uint64_t>(new_cols, 0));
    vector<vector<uint64_t>> tiled_threshold(new_rows, vector<uint64_t>(new_cols, 0));
    vector<vector<uint64_t>> tiled_direction(new_rows, vector<uint64_t>(new_cols, 0));

    for (int r = 0; r < rows; ++r) {
        int new_r = r / max_batch_rows;       // 目标密文索引
        int offset = (r % max_batch_rows) * aligned_cols; // 目标密文内的起始位置

        for (int c = 0; c < cols; ++c) {
            tiled_index[new_r][offset + c] = index_matrix[r][c];
            tiled_threshold[new_r][offset + c] = threshold_matrix[r][c];
            tiled_direction[new_r][offset + c] = direction_matrix[r][c];
        }
        // 余下的 (aligned_cols - raw_cols) 部分会自动保持为 0 或 1 (占位符)
    }
    index_matrix = std::move(tiled_index);
    threshold_matrix = std::move(tiled_threshold);
    direction_matrix = std::move(tiled_direction);

    cout<< "index_matrix rows    : "<< index_matrix.size() << ", cols: " << index_matrix[0].size() << endl;
    cout<< "threshold_matrix rows: "<< threshold_matrix.size() << ", cols: " << threshold_matrix[0].size() << endl;
    cout<< "leaf_values rows     : "<< leaf_values.size() << endl;
    print_matrix(index_matrix, index_matrix.size(), index_matrix[0].size(), "index_matrix: ");
    print_matrix(threshold_matrix, threshold_matrix[0].size(), threshold_matrix[0].size(), "threshold_matrix: ");
    print_matrix(direction_matrix, direction_matrix[0].size(), direction_matrix[0].size(), "direction_matrix: ");
    print_vector(leaf_values, leaf_values.size(), "leaf_values: ");
    
    auto index_flatten = get_index_flatten(index_matrix, cols, aligned_cols);
    for(size_t i = 0; i < index_flatten.size(); ++i){
        for(size_t j = 0; j < index_flatten[i].size(); ++j){
            cout<<" index: " << index_flatten[i][j].index 
                << " start: "<< index_flatten[i][j].start 
                << " width: " << index_flatten[i][j].width 
                << ", ";
        }
        cout << endl;
    }

    vector<vector<Plaintext>> threshold_pt(new_rows);
    vector<Plaintext> direction_pt(new_rows);
    for (size_t i = 0; i < new_rows; i++) {
        auto raw_encode_a = cmp->raw_encode_a(threshold_matrix[i]);
        threshold_pt[i] = cmp->encode_a(raw_encode_a);
        direction_pt[i] = cmp->init_x_zero_zero(direction_matrix[i]);
    }

    auto leaf_values_pt = cmp->init_x_zero_zero(leaf_values);

    // adapted
    //tree_depth_factorial_inv_pt = init_d_factorial_inv_pt();
    //tree_depth_vec_pt = init_tree_depth_vec();  

    return TreeFlatten{index_flatten, threshold_pt, direction_pt, leaf_values_pt, remiander, new_cols, aligned_cols};
}

vector<vector<Ciphertext>> PDTE::evaluate(shared_ptr<Node> root, vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    auto data = data_cipher[0]; // vector 
    auto data_rotate_columns = lhe->rotate_columns(data);
    /*{
        for(int i=0;i<14;i++){
            auto rot_data = lhe->rotate_rows(data, -512*i);
            auto raw_data = cmp->decode_b(rot_data);
            auto data_vec = cmp->raw_decode_b(raw_data, cmp->num_cmps);
            print_vector(data_vec, cmp->num_cmps, "*data_vec: ");
        }

    }*/

    // flatten
    //auto data_pt = lhe->decrypt(data[0]);
    //auto data_vec = lhe->decode(data_pt);
    //print_vector(data_vec, data_vec.size(), "data_vec: ");

    auto index_flatten = tree_flatten.index_flatten;  // new_rows * new_cols 
    auto threshold_pt = tree_flatten.threshold_pt;
    auto direction_pt = tree_flatten.direction_pt;
    auto leaf_values_pt = tree_flatten.leaf_values_pt;
    auto remiander = tree_flatten.remiander;
    auto new_cols = tree_flatten.new_cols;  // in index_matrix.
    auto aligned_cols = tree_flatten.aligned_cols;

    auto m = cmp->m;
    auto l = cmp->l;
    auto row_count = cmp->row_count;
    auto num_cmps = cmp->num_cmps;

    auto new_rows = index_flatten.size();
    auto cols_nums_per_new_cols = new_cols / aligned_cols;


    cout<< " cols_nums_per_new_cols      : " << cols_nums_per_new_cols
    << "\n remiander                   : "<< remiander
    << "\n aligned_cols                : "<< aligned_cols
    << "\n new_rows                    : "<< new_rows
    << "\n new_cols                    : "<< new_cols
    <<endl;


    cout<<"repeat: "<< repeat <<" m: "<< m <<" repeat * m: "<< repeat * m <<endl;
    vector<vector<Ciphertext>> extract_data_cipher(new_rows);
    for(size_t i = 0; i < new_rows; ++i){
        cout<<" new_rows: " << new_rows << " i: " << i << endl;
        // retrieval
        vector<vector<Ciphertext>> rows_ct; // new_rows * l*m;
        rows_ct.reserve(new_cols);
        vector<Ciphertext> rows_ct_temp;
        for(size_t j = 0; j < index_flatten[i].size(); ++j){
            cout<<" index_flatten[i].size(): " << index_flatten[i].size() << " j: " << j ;
            auto index = index_flatten[i][j].index;
            auto start = index_flatten[i][j].start;
            auto width = index_flatten[i][j].width;

            cout<<" index: " << int(index) 
                << " start: "<< start 
                << " width: " << width 
                << ", ";

            if (index == uint64_t(-1)){
                //cmp_zero
                //onehot
                auto onehot = cmp->get_one_hot(start, width);
                rows_ct_temp = lhe->multiply_plain(cmp_zero_b, onehot);
                rows_ct.push_back(rows_ct_temp);
                cout<<" * ";
            }else{
                int step = index * repeat - start;
                cout << " step: "<< step ;


                // set index nums to start.
                auto rotate_data = cmp->rotate_mmm_rows(data, data_rotate_columns, step); // bug


                if (width <= repeat){
                    // rotate and add
                    // one hot

                }else{

                    // one hot
                    auto onehot = cmp->get_one_hot(start, repeat);
                    lhe->multiply_plain_inplace(rotate_data, onehot);

                    auto rot_times = (width + repeat - 1) / repeat;
                    cout<< " start: " << start << " repeat: " << repeat << " rot_times: " << rot_times;
                    for(int k = 1; k < rot_times; k <<= 1 ){
                        cout<< " k: " << k;
                        //auto temp = rotate_data;
                        //lhe->rotate_rows_inplace(temp, -k * repeat); // 假设向右平移叠加
                        //step = - k * repeat;

                        auto temp = lhe->rotate_rows(rotate_data, -k * repeat * m);

                        lhe->add_inplace(rotate_data, temp);


                    }



                }
                auto onehot = cmp->get_one_hot(start, width);

                /*{
                    auto v = lhe->decode(onehot);
                    print_vector(v, 1024, "onehot: ");
                }*/

                rows_ct_temp = lhe->multiply_plain(rotate_data, onehot);
                rows_ct.push_back(rows_ct_temp);
            }

            cout << endl;

            /*{
                auto raw_data = cmp->decode_b(rows_ct_temp);
                auto data_vec = cmp->raw_decode_b(raw_data, cmp->num_cmps);
                print_vector(data_vec, 30, "rows_ct_temp: ");
            }*/
            

        }

        cout << endl;

        // sum
        auto  rows_ct_sum  = std::move(rows_ct[0]);
        for(size_t j = 1; j < rows_ct.size(); ++j){
            lhe->add_inplace(rows_ct_sum, rows_ct[j]);
        }
        extract_data_cipher[i] = std::move(rows_ct_sum);
    }

    

    /*{

        for(size_t i = 0; i < new_rows; i++){
            auto raw_data = cmp->decode_b(extract_data_cipher[i]);
            auto data_vec = cmp->raw_decode_b(raw_data, cmp->num_cmps);
            print_vector(data_vec, new_cols, "extract_data_cipher: ");

        }
        
    }*/

    vector<Ciphertext> cmp_raw_out(new_rows);
    for (size_t i = 0; i < new_rows; ++i){
        cmp_raw_out[i] = cmp->great_than(threshold_pt[i], extract_data_cipher[i]);
        lhe->sub_plain_inplace(cmp_raw_out[i], direction_pt[i]);
    }


    cout<<"**"<<endl;
    //vector<Ciphertext> cmp_raw_out_rotate_columns;
    //if (rot_flag){
    //cmp_raw_out_rotate_columns = lhe->rotate_columns(cmp_raw_out);
    //}
    cout<<"***"<<endl;
    // rotate
    vector<Ciphertext> cmp_out;
    cmp_out.reserve(20);
    
    for (size_t i = 0; i < cols_nums_per_new_cols; ++i){
        int step = i * aligned_cols;
        auto temp = cmp->rotate_m_rows(cmp_raw_out, step);
        for (size_t j = 0; j < new_rows - 1; ++j){
            cmp_out.push_back(temp[j]);
        }
    }
    cout<<"****"<<endl;

    //reminder    
    for (size_t i =0  ; i < remiander; i++){
        int step = i * aligned_cols;
        auto temp = cmp->rotate_m_rows(cmp_raw_out[new_rows - 1], step);
        cmp_out.push_back(temp);

    }

    cout<< "cmp_out.size(): " << cmp_out.size() <<endl;

    auto out = lhe->multiply_many(cmp_out);



    // pir
    lhe->multiply_plain_inplace(out, leaf_values_pt);

    /*{
        auto temp = cmp->recover(out);
        print_vector(temp, temp.size(), "out");
    }*/

    return {{ std::move(out) }};

}
    


vector<uint64_t> PDTE::recover(vector<vector<Ciphertext>>& a){
    uint64_t out = 0;
    auto temp = cmp->recover(a[0][0]);
    for (int i = 0; i < temp.size();i++){
        if (temp[i]!=0 ){
            out = temp[i];
            if (out >  (lhe->plain_modulus / 2)){
                return vector<uint64_t>{lhe->plain_modulus - out};
            }
        }
        
    }

    return vector<uint64_t>{out};
}

long PDTE::keys_size(){
    return cmp->keys_size();
}

void PDTE::print(){
    cmp->print();
    cout << " name                                     : " << cmp->scheme+"_"+scheme <<endl;
    cout << " tree depth                               : " << tree_depth <<endl;
}

bool PDTE::verify(const vector<uint64_t>& expect_result, const vector<uint64_t>& actural_result){
    //print_vector(expect_result, expect_result.size(),         "bpdte_result   : ");
    //print_vector(actural_result, actural_result.size(), "actural_result: ");
    
    for(size_t i = 0; i < actural_result.size(); ++i){
        if (actural_result[i]!=expect_result[i]){
            return false;
        }
    }
    return true;
}

long PDTE::comm_cost(const vector<vector<Ciphertext>>& ct1, const vector<vector<Ciphertext>>& ct2) {
    std::stringstream data_stream;
    long comm = 0;
    for(const auto& cte : ct1){
        for(const auto& e : cte){
            comm += e.save(data_stream);
        }
    } 
    for(const auto& cte : ct2){
        for(const auto& e : cte){
            comm += e.save(data_stream);
        }
    } 
    return comm;
}


/**
 * 根据路径数(cols)和树深度(rows)计算最优的对齐列宽
 * 目标：在保证旋转性能的前提下，最小化 new_rows (密文总数)
 * * @param cols 原始路径数 (叶子数)
 * @param rows 决策树的原始层数 (深度)
 * @param num_cmps SEAL 密文的 Slot 总数 (如 1024, 4096, 8192)
 * @return 优化后的 aligned_cols
 */
uint64_t PDTE::get_optimized_aligned_cols(uint64_t cols, uint64_t rows, uint64_t num_cmps) {

    uint64_t pure = 1;
    while (pure < cols) pure <<= 1;

    if (pure == cols) return pure;
    uint64_t x = pure >> 1;  // the nearst power small than cols.

    uint64_t y = 1;
    while (x + y < cols) y <<= 1;
    uint64_t sum = x + y; // the near x + y >= cols.
    if (sum < pure) {
        uint64_t rows_pure = (rows + (num_cmps / pure) - 1) / (num_cmps / pure);
        uint64_t rows_sum  = (rows + (num_cmps / sum) - 1) / (num_cmps / sum);
        if (rows_sum < rows_pure) return sum;
    }

    return pure;
}

vector<vector<IndexPos>> PDTE::get_index_flatten(vector<vector<uint64_t>> index_matrix, uint64_t cols,  uint64_t aligned_cols){
    auto new_rows = index_matrix.size();
    auto new_cols = index_matrix[0].size();
    vector<vector<IndexPos>> index_flatten(new_rows);
    vector<IndexPos> temp;
    temp.reserve(cols);
    uint64_t index, start, end, width;
    for(size_t i = 0; i < new_rows; ++i){
        index = index_matrix[i][0];
        start = 0;
        end = 0;
        width = 0;
        for(size_t j = 0; j < new_cols; ++j){
            if (j % aligned_cols >= cols){
                continue;
            }
            if (index != index_matrix[i][j]){
                width = end - start + 1;
                //cout<<" index: " << index << " start: "<< start << " width: " << width <<endl;
                temp.push_back(IndexPos{index, start, width});
                index = index_matrix[i][j];
                start = j;
            }
            end = j;
        }
        width = end - start + 1;
        //cout<<" index: " << index << " start: "<< start << " width: " << width <<endl;
        temp.push_back(IndexPos{index, start, width});

        index_flatten[i] = temp;
    }
    return index_flatten;
}

