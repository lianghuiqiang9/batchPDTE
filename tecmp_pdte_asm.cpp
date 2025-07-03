#include <iostream>
#include <fstream>
#include <cassert>
#include <iomanip>
#include "node.h"
#include "utils.h"
#include "cmp.h"
#include "pdte.h"
#include<seal/seal.h>
#include <stack>

using namespace std;
using namespace seal;

void pdte_tecmp_rec(vector<Ciphertext>& out,Node& node, Evaluator *evaluator,GaloisKeys* gal_keys_server, RelinKeys *rlk_server, vector<vector<Ciphertext>> client_input, Plaintext one,int l,int m,uint64_t m_degree, BatchEncoder *batch_encoder,int slot_count, int plain_modulus, seal::Ciphertext one_zero_init_cipher){
    if (node.is_leaf()){
        out.push_back(node.value);
    }else{
        //pearent     0           *  parent       1
        //left      0   1  right  *  left       1   0     right
        //note that the index of client_data in the tree starts from 0
        //cmp
        node.right->value = tecmp(evaluator, gal_keys_server,rlk_server, node.threshold_bitv, client_input[ node.feature_index ],l, m, m_degree, one_zero_init_cipher);
        evaluator->negate(node.right->value, node.left->value);
        evaluator->add_plain_inplace(node.left->value, one);
        //travel
        evaluator->add_inplace(node.left->value, node.value);
        evaluator->add_inplace(node.right->value, node.value);
        pdte_tecmp_rec(out, *(node.left), evaluator,gal_keys_server, rlk_server, client_input,one,l,m,m_degree,  batch_encoder,slot_count, plain_modulus,one_zero_init_cipher);
        pdte_tecmp_rec(out, *(node.right), evaluator,gal_keys_server, rlk_server, client_input,one,l,m,m_degree,  batch_encoder,slot_count, plain_modulus,one_zero_init_cipher);
    }
}


void pdte_tecmp_norm_iter(
    vector<Ciphertext>& out,
    Node& root,
    Evaluator* evaluator,
    GaloisKeys* gal_keys_server,
    RelinKeys* rlk_server,
    const vector<vector<Ciphertext>>& client_input,
    const Plaintext& one,
    int l,
    int m,
    uint64_t m_degree,
    BatchEncoder* batch_encoder,
    int slot_count,
    int plain_modulus,
    const seal::Ciphertext& one_zero_init_cipher
) {
    stack<StackFrame> stk;
    stk.push({ &root, false });

    while (!stk.empty()) {
        StackFrame frame = stk.top();
        stk.pop();

        Node* node = frame.node;

        if (node->is_leaf()) {

            out.push_back(node->value);
            continue;
        }

        if (!frame.visited) {
  
            node->right->value = tecmp_norm(
                evaluator, gal_keys_server, rlk_server,
                node->threshold_bitv,
                client_input[node->feature_index],
                l, m, m_degree,
                one_zero_init_cipher
            );

            evaluator->negate(node->right->value, node->left->value);
            evaluator->add_plain_inplace(node->left->value, one);
            evaluator->add_inplace(node->left->value, node->value);
            evaluator->add_inplace(node->right->value, node->value);


            stk.push(StackFrame{ node, true });

            stk.push(StackFrame{ node->right.get(), false });
            stk.push(StackFrame{ node->left.get(), false });
        }
 
    }
}


//g++ -o tecmp_pdte -O3 tecmp_pdte.cpp src/utils.cpp src/cmp.cpp src/node.cpp src/pdte.cpp -I./include -I /usr/local/include/SEAL-4.1 -lseal-4.1

