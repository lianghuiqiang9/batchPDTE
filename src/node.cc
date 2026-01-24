#include"node.h"

Node::Node(string filename){
    ifstream file(filename);
    nlohmann::json ref = nlohmann::json::parse(file);
    load_tree_rec(ref, *this);
};

void Node::save_tree(string filename){
    auto tree = save_tree_rec(*this);
    ofstream file(filename);
    file << tree << endl;  
}

void Node::print_tree() {
    print_tree_rec(this, "");
}

bool Node::is_leaf() const {
    return this->left == nullptr && this->right == nullptr;
}

uint64_t Node::get_depth() {
    if (this->is_leaf()) {
        return 0;
    } else {
        auto l = this->left->get_depth();
        auto r = this->right->get_depth();
        if (l > r) {
            return l + 1;
        } else {
            return r + 1;
        }
    }   
}

vector<uint64_t> Node::leaf_extract() {
    vector<uint64_t> out;
    stack<StackFrame> stk;
    stk.push({this});

    while (!stk.empty()) {
        StackFrame frame = stk.top();
        stk.pop();

        Node* node = frame.node;

        if (node->is_leaf()) {
            out.push_back(node->leaf_value);
        } else {

            if (node->right) stk.push({node->right.get()}); 
            if (node->left) stk.push({node->left.get()});
        }
    }
    return out;
}

uint64_t Node::eval(const vector<uint64_t> &features) {
    uint64_t out = 0;
    uint64_t parent = 1;
    eval_rec(out, *this, features, parent);
    return out;
}

vector<uint64_t> Node::eval(const vector<vector<uint64_t>> &features) {
    vector<uint64_t> out(features.size());
    for (size_t i = 0; i< features.size(); ++i){
        out[i] = eval(features[i]);
    }
    return out;
}



vector<shared_ptr<Node>> get_standard_level(shared_ptr<Node> root, int target_depth) {
    if (!root) return vector<shared_ptr<Node>>(1 << target_depth, nullptr);

    vector<shared_ptr<Node>> current_level = { root };

    for (int d = 0; d < target_depth; ++d) {
        vector<shared_ptr<Node>> next_level;
        next_level.reserve(current_level.size() * 2);

        for (auto& node : current_level) {
            if (node && !node->is_leaf()) {
                // 正常中间节点，走左右孩子
                next_level.push_back(node->left);
                next_level.push_back(node->right);
            } else if (node && node->is_leaf()) {
                // 【关键修改】：如果当前已经是叶子，为了补全，
                // 让它的“孩子”依然指向它自己
                next_level.push_back(node);
                next_level.push_back(node);
            } else {
                // 只有真正的空位才填 NULL
                next_level.push_back(nullptr);
                next_level.push_back(nullptr);
            }
        }
        current_level = std::move(next_level);
    }
    return current_level;
}

// 主函数：抽取对齐后的层级矩阵
vector<vector<shared_ptr<Node>>> extract_aligned_layers(shared_ptr<Node> root) {
    if (!root) return {};

    // 获取当前节点（树根）的总深度
    int total_depth = root->get_depth() + 1;
    if (total_depth == 0) return {};

    // 计算最后一层的理论最大宽度 W = 2^(depth-1)
    // 使用位移运算 1 << n 代替 pow(2, n) 更精确、高效
    int max_width = 1 << (total_depth - 1); 
    vector<vector<shared_ptr<Node>>> result(total_depth);

    // 1. 逐层抽取
    for (int d = 0; d < total_depth; ++d) {
        int nodes_in_this_level = 1 << d;
        int repeat_factor = max_width / nodes_in_this_level;
        
        // 获取该层标准节点序列
        vector<shared_ptr<Node>> standard_level = get_standard_level(root, d);

        // 2. 按照对齐要求进行扩展或复制
        result[d].reserve(max_width);
        for (auto& node : standard_level) {
            for (int r = 0; r < repeat_factor; ++r) {
                result[d].push_back(node);
            }
        }
    }
    return result;
}

void print_aligned_matrix(const vector<vector<shared_ptr<Node>>>& matrix) {
    for (size_t d = 0; d < matrix.size(); ++d) {
        cout << "--- Layer " << d << " ---" << endl;
        for (size_t w = 0; w < matrix[d].size(); ++w) {
            cout << "[" << w << "]: ";
            if (matrix[d][w]) {
                // 直接调用你现有的 print_node
                print_node(*matrix[d][w]); 
            } else {
                cout << "NULL" << endl;
            }
        }
        cout << endl;
    }
}

/**
 * 从对齐的节点矩阵中提取索引矩阵和阈值矩阵
 * @param aligned_layers 由 extract_aligned_layers 生成的矩阵
 * @param index_matrix 输出：存储 node->index
 * @param threshold_matrix 输出：存储 node->threshold
 */
