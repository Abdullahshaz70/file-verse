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
   
    OFSCore(int totalBlocks = 100):spaceManager(totalBlocks),isInitialized(false){

        header = OMNIHeader(0x00010000, totalBlocks * 4096, sizeof(OMNIHeader), 4096);
        stats = FSStats(totalBlocks * 4096, 0, totalBlocks * 4096);
        cout << "OFSCore initialized with " << totalBlocks << " blocks." << endl;


    }

    ~OFSCore(){ cout << "OFSCore shutting down." << endl;}

   
    
    void format() {
        cout << "Formatting OFS..." << endl;
        spaceManager.reset();
        dirTree.reset();        
        isInitialized = true;
    

    }
    
    void createUser(const string& username, const string& password, bool isAdmin){
        userManager.addUser(username , password , isAdmin);
    }
    
    bool login(const string& username, const string& password){
        return userManager.authenticate(username , password);
    }
    
    void createFile(const string& path, const string& name, const string& data){
        dirTree.createFile(path , name , data);
    }
    
    void deleteFile(const string& path){
        dirTree.deleteNode(path);
    }
    
    void listDirectory(const string& path){
        dirTree.list(path);
    }

    void createDirectory(const string& path, const string& name){
        dirTree.createDirectory(path , name);
    }
    
    void printStats() const{
        spaceManager.print();
    }                         
    bool isSystemReady() const { return isInitialized; }

};
