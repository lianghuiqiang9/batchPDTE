#ifndef __NODE__
#define __NODE__

#include<seal/seal.h>
#include <stack>
#include<fstream>
#include <set>
#include "json.hpp"

using namespace seal;
using json = nlohmann::json;
using namespace std;

class Node{
public:
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    int index;
    
    uint64_t threshold;
    vector<vector<uint64_t>> raw_encode_threshold;
    vector<Plaintext> cmp_encode_threshold;

    uint64_t leaf_value; 
    
    std::string op;
    Ciphertext value;

    Node() = default;

    explicit Node(std::string filename);

    void save_tree(std::string filename);

    void print_tree();

    bool is_leaf() const;

    uint64_t get_depth();

    vector<uint64_t> leaf_extract();

    uint64_t eval(const vector<uint64_t> &features);

    vector<uint64_t> eval(const vector<vector<uint64_t>> &features);

    uint64_t eval2(const vector<uint64_t> &features);

    vector<uint64_t> eval2(const vector<vector<uint64_t>> &features);

};

////
class Node;
struct StackFrame {
    Node* node;
    bool visited; 
};

struct Path {
    vector<shared_ptr<Node>> nodes;
};

void load_tree_rec(json::reference ref, Node& node);
json save_tree_rec(Node& node);
void print_node(Node& a, std::string tabs = "");
void print_tree_rec(Node* a, string tabs);
uint64_t eval_rec(const Node& node, const vector<uint64_t>& features);
uint64_t eval_rec2(const Node& node, const vector<uint64_t>& features);
vector<vector<uint64_t>> load_matrix(string filename, int data_size);
void save_data(const vector<vector<uint64_t>>& data, const string& filename);

void find_all_paths(shared_ptr<Node> node, vector<shared_ptr<Node>>& current_path, vector<Path>& all_paths);
vector<Path> get_raw_paths(shared_ptr<Node> root);
void print_paths(const vector<Path>& paths);
void extract_matrices_from_paths(const vector<Path>& paths, 
                                 vector<vector<uint64_t>>& index_matrix,
                                 vector<vector<uint64_t>>& threshold_matrix,
                                 vector<vector<uint64_t>>& direction_matrix,
                                 vector<uint64_t>& leaf_values);
////

#endif