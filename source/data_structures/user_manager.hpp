#include<iostream>
#include<string>
using namespace std;


struct UserNode
{
    UserNode* left;
    UserNode* right;
    string userName;
    string password;
    bool isAdmin;
    int height;

    UserNode(const string& _username, const string& _password, bool _isAdmin)
    :left(nullptr),right(nullptr) , userName(_username) , password(_password) , isAdmin(_isAdmin) , height(1){}

};

class UserManager{

    UserNode* root;
    
    UserNode* insertNode(UserNode* node, const string& username, const string& password, bool isAdmin){
        if(!node)
            return  new UserNode (username , password , isAdmin);

        if(username<node->userName)
            node->left = insertNode(node->left , username , password , isAdmin);
        
        else if (username > node->userName)
            node->right = insertNode(node->right , username , password , isAdmin);

        else 
            return node;

        
        node->height = getHeight(node);

        int balance = getBalance(node);

        if(balance >1 && username < node->left->userName)
            return rightRotate(node);
        
        if(balance > 1 && username > node->left->userName){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if(balance < -1 && username > node->right->userName)
            return leftRotate(node);

        if(balance < -1 && username < node->right->userName){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
        
    }

    void inorderPrint(UserNode* r) {
        if (!r) return;
        inorderPrint(r->left);
        cout << r->userName << " (" << (r->isAdmin ? "Admin" : "User") << ")\n";
        inorderPrint(r->right);
    }

    UserNode* isPresent(UserNode* r , const string& username){

        if(!r || r->userName == username)
            return r;

        if(username < r->userName)
            return isPresent(r->left , username);
        return isPresent(r->right , username);

    }

    int getHeight(UserNode* r){

        if(!r)
            return 0;

        return 1 + max(getHeight(r->left) , getHeight(r->right)); 
    }

    int getBalance(UserNode* r){
        if(!r)
            return 0;

        return (getHeight(r->left) - getHeight(r->right));
    }

    UserNode* leftRotate(UserNode* r){

        UserNode* right = r->right;
        UserNode* left = right->left;

        right->left = r;
        r->right = left;

        r->height = getHeight(r);
        right->height = getHeight(right);

        

        return right;
    }

    UserNode* rightRotate(UserNode* r){

        UserNode* left = r->left;
        UserNode* right = left->right;

        left->right = r;
        r->left = right;

        r->height = getHeight(r);
        left->height = getHeight(left);


        return left;
    }

    UserNode* findSucc(UserNode* r){
        UserNode* curr = r;
        while(curr && curr->left)
            curr = curr->left;
        return curr;
    }

    UserNode* deleteNode(UserNode* node, const string& username){
        if(!node)
            return node;

        if(username < node->userName)
            node->left = deleteNode(node->left , username);

        else if(username > node->userName)
            node->right = deleteNode(node->right , username);

        else{

            if(!node->right || !node->left){
                UserNode* temp = node->left ? node->left : node->right;

                if(!temp){
                    delete node;
                    node = nullptr;
                }
                else{
                    *node = *temp;
                    delete temp;
                }
            }
            else{
                UserNode* succ = findSucc(node->right);
                node->userName = succ->userName;
                node->isAdmin = succ->isAdmin;
                node->height = succ->height;
                node->password = succ->password;
                node->right = deleteNode(node->right , succ->userName);
            }
        }

        if(!node) return node;

        node->height = getHeight(node);

        int balance = getBalance(node);

        if(balance >1 && username < node->left->userName)
            return rightRotate(node);
        
        if(balance > 1 && username > node->left->userName){
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        if(balance < -1 && username > node->right->userName)
            return leftRotate(node);

        if(balance < -1 && username < node->right->userName){
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }
    
    void deleteTree(UserNode* node){
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    }

public:
    UserManager():root(nullptr){}
    ~UserManager(){deleteTree(root);}

 

    bool addUser(const string& username, const std::string& password, bool isAdmin){

        if(isPresent(root , username)){
            cout<<"Username is Already Registered"<<endl;
            return false;
        }

        root = insertNode(root ,username , password , isAdmin);
        return true;
    }

    bool authenticate(const string& username, const string& password){
        UserNode* user = isPresent(root , username);
        return (user && user->password == password);
    }
    
    bool removeUser(const string& username){
        if(!isPresent(root,username)){
            cout<<"Username not found to delete!"<<endl;
            return false;
        }

        root = deleteNode(root , username);
        return true;
    }
    
    void print() { inorderPrint(root); }

    UserNode* getRoot(){return root;}
};