int main(int argc, char* argv[]){
    if (argc < 3) {
        cout << "Need at least a model and a vector" << endl;
        return -1;
    }
    string address_data;
    string address_tree;
    int l,m;
    int data_m;
    int opt;
    int d;
    int extra = 0;
    while ((opt = getopt(argc, argv, "ft:v:r:l:m:d:e:")) != -1) {
        switch (opt) {
        case 't': address_tree = string(optarg);break;       
        case 'v':address_data = string(optarg);break;   
        case 'r':data_m = atoi(optarg);break;   
        case 'l':l = atoi(optarg);break; 
        case 'm':m = atoi(optarg);break;
        case 'd':d = atoi(optarg);break;
        case 'e':extra = atoi(optarg);break;       
        }
    }
    
    int n =l*m;
    stringstream client_send;
    stringstream server_send;
    long client_send_commun=0;
    long server_send_commun=0;

    cout<<"******************************* step 1: client begin *******************************"<<endl;
    clock_t global_start = clock();
    clock_t start = clock();
    
    vector<vector<uint64_t>> client_data = read_csv_to_vector(address_data, data_m);
    data_m = client_data.size();
    int data_n = client_data[0].size();
    cout<<"data_m    = "<<data_m<<endl;
    cout<<"data_n    = "<<data_n<<endl;//print_data(client_data); 
    cout<<"load the client_data,                 run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();
    cout<<"Init fhe ... " <<endl;
    

    EncryptionParameters parms;
    
    parms = tecmp_init(n,l,m,d,extra);

    SEALContext* context = new SEALContext(parms);
    KeyGenerator keygen(*context);
    PublicKey pk;
    keygen.create_public_key(pk);
    SecretKey sk = keygen.secret_key();
    Encryptor *encryptor = new Encryptor(*context, pk);
    Decryptor *decryptor = new Decryptor(*context, sk);
    Evaluator *evaluator = new Evaluator(*context);
    BatchEncoder* batch_encoder = new BatchEncoder(*context);
    RelinKeys* rlk_server = new RelinKeys();
    keygen.create_relin_keys(*rlk_server);
    GaloisKeys* gal_keys_server = new GaloisKeys();
    keygen.create_galois_keys(*gal_keys_server);    
    uint64_t plain_modulus = parms.plain_modulus().value();

    //explain slot info 
    uint64_t slot_count = batch_encoder->slot_count(); // 8192 16384
    uint64_t row_count = slot_count / 2;               // 4096 8192
    uint64_t m_degree = 1 << m ;
    uint64_t num_slots_per_element =  m_degree; 
    uint64_t num_cmps_per_row = row_count/num_slots_per_element;
    uint64_t num_cmps = 2 * num_cmps_per_row;

    cout << "Init fhe finish ... " <<endl;
    cout << "Plaintext matrix num_cmps:              " << num_cmps << endl;

    //data_m < num_cmps 
    if(num_cmps < data_m){
        cout<<"data_m : "<<data_m<<" >= num_cmps : "<<num_cmps<<endl;
        cout<<"data_m is too large, please divide different page until the size is small than num_cmps"<<endl;
        exit(0);
    }
    cout<<"Init the BFV batch scheme,            run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();

    vector<vector<uint64_t>> index_feat = Transpose(client_data); 
    vector<vector<Ciphertext>> client_input;
    for(int i = 0; i < index_feat.size(); i++){
        vector<vector<uint64_t>> encrypted_op_encode = tecmp_encode_b_step_1(index_feat[i],l,m,m_degree,num_cmps);
        vector<vector<uint64_t>> encrypted_op_encode_te = tecmp_encode_b_step_2(encrypted_op_encode,slot_count,row_count,num_cmps_per_row,num_slots_per_element);
        vector<Ciphertext> temp = tecmp_encode_b_enc(encrypted_op_encode_te, batch_encoder, encryptor);
        client_input.push_back(temp);
    }

    Plaintext one_zero_init = init_one_zero_zero(batch_encoder,slot_count,num_cmps,num_cmps_per_row,num_slots_per_element,row_count);
    Ciphertext one_zero_init_cipher;
    encryptor->encrypt(one_zero_init,one_zero_init_cipher);

    cout<<"encrypt the client data ,             run time is "<<(clock()-start) /1000<<" ms"<<endl;start = clock();

    cout<<"******************************* step 1: client end *******************************"<<endl;
    //evaluater
    //client_input
    for(auto e:client_input){
        for(auto f:e){
            client_send_commun+=f.save(client_send);
        }
    }

    cout<<"******************************* step 2: server begin *******************************"<<endl;

    cout<<"load tree"<<endl;
    Node root = Node(address_tree);
    cout<<"root.leaf_num()  = "<<root.leaf_num()<<endl;

    Plaintext one = init_one_one_one(batch_encoder, slot_count);
    root.value = init_zero_zero_zero_cipher(batch_encoder, encryptor, slot_count);
    root.tecmp_pdte_init(l, m);

    cout<<"Init the tree,                          run time is "<<(clock()-start) /1000<<" ms"<<endl;
    start = clock();

    vector<Ciphertext> pdte_out;
    pdte_tecmp_norm_iter( pdte_out, root, evaluator, gal_keys_server, rlk_server, client_input, one, l, m,m_degree, batch_encoder, slot_count, plain_modulus, one_zero_init_cipher);

    uint64_t tree_depth = root.get_depth();
    cout<<"tree_depth : "<<tree_depth<<endl;
    if(d < tree_depth){
        cout<<" The d is too small, should bigger than the tree_depth : "<<tree_depth<<", d : "<<d<<"."<<endl;
        return 0;
    }

    uint64_t d_factorial_inv_with_sign = init_the_d_factorial_inv_with_sign(tree_depth,plain_modulus);
    Plaintext d_factorial_inv_with_sign_pt = init_b_b_b(d_factorial_inv_with_sign, batch_encoder, slot_count);
    for(int i = 0; i < pdte_out.size(); i++){
        pdte_out[i] = map_zero_to_one_and_the_other_to_zero(pdte_out[i],batch_encoder,evaluator,rlk_server,tree_depth,d_factorial_inv_with_sign_pt,slot_count);
    }
    
    vector<uint64_t> leaf_vec;
    leaf_extract_iter(leaf_vec, root);
    vector<Plaintext> leaf_vec_plain;
    for(int i = 0; i<leaf_vec.size(); i++){
        Plaintext pt = init_b_zero_zero(batch_encoder,leaf_vec[i],slot_count,data_m,num_cmps_per_row,num_slots_per_element,row_count); 
        leaf_vec_plain.push_back(pt);
    }
    Ciphertext zero_zero_zero = init_zero_zero_zero_cipher(batch_encoder,encryptor,slot_count);
    Ciphertext server_out = private_info_retrieval_with_b_b_b(evaluator, pdte_out, leaf_vec_plain,leaf_vec,zero_zero_zero);

    clock_t finish = clock() - start; start = clock();

    cout<<"******************************* step 2: server end *******************************"<<endl;
    //server_out
    server_send_commun += server_out.save(server_send);

    cout<<"******************************* step 3: client start *******************************"<<endl;

    Plaintext pt;
    decryptor->decrypt(server_out,pt);
    vector<uint64_t> res;
    batch_encoder->decode(pt, res);
    vector<uint64_t> expect_result(num_cmps);
    for(int j = 0; j < num_cmps ; j++){
        //jdx = 0            num_cmps_per_row                2 * num_cmps_per_row              ...
        //    = row_count    row_count + num_cmps_per_row    row_count + 2 * num_cmps_per_row  ...
        bool flag = j < num_cmps_per_row; 
        uint64_t jdx = flag ? ( j * num_slots_per_element ) : ( row_count + (j - num_cmps_per_row) * num_slots_per_element);
        expect_result[j] = res[jdx];
    }

    cout<<"decrypt the result ,                 run time is "<<(clock()-start) <<"\\mus"<<endl;start = clock();

    for(int j = 0; j < data_m ; j++){
        uint64_t actural_result = root.eval(client_data[j]);
        //cout<< expect_result[j] <<" "<<actural_result<< endl;
        if( !( expect_result[j] == actural_result ) ){
            cout<<"In "<< j <<" col is  error "<< endl;
            cout<<expect_result[j]  <<"  "<<actural_result << endl;
            exit(0);
        }            
    }cout<<endl;

    long comm = client_send_commun + server_send_commun;
    cout<<"compare with the real result , run time is "<<(float)(clock()-start)/1000 <<" ms"<<endl;start = clock();
    cout<<"*********************************************************************************"<<endl;
    cout<<"address_tree : "<<address_tree<<endl;
    cout<<"l            : "<<l<<endl;
    cout<<"m            : "<<m<<endl;
    cout<<"n            : "<<n <<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall run time is "<< finish/1000 <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         overall commun.  is "<< comm/1000 <<" KB"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average run time is "<< (float)finish/1000/data_m <<" ms"<<endl;
    cout<<"PDTE         "<< data_m <<  "   col data ,         average commun.  is "<< comm/1000/data_m <<" KB"<<endl;
    cout<<"******************************* step 3: client end *******************************"<<endl;
    cout<<"all done, overall run time is "<< (clock() - global_start)/1000 <<" ms"<<endl;

    return 0;
}

