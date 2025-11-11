#pragma once
#include <string>
#include <iostream>

#include "../../data_structures/user_manager.hpp"
#include "../../data_structures/directory_tree.hpp"
#include "../../data_structures/free_space.hpp"
#include "odf_types.hpp"   
#include "file_io_manager.hpp"


using namespace std;

class OFSCore {
private:
    
    UserManager userManager;
    DirectoryTree dirTree;
    FreeSpace spaceManager;

   
    OMNIHeader header;
    FSStats stats;
    bool isInitialized;     

    FileIOManager fileManager;
    string omniFileName = "filesystem.omni";

    uint64_t dataStartOffset = 0;


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

        uint64_t totalSize = 1024 * 1024;   
        uint64_t blockSize = 4096;

        fileManager.createOmniFile(omniFileName, totalSize, blockSize);
        fileManager.openFile(omniFileName, blockSize);

        OMNIHeader header(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
        strcpy(header.magic, "OMNIFS01");
        strcpy(header.student_id, "2022-CS-7062");
        strcpy(header.submission_date, "2025-11-09");

        fileManager.writeHeader(header);

        vector<UserInfo> emptyUsers(10); 
        uint64_t userTableOffset = sizeof(OMNIHeader);
        fileManager.writeUsers(emptyUsers, userTableOffset);

        vector<bool> freeMap(256, false); 
        uint64_t freeMapOffset = userTableOffset + (emptyUsers.size() * sizeof(UserInfo));
        fileManager.writeFreeMap(freeMap, freeMapOffset);

        vector<FileEntry> entries;
        dirTree.exportToEntries(entries);   
        uint64_t metaOffset = freeMapOffset + freeMap.size();
        fileManager.writeFileEntries(entries, metaOffset);

        dataStartOffset = metaOffset + (entries.size() * sizeof(FileEntry));


        fileManager.closeFile();

        isInitialized = true;
        cout << "OFS formatted and .omni file created successfully.\n";
    }


    bool loadSystem() {
        cout << "Loading OFS from " << omniFileName << "...\n";
        uint64_t blockSize = 4096;


        if (!fileManager.openFile(omniFileName, blockSize)) {
            cerr << "Error: Could not open .omni file. Please format first.\n";
            return false;
        }

        OMNIHeader header;
        if (!fileManager.readHeader(header)) {
            cerr << "Error reading header.\n";
            return false;
        }

        cout << "Loaded OMNI file successfully.\n";
        cout << "Magic: " << header.magic << endl;
        cout << "Total Size: " << header.total_size << " bytes\n";
        cout << "Block Size: " << header.block_size << " bytes\n";
        cout << "Student ID: " << header.student_id << "\n";
        cout << "Submission Date: " << header.submission_date << "\n";

        
        
        
        vector<UserInfo> loadedUsers;
        uint64_t userTableOffset = sizeof(OMNIHeader);
        fileManager.readUsers(loadedUsers, userTableOffset, 10);

        cout << "\n--- Loaded Users ---\n";
        for (const auto& u : loadedUsers) {
            if (u.is_active)
                cout << "Username: " << u.username
                     << " | Role: " << (u.role == UserRole::ADMIN ? "Admin" : "User") << "\n";
        }

        vector<bool> loadedMap;
        uint64_t freeMapOffset = userTableOffset + (loadedUsers.size() * sizeof(UserInfo));
        fileManager.readFreeMap(loadedMap, freeMapOffset, 256);

        cout << "\nFree Blocks Available: "
             << count(loadedMap.begin(), loadedMap.end(), false) << "\n";


            
        vector<FileEntry> entries;
        uint64_t metaOffset = freeMapOffset + loadedMap.size();
        fileManager.readFileEntries(entries, metaOffset, 50); 

        cout << "\n--- Directory Metadata Loaded ---\n";
        for (auto& e : entries)
            if (strlen(e.name) > 0)
                cout << (e.type == 1 ? "[DIR] " : "[FILE] ") << e.name
                    << " (" << e.size << " bytes)\n";


        fileManager.closeFile();
        isInitialized = true;
        return true;
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



    bool writeFileContent(const string& filePath, const string& fileData) {
        cout << "Writing file content for: " << filePath << endl;
        fileManager.openFile(omniFileName, 4096);


        int blockIndex = spaceManager.allocateBlock();
        if (blockIndex == -1) {
            cerr << "No free space available.\n";
            return false;
        }

        
        vector<char> buffer(fileData.begin(), fileData.end());

        
        fileManager.writeFileData(dataStartOffset, blockIndex, 4096, buffer);
        vector<bool> freeMap = spaceManager.getMap();
        uint64_t freeMapOffset = sizeof(OMNIHeader) + (10 * sizeof(UserInfo));
        fileManager.writeFreeMap(freeMap, freeMapOffset);

        fileManager.closeFile();
        cout << "File stored successfully at block #" << blockIndex << "\n";
        return true;
    }

    bool readFileContent(uint32_t blockIndex, uint32_t dataLength) {
        cout << "Reading data from block #" << blockIndex << endl;
        fileManager.openFile(omniFileName, 4096);

        vector<char> buffer;
        fileManager.readFileData(dataStartOffset, blockIndex, 4096, buffer);
        fileManager.closeFile();

        string content(buffer.begin(), buffer.begin() + dataLength);
        cout << "File content:\n" << content << endl;
        return true;
    }


};
