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
    uint64_t totalBlocks = 256;

    bool isInitialized;     

    FileIOManager fileManager;
    string omniFileName = "filesystem.omni";

    uint64_t dataStartOffset = 0;





    void updateStats() {
    vector<bool> map = spaceManager.getMap();
    uint64_t used = count(map.begin(), map.end(), true);
    uint64_t free = count(map.begin(), map.end(), false);

    stats.total_size = header.total_size;
    uint64_t blockSize = header.block_size;
    stats.used_space = used * blockSize;
    stats.free_space = free * blockSize;
    stats.total_files = 0;
    stats.total_directories = 1;
    stats.total_users = 1;
    stats.active_sessions = 0;
    stats.fragmentation = ((double)used / (double)totalBlocks) * 100.0;

    cout << "\n--- File System Stats Updated ---\n";
    cout << "Total Size: " << stats.total_size / 1024 << " KB\n";
    cout << "Used Space: " << stats.used_space / 1024 << " KB\n";
    cout << "Free Space: " << stats.free_space / 1024 << " KB\n";
    cout << "Fragmentation: " << stats.fragmentation << "%\n";
}



public:
   
    OFSCore(int blocks = 256)
    : spaceManager(blocks), totalBlocks(blocks), isInitialized(false) {
    uint64_t blockSize = 4096;
    uint64_t totalSize = blocks * blockSize;
    header = OMNIHeader(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
    stats = FSStats(totalSize, 0, totalSize);
    cout << "OFSCore initialized with " << blocks << " blocks." << endl;
}

    ~OFSCore(){ cout << "OFSCore shutting down." << endl;}

    void format() {
    cout << "Formatting OFS..." << endl;

    spaceManager.reset();
    dirTree.reset();

    uint64_t totalSize = totalBlocks * 4096;
    uint64_t blockSize = 4096;

    fileManager.createOmniFile(omniFileName, totalSize, blockSize);
    fileManager.openFile(omniFileName, blockSize);

    header = OMNIHeader(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
    strcpy(header.magic, "OMNIFS01");
    strcpy(header.student_id, "2022-CS-7062");
    strcpy(header.submission_date, "2025-11-09");
    fileManager.writeHeader(header);

    vector<UserInfo> emptyUsers(10);
    uint64_t userTableOffset = sizeof(OMNIHeader);
    fileManager.writeUsers(emptyUsers, userTableOffset);

    vector<bool> freeMap(totalBlocks, false);
    uint64_t freeMapOffset = userTableOffset + (emptyUsers.size() * sizeof(UserInfo));
    fileManager.writeFreeMap(freeMap, freeMapOffset);

    vector<FileEntry> entries;
    dirTree.exportToEntries(entries);
    uint64_t metaOffset = freeMapOffset + freeMap.size();
    fileManager.writeFileEntries(entries, metaOffset);

    dataStartOffset =
        sizeof(OMNIHeader) +
        (emptyUsers.size() * sizeof(UserInfo)) +
        freeMap.size() +
        (entries.size() * sizeof(FileEntry));


        
        header.file_state_storage_offset = dataStartOffset + (totalBlocks * header.block_size);
        header.change_log_offset = header.file_state_storage_offset + (256 * sizeof(uint64_t));

        
        fileManager.openFile(omniFileName, 4096);
        fileManager.writeHeader(header);
        fileManager.closeFile();



    fileManager.closeFile();

    isInitialized = true;
    cout << "OFS formatted and .omni file created successfully.\n";

    updateStats();
}


    bool loadSystem() {
    cout << "Loading OFS from " << omniFileName << "...\n";
    uint64_t blockSize = 4096;

    if (!fileManager.openFile(omniFileName, blockSize)) {
        cerr << "Error: Could not open .omni file. Please format first.\n";
        return false;
    }

    OMNIHeader tmp;
    if (!fileManager.readHeader(tmp)) {
        cerr << "Error reading header.\n";
        return false;
    }
    header = tmp;

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
    fileManager.readFreeMap(loadedMap, freeMapOffset, totalBlocks);
    spaceManager.setMap(loadedMap); 

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

    dataStartOffset =
        sizeof(OMNIHeader) +
        (loadedUsers.size() * sizeof(UserInfo)) +
        loadedMap.size() +
        (entries.size() * sizeof(FileEntry));

    fileManager.closeFile();
    isInitialized = true;
    updateStats(); 
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
    
    void spacePrint() const{
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

        updateStats();


        fileManager.closeFile();
        cout << "File stored successfully at block #" << blockIndex << "\n";
        logChange(filePath, "admin", "MODIFY", 1);

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



    void logChange(const string& path, const string& user, const string& action, uint64_t versionID) {
        fileManager.openFile(omniFileName, 4096);

        ChangeLogEntry entry;
        strncpy(entry.filePath, path.c_str(), sizeof(entry.filePath) - 1);
        strncpy(entry.user, user.c_str(), sizeof(entry.user) - 1);
        strncpy(entry.action, action.c_str(), sizeof(entry.action) - 1);
        entry.timestamp = time(nullptr);
        entry.versionID = versionID;

        fileManager.writeChangeLog({ entry }, header.change_log_offset);
        fileManager.closeFile();
    }




    void printStats() const {
        cout << "\n--- OFS Statistics ---\n";
        cout << "Total Size: " << stats.total_size / 1024 << " KB\n";
        cout << "Used Space: " << stats.used_space / 1024 << " KB\n";
        cout << "Free Space: " << stats.free_space / 1024 << " KB\n";
        cout << "Total Blocks: " << totalBlocks << "\n";
        cout << "Fragmentation: " << stats.fragmentation << "%\n";
        cout << "-------------------------\n";
    }


        void showChangeLog() {
        vector<ChangeLogEntry> log;
        fileManager.openFile(omniFileName, 4096);
        fileManager.readChangeLog(log, header.change_log_offset, 10);
        fileManager.closeFile();

        cout << "\n--- Change Log ---\n";
        for (auto& e : log) {
            if (strlen(e.filePath) > 0) {
                cout << e.filePath << " | " << e.action
                     << " | " << e.user
                     << " | v" << e.versionID
                     << " | " << ctime((time_t*)&e.timestamp);
            }
        }
    }
    void modifyFileVersion(const string& path, const string& newData) {
        writeFileContent(path, newData);
        logChange(path, "admin", "MODIFY", 2);
    }


};

