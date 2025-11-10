#pragma once
#include <string>
#include <iostream>

#include "../../data_structures/user_manager.hpp"
#include "../../data_structures/directory_tree.hpp"
#include "../../data_structures/free_space.hpp"
#include "odf_types.hpp"   

using namespace std;

class OFSCore {
private:
    
    UserManager userManager;
    DirectoryTree dirTree;
    FreeSpace spaceManager;

   
    OMNIHeader header;
    FSStats stats;
    bool isInitialized;

public:
   
    OFSCore(int totalBlocks = 100);
    ~OFSCore();

   
    void format(); 
    void createUser(const string& username, const string& password, bool isAdmin);
    bool login(const string& username, const string& password);
    void createFile(const string& path, const string& name, const string& data);
    void deleteFile(const string& path);
    void listDirectory(const string& path);
};