void extract_matrices(const vector<vector<shared_ptr<Node>>>& aligned_layers,
                      vector<vector<uint64_t>>& index_matrix,
                      vector<vector<uint64_t>>& threshold_matrix,
                      vector<uint64_t>& class_vec) {
    
    index_matrix.clear();
    threshold_matrix.clear();
    class_vec.clear();
    
    if (aligned_layers.empty()) return;

    int depth = aligned_layers.size();
    int width = aligned_layers[0].size();

    // 1. 预留决策层空间 (不包含最后一层叶子层)
    index_matrix.resize(depth - 1);
    threshold_matrix.resize(depth - 1);

    for (int d = 0; d < depth - 1; ++d) {
        index_matrix[d].reserve(width);
        threshold_matrix[d].reserve(width);

        for (int w = 0; w < width; ++w) {
            auto node = aligned_layers[d][w];
            // 非叶子节点：提取 index 和 threshold
            if (node && !node->is_leaf()) {
                index_matrix[d].push_back(static_cast<uint64_t>(node->index));
                threshold_matrix[d].push_back(node->threshold);
            } else {
                // 如果是提前出现的叶子或空位，填充 0 占位
                index_matrix[d].push_back(0);
                threshold_matrix[d].push_back(0);
            }
        }
    }

    // 2. 提取最后一层作为 class_vec (利用叶子下沉逻辑)
    class_vec.reserve(width);
    const auto& last_layer = aligned_layers.back(); 
    for (int w = 0; w < width; ++w) {
        if (last_layer[w]) {
            class_vec.push_back(last_layer[w]->leaf_value);
        } else {
            class_vec.push_back(0); // 兜底 NULL 情况
        }
    }
}
void print_node(Node& a, string tabs){
    if (a.is_leaf()){
       cout << tabs << "(class: " << a.leaf_value << ")" << endl; 
    }else{
        cout << tabs
            << "(f: " << a.index 
            << ", t: " << a.threshold
            << ")" << endl; 
    }
}

void print_tree_rec(Node* a, string tabs) {
    if (a == nullptr) return;
    if (a->is_leaf()) {
    print_node(*a, tabs);
    } else {
        print_tree_rec(a->right.get(), tabs + "    ");
        print_node(*a, tabs);
        print_tree_rec(a->left.get(), tabs + "    ");
    }
}

void load_tree_rec(json::reference ref, Node& node) {
    if (!ref["leaf"].is_null()) {
        node.threshold = 0;
        node.index = 0;
        node.leaf_value = ref["leaf"].get<uint64_t>();
        node.left = nullptr;
        node.right = nullptr;
    } else {
        node.threshold = ref["internal"]["threshold"].get<uint64_t>();
        node.index = ref["internal"]["feature"].get<unsigned>();
        node.op = ref["internal"]["op"].get<string>();
        node.leaf_value = -1;

        node.left = make_shared<Node>();
        node.right = make_shared<Node>();
        load_tree_rec(ref["internal"]["left"], *node.left);
        load_tree_rec(ref["internal"]["right"], *node.right);
    }
}

json save_tree_rec(Node& node){
    json j;
    if(node.left == nullptr && node.right == nullptr){
        j["leaf"] = node.leaf_value;
    }else{
        j["internal"]["threshold"] = node.threshold;
        j["internal"]["feature"] = node.index;
        j["internal"]["op"] = node.op;
        j["internal"]["left"] = save_tree_rec(*node.left);
        j["internal"]["right"] = save_tree_rec(*node.right);
    }
    return j;
}

void eval_rec(uint64_t &out, const Node& node, const vector<uint64_t> &features, u_int64_t parent) {
    if (node.is_leaf()) {
        //cout<<"node.class_leaf = " <<node.class_leaf<< " parent = "<<parent<<endl;
        out += node.leaf_value * parent;
    }else{
        if (node.op == "leq") {
            //pearent     1           *  parent       0
            //left      0   1  right  *  left       1   0     right
            //cout<<"features[" <<node.feature_index<< "] = "<<features[node.feature_index] <<" > "<< node.threshold <<"  --- "<<(features[node.feature_index] > node.threshold)<<endl;
            if (features[node.index] >= node.threshold) {
                eval_rec(out, *node.left, features, parent*(1-parent));
                eval_rec(out, *node.right, features, parent);
                
            } else {
                eval_rec(out, *node.left, features, parent);
                eval_rec(out, *node.right, features, parent*(1-parent));
            }
        } else {
            // unimplemented
            assert(false);
        }
    }
}

vector<vector<uint64_t>> load_matrix(string filename, int data_size){
    vector<vector<uint64_t>> data;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "open files error" << endl;
        return data;
    }
    string line;

    // read csv line by line
    int i=0;
    while(i<data_size && getline(file, line)){
        vector<uint64_t> row;
        stringstream lineStream(line);
        string cell;
        while (getline(lineStream, cell, ',')) {
            uint64_t cellValue;
            istringstream(cell) >> cellValue;
            row.push_back(cellValue);
        }

        data.push_back(row);
        i++;
    }
    return data;
}

void save_data(const vector<vector<uint64_t>>& data, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file " << filename << endl;
        cout<<"Need mkdir the output file first "<<endl;
        exit(0);
        return;
    }
    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
}
