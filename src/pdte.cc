
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
    
    this->data_cols = data[0].size();

    return data;

}

// client
vector<vector<Ciphertext>> PDTE::encode_data(const vector<vector<uint64_t>>& raw_data){
    auto num_cmps = cmp->num_cmps;

    repeat = num_cmps / 2 / data_cols ; 
    repeat = 1 << static_cast<uint64_t>(std::log2(repeat)); 

    //cout<<"data_cols       : "<< data_cols <<endl;
    //cout<<"num_cmps        : "<< num_cmps <<endl;
    //cout<<"repeat          : "<< repeat <<endl;
    
    vector<uint64_t> data(num_cmps, 0);
    auto half_num_cmps = (num_cmps + 1) / 2;
    for (uint64_t i = 0; i < repeat * data_cols; i++){
        data[i] = raw_data[0][ i / repeat];
        data[half_num_cmps + i] = data[i];
    }

    //print_vector(data, data.size(), "data: ");

    auto raw_encode_data = cmp->raw_encode_b(data);
    auto cmp_encode_data = cmp->encode_b(raw_encode_data);
    auto client_input = cmp->encrypt(cmp_encode_data);

    return vector<vector<Ciphertext>>{client_input};
}

/*
// client
vector<vector<Ciphertext>> PDTE::old_encode_data(const vector<vector<uint64_t>>& raw_data){
    auto num_cmps = cmp->num_cmps;

    repeat = num_cmps / data_cols ; 
    repeat = 1 << static_cast<int>(std::log2(repeat)); 

    //cout<<"data_cols       : "<< data_cols <<endl;
    //cout<<"num_cmps        : "<< num_cmps <<endl;
    //cout<<"repeat          : "<< repeat <<endl;
    
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
*/

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

    this->leaf_nums = cols;

    if (cols >= num_cmps){
        cout<<"cols: "<< cols << " num_cmps: "<< num_cmps <<" the leaves is too much, reduce the m."<<endl;
        exit(0);
    }

    /*
    cout<< "index_matrix rows    : "<< index_matrix.size() << ", cols: " << index_matrix[0].size() << endl;
    cout<< "threshold_matrix rows: "<< threshold_matrix.size() << ", cols: " << threshold_matrix[0].size() << endl;
    cout<< "leaf_values rows     : "<< leaf_values.size() << endl;
    print_matrix(index_matrix, index_matrix.size(), index_matrix[0].size(), "index_matrix: ");
    print_matrix(threshold_matrix, threshold_matrix[0].size(), threshold_matrix[0].size(), "threshold_matrix: ");
    print_matrix(direction_matrix, direction_matrix[0].size(), direction_matrix[0].size(), "direction_matrix: ");
    print_vector(leaf_values, leaf_values.size(), "leaf_values: ");
    */

    int aligned_cols = 1 << static_cast<int>(std::ceil(std::log2(cols)));

    int max_batch_rows = num_cmps / aligned_cols ; 
    int new_rows = (rows + max_batch_rows - 1) / max_batch_rows; 
    int new_cols = (rows + new_rows - 1) / new_rows * aligned_cols;
    int cols_nums_per_new_cols = new_cols / aligned_cols;
    int remainder = (new_rows == 1) ?  cols_nums_per_new_cols : rows / ((new_rows - 1) *  cols_nums_per_new_cols);

    /*
    cout << "max_batch_rows: "<< max_batch_rows 
        << ", new_rows: "<< new_rows
        << ", new_cols: "<< new_cols
        << ", aligned_cols: "<< aligned_cols
        << ", cols_nums_per_new_cols: "<< cols_nums_per_new_cols
        << ", remainder: "<< remainder
         <<endl;
    */

    vector<vector<uint64_t>> tiled_index(new_rows, vector<uint64_t>(new_cols, 0));
    vector<vector<uint64_t>> tiled_threshold(new_rows, vector<uint64_t>(new_cols, 0));
    vector<vector<uint64_t>> tiled_direction(new_rows, vector<uint64_t>(new_cols, 0));

    for (size_t r = 0; r < rows; ++r) {
        int new_r = r / max_batch_rows;      
        int offset = (r % max_batch_rows) * aligned_cols; 
        for (size_t c = 0; c < cols; ++c) {
            tiled_index[new_r][offset + c] = index_matrix[r][c];
            tiled_threshold[new_r][offset + c] = threshold_matrix[r][c];
            tiled_direction[new_r][offset + c] = direction_matrix[r][c];
        }
    }

    index_matrix = std::move(tiled_index);
    threshold_matrix = std::move(tiled_threshold);
    direction_matrix = std::move(tiled_direction);
    auto index_flatten = get_index_flatten(index_matrix, cols, aligned_cols);

    /*
    cout<< "index_matrix rows    : "<< index_matrix.size() << ", cols: " << index_matrix[0].size() << endl;
    cout<< "threshold_matrix rows: "<< threshold_matrix.size() << ", cols: " << threshold_matrix[0].size() << endl;
    cout<< "leaf_values rows     : "<< leaf_values.size() << endl;
    print_matrix(index_matrix, index_matrix.size(), index_matrix[0].size(), "index_matrix: ");
    print_matrix(threshold_matrix, threshold_matrix[0].size(), threshold_matrix[0].size(), "threshold_matrix: ");
    print_matrix(direction_matrix, direction_matrix[0].size(), direction_matrix[0].size(), "direction_matrix: ");
    print_vector(leaf_values, leaf_values.size(), "leaf_values: ");
    
    for(size_t i = 0; i < index_flatten.size(); ++i){
        for(size_t j = 0; j < index_flatten[i].size(); ++j){
            cout<<" index: " << index_flatten[i][j].index 
                << " start: "<< index_flatten[i][j].start 
                << " width: " << index_flatten[i][j].width 
                << ", ";
        }
        cout << endl;
    }
    */

    vector<vector<Plaintext>> threshold_pt(new_rows);
    vector<Plaintext> direction_pt(new_rows);
    for (int i = 0; i < new_rows; i++) {
        auto raw_encode_a = cmp->raw_encode_a(threshold_matrix[i]);
        threshold_pt[i] = cmp->encode_a(raw_encode_a);
        direction_pt[i] = cmp->init_x_zero_zero(direction_matrix[i]);
    }
    auto leaf_values_pt = cmp->init_x_zero_zero(leaf_values);

    return TreeFlatten{index_flatten, threshold_pt, direction_pt, leaf_values_pt, remainder, new_cols, aligned_cols};
}


