#include<iostream>
#include<string>
#include<sstream>
#include<algorithm>
#include<vector>
using namespace std;


struct FileNode{

    string name;
    bool isFile;
    FileNode* parent;
    string data;
    vector<FileNode*> children;

    FileNode(const string& _name, bool _isFile, FileNode* _parent = nullptr)
        : name(_name), isFile(_isFile), parent(_parent) {}
};

class DirectoryTree{


    FileNode* root;

    vector<string> slpitDirectory(const string& path){

        vector<string> result;
        string token;
        stringstream ss(path);

        while(getline(ss , token , '/'))
            if(!token.empty())
                result.push_back(token);
        

        return result;

    }

    FileNode* findNodeByPath(const string& path){

        if(!root)
            return nullptr;

        if(path=="/" || path.empty())
            return root;

        vector<string> folders = slpitDirectory(path);
        FileNode* curr = root;
        
        for (size_t i = 0; i < folders.size(); ++i) {
            const string& folder = folders[i];
            bool found = false;

            for (FileNode* child : curr->children) {
                if (child->name == folder && (i == folders.size() - 1 || !child->isFile)) {
                    curr = child;
                    found = true;
                    break;
                }
            }
            if (!found) return nullptr;
        }
        return curr;

    }

    void deleteNodeRec(FileNode* node) {
        for (FileNode* child : node->children)
            deleteNodeRec(child);
        delete node;
    }


public:

    DirectoryTree():root(nullptr){root = new FileNode("root", false, nullptr); }
    ~DirectoryTree() { deleteNodeRec(root); }


    bool createDirectory(const string& path , const string& name){

        FileNode* parent = findNodeByPath(path);
        if(!parent) return false; 

        FileNode* newDir = new FileNode(name , false , parent);
        parent->children.push_back(newDir);

        return true;
    }

    bool createFile(const string& path , const string& name , const string& data){

        if(!root)
            return false;

        FileNode* parent = findNodeByPath(path);
        if(!parent)
            return false;

        if(parent->isFile)
            return false;

        for(FileNode* child : parent->children){
            if(child->name == name){
                cout<<"Name is already in used"<<endl;
                return false;
            }
        }

        FileNode* newFile = new FileNode(name, true, parent);
        newFile->data = data;
        parent->children.push_back(newFile);

        return true;

    }

    bool deleteNode(const string& path) {
    if (!root) return false;

    FileNode* n = findNodeByPath(path);
    if (!n || n == root) return false;

    FileNode* parent = n->parent;
    if (!parent) return false;

    
    auto& siblings = parent->children;
    siblings.erase(remove(siblings.begin(), siblings.end(), n), siblings.end());

    
    deleteNodeRec(n);
    return true;
    }


    void printTree(FileNode* node, int depth = 0) {
        if (!node) return;
        for (int i = 0; i < depth; ++i) cout << "  ";
        cout << (node->isFile ? "- " : "[DIR] ") << node->name << endl;
        for (FileNode* child : node->children)
            printTree(child, depth + 1);
    }

    void list(const string& path) {
        if (!root) {
            cout << "No file system initialized.\n";
            return;
        }

        FileNode* dir = findNodeByPath(path);
        if (!dir) {
            cout << "Path not found.\n";
            return;
        }

        if (dir->isFile) {
            cout << dir->name << " (File)\n";
            return;
        }

        if (dir->children.empty()) {
            cout << "(Empty directory)\n";
            return;
        }

        cout << "Contents of " << path << ":\n";
        for (FileNode* child : dir->children) {
            cout << "  " << (child->isFile ? "[File] " : "[Dir] ") << child->name << endl;
        }
    }


    FileNode* getRoot() { return root; }


};