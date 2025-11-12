// #pragma once
// #include<iostream>
// #include<string>
// #include<sstream>
// #include<algorithm>
// #include<vector>

// #include "core/odf_types.hpp"

// using namespace std;


// struct FileNode{

//     string name;
//     bool isFile;
//     FileNode* parent;
//     string data;
//     vector<FileNode*> children;

//     FileNode(const string& _name, bool _isFile, FileNode* _parent = nullptr)
//         : name(_name), isFile(_isFile), parent(_parent) {}
// };

// class DirectoryTree{


//     FileNode* root;

//     vector<string> slpitDirectory(const string& path){

//         vector<string> result;
//         string token;
//         stringstream ss(path);

//         while(getline(ss , token , '/'))
//             if(!token.empty())
//                 result.push_back(token);
        

//         return result;

//     }

  
//     void deleteNodeRec(FileNode* node){
//         if (!node) return; 
//         for (auto* child : node->children)
//             deleteNodeRec(child);
//         node->children.clear(); 
//         delete node;
//     }

    




// public:

//     DirectoryTree():root(nullptr){root = new FileNode("root", false, nullptr); }
//     ~DirectoryTree() { 
//         deleteNodeRec(root);
//         root = nullptr;
//     }

//       FileNode* findNodeByPath(const string& path){

//         if(!root)
//             return nullptr;

//         if(path=="/" || path.empty())
//             return root;

//         vector<string> folders = slpitDirectory(path);
//         FileNode* curr = root;
        
//         for (size_t i = 0; i < folders.size(); ++i) {
//             const string& folder = folders[i];
//             bool found = false;

//             for (FileNode* child : curr->children) {
//                 if (child->name == folder && (i == folders.size() - 1 || !child->isFile)) {
//                     curr = child;
//                     found = true;
//                     break;
//                 }
//             }
//             if (!found) return nullptr;
//         }
//         return curr;

//     }


//     bool createDirectory(const string& path , const string& name){

//         FileNode* parent = findNodeByPath(path);
//         if(!parent) return false; 

//         FileNode* newDir = new FileNode(name , false , parent);
//         parent->children.push_back(newDir);

//         return true;
//     }

//     bool createFile(const string& path , const string& name , const string& data){

//         if(!root)
//             return false;

//         FileNode* parent = findNodeByPath(path);
//         if(!parent)
//             return false;

//         if(parent->isFile)
//             return false;

//         for(FileNode* child : parent->children){
//             if(child->name == name){
//                 cout<<"Name is already in used"<<endl;
//                 return false;
//             }
//         }

//         FileNode* newFile = new FileNode(name, true, parent);
//         newFile->data = data;
//         parent->children.push_back(newFile);

//         return true;

//     }

//     bool deleteNode(const string& path) {
//     if (!root) return false;

//     FileNode* n = findNodeByPath(path);
//     if (!n || n == root) return false;

//     FileNode* parent = n->parent;
//     if (!parent) return false;

    
//     auto& siblings = parent->children;
//     siblings.erase(remove(siblings.begin(), siblings.end(), n), siblings.end());

    
//     deleteNodeRec(n);
//     return true;
//     }


//     // ===============================
//     // 🧩 Clean recursive print function
//     // ===============================
//     void printTree(FileNode* node, int depth = 0) const {
//         if (!node) return;

//         for (int i = 0; i < depth; ++i) cout << "  ";

//         if (node->isFile)
//             cout << "📄 " << node->name << endl;
//         else
//             cout << "📁 " << node->name << endl;

//         // Sort children so output is consistent
//         vector<FileNode*> sorted = node->children;
//         sort(sorted.begin(), sorted.end(),
//              [](FileNode* a, FileNode* b) { return a->name < b->name; });

//         for (auto* child : sorted)
//             printTree(child, depth + 1);
//     }
   
   
//     void list(const string& path) {
//         if (!root) {
//             cout << "No file system initialized.\n";
//             return;
//         }

