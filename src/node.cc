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

// t > f[a] --> left
uint64_t eval_rec(const Node& node, const vector<uint64_t>& features) {
    if (node.is_leaf()) {
        return node.leaf_value;
    }
    /*
    cout<<"node.threshold: " << node.threshold 
        << " index: " << node.index 
        << " features[node.index]: " << features[node.index] 
        << " direction: " << ((node.threshold > features[node.index]) ? " left" : "right")
        <<endl;
    */
    const Node* next_node = (node.threshold > features[node.index]) 
                            ? node.left.get()
                            : node.right.get();

    return eval_rec(*next_node, features);
}

uint64_t Node::eval(const vector<uint64_t> &features) {
    return eval_rec(*this, features);

}

vector<uint64_t> Node::eval(const vector<vector<uint64_t>> &features) {
    vector<uint64_t> out(features.size());
    for (size_t i = 0; i< features.size(); ++i){
        out[i] = eval(features[i]);
    }
    return out;
}

// t > f[a] --> right
uint64_t eval_rec2(const Node& node, const vector<uint64_t>& features) {
    if (node.is_leaf()) {
        return node.leaf_value;
    }
    const Node* next_node = (node.threshold > features[node.index]) 
                            ? node.right.get() 
                            : node.left.get();

    return eval_rec2(*next_node, features);
}

uint64_t Node::eval2(const vector<uint64_t> &features) {
    return eval_rec2(*this, features); 
}

vector<uint64_t> Node::eval2(const vector<vector<uint64_t>> &features) {
    vector<uint64_t> out(features.size());
    for (size_t i = 0; i< features.size(); ++i){
        out[i] = eval2(features[i]);
    }
    return out;
}

void find_all_paths(shared_ptr<Node> node, vector<shared_ptr<Node>>& current_path, vector<Path>& all_paths) {
    if (!node) return;
    current_path.push_back(node);

    if (node->is_leaf()) {
        all_paths.push_back({ current_path });
    } else {
        if (node->left) find_all_paths(node->left, current_path, all_paths);
        if (node->right) find_all_paths(node->right, current_path, all_paths);
    }

    current_path.pop_back();
}

vector<Path> get_raw_paths(shared_ptr<Node> root) {
    vector<Path> all_paths;
    vector<shared_ptr<Node>> current_path;
    find_all_paths(root, current_path, all_paths);
    return all_paths;
}

void print_paths(const vector<Path>& paths) {
    cout << "--- Tree Paths (Total Leaf Paths: " << paths.size() << ") ---" << endl;

    for (size_t i = 0; i < paths.size(); ++i) {
        cout << "Path " << i << ": ";
        const auto& path_nodes = paths[i].nodes;

        for (size_t j = 0; j < path_nodes.size(); ++j) {
            auto node = path_nodes[j];
            
            if (node->is_leaf()) {
                cout << "[Leaf Label: " << node->leaf_value << "]";
            } else {
                cout << "(F" << node->index << " < " << node->threshold << ")";

                if (j + 1 < path_nodes.size()) {
                    auto next_node = path_nodes[j + 1];
                    if (next_node == node->left) {
                        cout << " --L--> ";
                    } else if (next_node == node->right) {
                        cout << " --R--> ";
                    }
                }
            }
        }
        cout << endl;
    }
    cout << "------------------------------------------" << endl;
}

void extract_matrices_from_paths(const vector<Path>& paths, 
                                 vector<vector<uint64_t>>& index_matrix,
                                 vector<vector<uint64_t>>& threshold_matrix,
                                 vector<vector<uint64_t>>& direction_matrix,
                                 vector<uint64_t>& leaf_values) {
    if (paths.empty()) return;

    size_t max_nodes = 0;
    for (const auto& p : paths) {
        max_nodes = max(max_nodes, p.nodes.size());
    }

    int total_depth = static_cast<int>(max_nodes);
    int decision_layers = total_depth - 1; 
    size_t num_paths = paths.size();
    
    index_matrix.assign(decision_layers, vector<uint64_t>(num_paths, 0));
    threshold_matrix.assign(decision_layers, vector<uint64_t>(num_paths, 0));
    direction_matrix.assign(decision_layers, vector<uint64_t>(num_paths, 0));
    leaf_values.assign(num_paths, 0);

    for (size_t col = 0; col < num_paths; ++col) {
        const auto& nodes = paths[col].nodes;
        
        for (size_t row = 0; row < nodes.size(); ++row) {
            auto node = nodes[row];

            if (node->is_leaf()) {
                leaf_values[col] = node->leaf_value;
                
                for (int r = row; r < decision_layers; ++r) {
                    index_matrix[r][col] = static_cast<uint64_t>(-1); 
                    threshold_matrix[r][col] = 1;                   
                    direction_matrix[r][col] = 0;                  
                }
                break; 
            } else {
                index_matrix[row][col] = static_cast<uint64_t>(node->index);
                threshold_matrix[row][col] = node->threshold;

                if (row + 1 < nodes.size()) {
                    auto next_node = nodes[row + 1];
                    if (next_node == node->left) {
                        direction_matrix[row][col] = 0; 
                    } else {
                        direction_matrix[row][col] = 1;
                    }
                }
            }
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
