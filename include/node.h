#ifndef __NODE__
#define __NODE__

#include<seal/seal.h>
#include <stack>
#include<fstream>

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

};

////
class Node;
struct StackFrame {
    Node* node;
    bool visited; 
};

void load_tree_rec(json::reference ref, Node& node);
json save_tree_rec(Node& node);
void print_node(Node& a, std::string tabs = "");
void print_tree_rec(Node* a, string tabs);
void eval_rec(uint64_t &out, const Node& node, const std::vector<uint64_t> &features, u_int64_t parent);

vector<vector<uint64_t>> load_matrix(string filename, int data_size);
void save_data(const vector<vector<uint64_t>>& data, const string& filename);

vector<shared_ptr<Node>> get_standard_level(shared_ptr<Node> root, int target_depth);
vector<vector<shared_ptr<Node>>> extract_aligned_layers(shared_ptr<Node> root);
void print_aligned_matrix(const vector<vector<shared_ptr<Node>>>& matrix);
void extract_matrices(const vector<vector<shared_ptr<Node>>>& aligned_layers,
                      vector<vector<uint64_t>>& index_matrix,
                      vector<vector<uint64_t>>& threshold_matrix,
                      vector<uint64_t>& class_vec);
////

#endif