//         FileNode* dir = findNodeByPath(path);
//         if (!dir) {
//             cout << "Path not found.\n";
//             return;
//         }

//         if (dir->isFile) {
//             cout << dir->name << " (File)\n";
//             return;
//         }

//         if (dir->children.empty()) {
//             cout << "(Empty directory)\n";
//             return;
//         }

//         cout << "Contents of " << path << ":\n";
//         for (FileNode* child : dir->children) {
//             cout << "  " << (child->isFile ? "[File] " : "[Dir] ") << child->name << endl;
//         }
//     }


//     FileNode* getRoot() { return root; }

//     void reset() {
//         deleteNodeRec(root);
//         root = new FileNode("root", false, nullptr);
//     }


    
// void exportToEntries(vector<FileEntry>& entries, const string& currentPath = "/") {
//         if (!root) return;
//         exportNode(root, currentPath, entries);
//     }

// void exportNode(FileNode* node, const string& path, vector<FileEntry>& entries) {
//     if (!node) return;

//     FileEntry entry;
//     memset(&entry, 0, sizeof(FileEntry));

//     strcpy(entry.name, (path + node->name).c_str());
//     entry.type = node->isFile ? 0 : 1;
//     entry.size = node->isFile ? node->data.size() : 0;
//     entry.permissions = 0644;
//     strcpy(entry.owner, "admin");
//     entry.inode = reinterpret_cast<uint64_t>(node) & 0xFFFFFFFF;  

//     entries.push_back(entry);

    
//     if (!node->isFile) {
//         for (auto* child : node->children)
//             exportNode(child, path + node->name + "/", entries);
//     }
// }




// // ================================
// // 🏠 User-based directory helpers
// // ================================
// bool ensureHomeBase() {
//     // Ensure /home directory exists
//     FileNode* home = findNodeByPath("/home");
//     if (!home) {
//         return createDirectory("/", "home");
//     }
//     return true;
// }

// bool createUserHome(const string& username) {
//     ensureHomeBase();
//     // check if already exists
//     FileNode* existing = findNodeByPath("/home/" + username);
//     if (existing) return false;  // already has home dir
//     return createDirectory("/home", username);
// }



//    // Clean version — no duplicate subpaths
//     void listUserFiles(const string& username) {
//         FileNode* home = findNodeByPath("/home/" + username);
//         if (!home) {
//             cout << "❌ No directory found for user '" << username << "'.\n";
//             return;
//         }
//         cout << "📂 Files for user " << username << ":\n";
//         printTree(home, 1);
//     }

//     void listAll() {
//         cout << "\n🌳 Complete File System Tree:\n";
//         printTree(root);
//     }


// };







#pragma once
#include<iostream>
#include<string>
#include<sstream>
#include<algorithm>
#include<vector>

#include "core/odf_types.hpp"

using namespace std;

struct FileNode {
    string name;
    bool isFile;
    FileNode* parent;
    string data;
    vector<FileNode*> children;

    FileNode(const string& _name, bool _isFile, FileNode* _parent = nullptr)
        : name(_name), isFile(_isFile), parent(_parent) {}
};

class DirectoryTree {

    FileNode* root;

    vector<string> splitDirectory(const string& path) {
        vector<string> result;
        string token;
        stringstream ss(path);
        while (getline(ss, token, '/'))
            if (!token.empty())
                result.push_back(token);
        return result;
    }

    void deleteNodeRec(FileNode* node) {
        if (!node) return;
        for (auto* child : node->children)
            deleteNodeRec(child);
        node->children.clear();
        delete node;
    }

public:
    DirectoryTree() { root = new FileNode("root", false, nullptr); }
    ~DirectoryTree() { deleteNodeRec(root); root = nullptr; }