vector<vector<IndexPos>> PDTE::get_index_flatten(vector<vector<uint64_t>> index_matrix, uint64_t cols, uint64_t aligned_cols) {
    auto new_rows = index_matrix.size();
    auto new_cols = index_matrix[0].size();
    vector<vector<IndexPos>> index_flatten(new_rows);

    for (size_t i = 0; i < new_rows; ++i) {
        vector<IndexPos> row_temp; 
        row_temp.reserve(cols); 

        uint64_t current_index = 0;
        uint64_t start = 0;

        for (size_t j = 0; j < new_cols; ++j) {
            auto flag = j % aligned_cols;

            if (flag == 0) {
                current_index = index_matrix[i][j];
                start = j;
            } 
            else if (flag < cols) {
                if (current_index != index_matrix[i][j]) {
                    row_temp.push_back(IndexPos{current_index, start, j - start});
                    current_index = index_matrix[i][j];
                    start = j;
                }
            }

            if (flag == cols - 1) {
                row_temp.push_back(IndexPos{current_index, start, j - start + 1});
            }
        }
        index_flatten[i] = std::move(row_temp); 
    }
    return index_flatten;
}


vector<vector<Ciphertext>> PDTE::feature_extract(vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    auto& data = data_cipher[0]; // vector 
    
    const auto& index_flatten = tree_flatten.index_flatten;  // new_rows * new_cols 
    const uint64_t new_rows = index_flatten.size();
    uint64_t new_cols = tree_flatten.new_cols;
    uint64_t num_cmps_per_row = cmp->num_cmps_per_row;
    uint64_t feature_width = new_cols > num_cmps_per_row ? num_cmps_per_row : new_cols;

    vector<vector<Ciphertext>> encode_data(data_cols);
    vector<Ciphertext> data_rot;
    for(uint64_t i = 0; i < data_cols; i++){

        if(feature_width > repeat){
            data_rot = cmp->fill_double_width_hot(data, i * repeat, feature_width, repeat);
        }

        if (feature_width < num_cmps_per_row){
            data_rot = cmp->exchange(data_rot, 0, i * repeat);
        }

        encode_data[i] = std::move(data_rot);
    }

    vector<vector<Ciphertext>> extract_data(new_rows);
    vector<Ciphertext> temp;
    Plaintext onehot;
    for(size_t i = 0; i < new_rows; ++i){
        //cout<<" new_rows: " << new_rows << " i: " << i << endl;
        // retrieval
        for(size_t j = 0; j < index_flatten[i].size(); ++j){
            auto& index = index_flatten[i][j].index;
            auto& start = index_flatten[i][j].start;
            auto& width = index_flatten[i][j].width;
            //cout<<" index_flatten["<<i<<"].size(): " << index_flatten[i].size() << " j: " << j ;
            //cout<<" index: " << int(index) << " start: "<< start << " width: " << width << endl;;

            onehot = cmp->get_one_hot(start, width);

            if (index == uint64_t(-1)){
                temp = lhe->multiply_plain(cmp_zero_b, onehot);
            }else{
                temp =lhe->multiply_plain(encode_data[index], onehot);
            }

            // sum
            if (j==0){
                extract_data[i]  = std::move(temp);
            }else{
                lhe->add_inplace(extract_data[i], temp);
            }
        }
    }

    return extract_data;
}

