#pragma once
#include<iostream>
#include<string>
#include<sstream>
#include<algorithm>
#include<vector>


#include "../include/core/odf_types.hpp"


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

 bool createDirectory(const string& basePath, const string& relativePath) {
    if (relativePath.empty()) return false;

    vector<string> parts = splitDirectory(relativePath);
    FileNode* current = findNodeByPath(basePath);
    if (!current) return false;

    for (const string& part : parts) {
        if (part.empty()) continue;

        FileNode* next = nullptr;
        for (auto* c : current->children) {
            if (!c->isFile && c->name == part) {
                next = c;
                break;
            }
        }

        if (!next) {
            next = new FileNode(part, false, current);
            current->children.push_back(next);
        }
        current = next;
    }

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



bool createDirectoryRecursive(const string& fullPath) {
    if (fullPath.empty()) return false;

    size_t pos = fullPath.find_last_of('/');
    if (pos == string::npos) return false;

    string base = fullPath.substr(0, pos);
    string rel  = fullPath.substr(pos + 1);

    return createDirectory(base, rel);
}


bool deleteFile(const string& fullPath) {
    FileNode* node = findNodeByPath(fullPath);
    if (!node) {
        cout << "❌ File not found: " << fullPath << endl;
        return false;
    }
    if (!node->isFile) {
        cout << "⚠️  Path is a directory. Use deleteDirectory() instead.\n";
        return false;
    }

    FileNode* parent = node->parent;
    if (!parent) return false;

    auto& siblings = parent->children;
    siblings.erase(remove(siblings.begin(), siblings.end(), node), siblings.end());
    deleteNodeRec(node);

    cout << "🗑️  File deleted: " << fullPath << endl;
    return true;
}

bool deleteDirectoryRecursive(const string& dirPath) {
    FileNode* node = findNodeByPath(dirPath);
    if (!node) {
        cout << "❌ Directory not found: " << dirPath << endl;
        return false;
    }
    if (node->isFile) {
        cout << "⚠️  Path is a file. Use deleteFile() instead.\n";
        return false;
    }

    if (node == root) {
        cout << "⚠️  Cannot delete root directory.\n";
        return false;
    }

    FileNode* parent = node->parent;
    if (!parent) return false;

    auto& siblings = parent->children;
    siblings.erase(remove(siblings.begin(), siblings.end(), node), siblings.end());
    deleteNodeRec(node);

    cout << "🗑️  Directory deleted (and all sub-contents removed): " << dirPath << endl;
    return true;
}



    void printTree(FileNode* node, int depth = 0) const {
        if (!node) return;

        for (int i = 0; i < depth; ++i) cout << "  ";

        if (node->isFile)
            cout << "📄 " << node->name << endl;
        else
            cout << "📁 " << node->name << endl;

        vector<FileNode*> sorted = node->children;
        sort(sorted.begin(), sorted.end(),
             [](FileNode* a, FileNode* b) { return a->name < b->name; });

        for (auto* child : sorted)
            printTree(child, depth + 1);
    }

    
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


void exportToEntries(vector<FileEntry>& entries) {
    if (!root) return;
    for (auto* child : root->children) {
        exportNode(child, "/", entries); 
    }
}

void exportNode(FileNode* node, const string& path, vector<FileEntry>& entries) {
    if (!node) return;

    string full = (path == "/") ? ("/" + node->name) : (path + "/" + node->name);

    FileEntry entry{};
    memset(&entry, 0, sizeof(FileEntry));
    strncpy(entry.name, full.c_str(), sizeof(entry.name) - 1);
    entry.type = node->isFile ? 0 : 1;
    entry.size = node->isFile ? node->data.size() : 0;
    entry.permissions = 0644;
    strncpy(entry.owner, "admin", sizeof(entry.owner) - 1);
    entry.inode = reinterpret_cast<uint64_t>(node) & 0xFFFFFFFF;
    entries.push_back(entry);

    if (!node->isFile) {
        for (auto* child : node->children)
            exportNode(child, full, entries);
    }
}

void importFromEntries(const vector<FileEntry>& entries) {
    reset();

    auto ensurePath = [&](const string& fullPath, bool isDir) {
        vector<string> parts = splitDirectory(fullPath);
        FileNode* cur = root;

        for (size_t i = 0; i < parts.size(); ++i) {
            bool last = (i == parts.size() - 1);
            const string& name = parts[i];

            FileNode* child = nullptr;
            for (auto* c : cur->children)
                if (c->name == name) { child = c; break; }

            if (!child) {
                child = new FileNode(name, last && !isDir, cur);
                cur->children.push_back(child);
            }
            cur = child;
        }
    };

    for (const auto& e : entries) {
        if (e.name[0] == '\0') continue;
        string path = e.name;
        bool isDir = (e.type == 1);
        ensurePath(path, isDir);
    }
}



};