    FileNode* findNodeByPath(const string& path) {
        if (!root) return nullptr;
        if (path == "/" || path.empty()) return root;

        vector<string> folders = splitDirectory(path);
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

    bool createDirectory(const string& path, const string& name) {
        FileNode* parent = findNodeByPath(path);
        if (!parent) return false;

        // prevent duplicates
        for (auto* c : parent->children)
            if (!c->isFile && c->name == name)
                return false;

        FileNode* newDir = new FileNode(name, false, parent);
        parent->children.push_back(newDir);
        return true;
    }

    bool createFile(const string& path, const string& name, const string& data) {
        if (!root) return false;

        FileNode* parent = findNodeByPath(path);
        if (!parent || parent->isFile) return false;

        for (FileNode* child : parent->children) {
            if (child->name == name) {
                cout << "⚠️ Name already in use: " << name << endl;
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

    // ===============================
    // 🧩 Clean recursive print function
    // ===============================
    void printTree(FileNode* node, int depth = 0) const {
        if (!node) return;

        for (int i = 0; i < depth; ++i) cout << "  ";

        if (node->isFile)
            cout << "📄 " << node->name << endl;
        else
            cout << "📁 " << node->name << endl;

        // Sort children so output is consistent
        vector<FileNode*> sorted = node->children;
        sort(sorted.begin(), sorted.end(),
             [](FileNode* a, FileNode* b) { return a->name < b->name; });

        for (auto* child : sorted)
            printTree(child, depth + 1);
    }

    // For listing directory contents (non-recursive)
    void list(const string& path) {
        if (!root) {
            cout << "❌ No file system initialized.\n";
            return;
        }

        FileNode* dir = findNodeByPath(path);
        if (!dir) {
            cout << "❌ Path not found.\n";
            return;
        }

        if (dir->isFile) {
            cout << "📄 " << dir->name << endl;
            return;
        }

        if (dir->children.empty()) {
            cout << "(Empty directory)\n";
            return;
        }

        cout << "Contents of " << path << ":\n";
        for (FileNode* child : dir->children)
            cout << "  " << (child->isFile ? "[File] " : "[Dir] ") << child->name << endl;
    }

    FileNode* getRoot() { return root; }

    void reset() {
        deleteNodeRec(root);
        root = new FileNode("root", false, nullptr);
    }

    // ==========================================
    // 🏠 User-based directory helpers
    // ==========================================
    bool ensureHomeBase() {
        FileNode* home = findNodeByPath("/home");
        if (!home) return createDirectory("/", "home");
        return true;
    }

    bool createUserHome(const string& username) {
        ensureHomeBase();
        FileNode* existing = findNodeByPath("/home/" + username);
        if (existing) return false;
        return createDirectory("/home", username);
    }

    // Clean version — no duplicate subpaths
    void listUserFiles(const string& username) {
        FileNode* home = findNodeByPath("/home/" + username);
        if (!home) {
            cout << "❌ No directory found for user '" << username << "'.\n";
            return;
        }
        cout << "📂 Files for user " << username << ":\n";
        printTree(home, 1);
    }

    void listAll() {
        cout << "\n🌳 Complete File System Tree:\n";
        printTree(root);
    }

    // ==========================================
    // For saving directory structure to .omni
    // ==========================================
    void exportToEntries(vector<FileEntry>& entries, const string& currentPath = "/") {
        if (!root) return;
        exportNode(root, currentPath, entries);
    }

    void exportNode(FileNode* node, const string& path, vector<FileEntry>& entries) {
        if (!node) return;

        FileEntry entry;
        memset(&entry, 0, sizeof(FileEntry));
        strcpy(entry.name, (path + node->name).c_str());
        entry.type = node->isFile ? 0 : 1;
        entry.size = node->isFile ? node->data.size() : 0;
        entry.permissions = 0644;
        strcpy(entry.owner, "admin");
        entry.inode = reinterpret_cast<uint64_t>(node) & 0xFFFFFFFF;
        entries.push_back(entry);

        if (!node->isFile) {
            for (auto* child : node->children)
                exportNode(child, path + node->name + "/", entries);
        }
    }
};