/*
vector<vector<Ciphertext>> PDTE::old_feature_extract(vector<vector<Ciphertext>>& data_cipher, TreeFlatten& tree_flatten){
    auto& data = data_cipher[0]; // vector 
    auto data_rot_columns = lhe->rotate_columns(data);

    const auto& index_flatten = tree_flatten.index_flatten;  // new_rows * new_cols 
    const uint64_t new_rows = index_flatten.size();

    vector<vector<Ciphertext>> extract_data(new_rows);
    vector<Ciphertext> temp;
    Plaintext onehot;
    for(size_t i = 0; i < new_rows; ++i){
        //cout<<" new_rows: " << new_rows << " i: " << i << endl;
        // retrieval
        for(size_t j = 0; j < index_flatten[i].size(); ++j){
            auto& index = index_flatten[i][j].index;
            auto& start = index_flatten[i][j].start;
            auto& width = index_flatten[i][j].width;
            //cout<<" index_flatten["<<i<<"].size(): " << index_flatten[i].size() << " j: " << j ;
            //cout<<" index: " << int(index) << " start: "<< start << " width: " << width << endl;;

            onehot = cmp->get_one_hot(start, width);
            if (index == uint64_t(-1)){
                temp = lhe->multiply_plain(cmp_zero_b, onehot);
            }else{
                temp = cmp->exchange(data, data_rot_columns, start, index * repeat);
                cmp->fill_width_hot(temp, start, width, repeat);
                lhe->multiply_plain_inplace(temp, onehot);
            }

            // sum
            if (j==0){
                extract_data[i]  = std::move(temp);
            }else{
                lhe->add_inplace(extract_data[i], temp);
            }
        }
    }

    return extract_data;
}
*/

vector<Ciphertext> PDTE::expend_compare_result(vector<Ciphertext>& cmp_raw_out, TreeFlatten& tree_flatten) {
    uint64_t aligned_cols = tree_flatten.aligned_cols;
    uint64_t new_cols = tree_flatten.new_cols;
    uint64_t remainder = tree_flatten.remainder;
    
    const size_t new_rows = cmp_raw_out.size();
    if (new_rows == 0) return {};

    const size_t cols_nums_per_new_cols = new_cols / aligned_cols;
    
    vector<Ciphertext> cmp_out;
    size_t total_expected = (cols_nums_per_new_cols * (new_rows - 1)) + remainder;
    cmp_out.reserve(total_expected);

    for (size_t i = 0; i < cols_nums_per_new_cols; ++i) {
        auto temp = cmp->exchange(cmp_raw_out, 0, i * aligned_cols);
        for (size_t j = 0; j < new_rows - 1; ++j) {
            cmp_out.push_back(std::move(temp[j]));
        }
    }

    if (remainder > 0) {
        const auto& last_row = cmp_raw_out[new_rows - 1]; 
        for (size_t i = 0; i < remainder; i++) {
            auto temp = cmp->exchange(last_row, 0, i * aligned_cols);
            cmp_out.push_back(std::move(temp));
        }
    }

    return cmp_out; 
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
