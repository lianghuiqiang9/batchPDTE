#pragma once

#include<seal/seal.h>
#include <stack>
#include<fstream>

#include "json.hpp"

using namespace seal;
using json = nlohmann::json;
using namespace std;

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
////

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

    explicit Node(std::string filename){
        std::ifstream file(filename);
        nlohmann::json ref = nlohmann::json::parse(file);
        load_tree_rec(ref, *this);
    };

    void save_tree(std::string filename){
        auto tree = save_tree_rec(*this);
        std::ofstream file(filename);
        file << tree << std::endl;  
    }

    void print_tree() {
        print_tree_rec(this, "");
    }

    bool is_leaf() const {
        return this->left == nullptr && this->right == nullptr;
    }

    uint64_t get_depth() {
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

    vector<uint64_t> leaf_extract() {
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

    uint64_t eval(const vector<uint64_t> &features) {
        uint64_t out = 0;
        uint64_t parent = 1;
        eval_rec(out, *this, features, parent);
        return out;
    }

    vector<uint64_t> eval(const vector<vector<uint64_t>> &features) {
        vector<uint64_t> out(features.size());
        for (int i = 0; i< features.size(); ++i){
            out[i] = eval(features[i]);
        }
        return out;
    }

};


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
        node.leaf_value = ref["leaf"].get<uint64_t>();
        node.left = nullptr;
        node.right = nullptr;
    } else {
        node.threshold = ref["internal"]["threshold"].get<uint64_t>();
        node.index = ref["internal"]["feature"].get<unsigned>();
        node.op = ref["internal"]["op"].get<std::string>();
        node.leaf_value = -1;

        node.left = std::make_shared<Node>();
        node.right = std::make_shared<Node>();
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

void eval_rec(uint64_t &out, const Node& node, const std::vector<uint64_t> &features, u_int64_t parent) {
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




