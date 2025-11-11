#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstring>

#include "../../data_structures/session_manger.hpp"
#include "../../data_structures/user_manager.hpp"
#include "../../data_structures/directory_tree.hpp"
#include "../../data_structures/free_space.hpp"
#include "odf_types.hpp"
#include "file_io_manager.hpp"

using namespace std;

class OFSCore {


    UserManager userManager;
    DirectoryTree dirTree;
    FreeSpace spaceManager;
    FileIOManager fileManager;

    OMNIHeader header{};
    FSStats stats{};
    uint64_t totalBlocks = 2048;
    uint64_t dataStartOffset = 0;
    bool isInitialized = false;
    string omniFileName = "filesystem.omni";

    
    SessionManager sessionManager;   

    

    // =============================================================
    // 🧮 Update runtime FS statistics
    // =============================================================
    void updateStats() {
        vector<bool> map = spaceManager.getMap();
        uint64_t used = count(map.begin(), map.end(), true);
        uint64_t free = count(map.begin(), map.end(), false);

        stats.total_size = header.total_size;
        stats.used_space = used * header.block_size;
        stats.free_space = free * header.block_size;
        stats.fragmentation = ((double)used / (double)totalBlocks) * 100.0;

        cout << "\n--- File System Stats Updated ---\n";
        cout << "Total Size: " << stats.total_size / 1024 << " KB\n";
        cout << "Used Space: " << stats.used_space / 1024 << " KB\n";
        cout << "Free Space: " << stats.free_space / 1024 << " KB\n";
        cout << "Fragmentation: " << stats.fragmentation << "%\n";
    }

public:
    // =============================================================
    // 🧱 Constructor
    // =============================================================
    OFSCore(int blocks = 2048)
    : spaceManager(blocks), totalBlocks(blocks),
      isInitialized(false), sessionManager(&userManager)
{
    uint64_t blockSize = 4096;
    uint64_t totalSize = blocks * blockSize;
    header = OMNIHeader(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
    stats = FSStats(totalSize, 0, totalSize);

    
    userManager.addUser("admin", "admin123", true);
    cout << "Default admin (admin / admin123) created.\n";

    cout << "OFSCore initialized with " << blocks
         << " blocks (" << totalSize / 1024 << " KB)." << endl;
}

    ~OFSCore() { cout << "OFSCore shutting down." << endl; }

    // =============================================================
    // 🧾 FORMAT OFS
    // =============================================================
    void format() {
    if (!sessionManager.isLoggedIn()) {
        cerr << "❌ Access Denied: Please log in as an Admin to format the system.\n";
        return;
    }
    if (!sessionManager.isAdmin()) {
        cerr << "🛑 Access Denied: Only Admins can format the file system.\n";
        return;
    }

    cout << "🧹 Formatting OFS...\n";
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

    fileManager.closeFile();
    isInitialized = true;
    cout << "✅ OFS formatted successfully by Admin.\n";
    updateStats();
}

    // =============================================================
    // 📂 LOAD EXISTING SYSTEM
    // =============================================================
    bool loadSystem() {
        cout << "\nLoading OFS from " << omniFileName << "...\n";
        if (!fileManager.openFile(omniFileName, 4096)) {
            cerr << "❌ Error: Could not open .omni file.\n";
            return false;
        }

        OMNIHeader tmp;
        if (!fileManager.readHeader(tmp)) return false;
        header = tmp;

        if (strcmp(header.magic, "OMNIFS01") != 0) {
            cerr << "❌ Corrupted or invalid .omni file.\n";
            fileManager.closeFile();
            return false;
        }

        cout << "✅ Loaded OMNI file successfully.\n";
        cout << "Magic: " << header.magic << "\nTotal Size: " << header.total_size
             << "\nVersion Offset: " << header.file_state_storage_offset
             << "\nChangeLog Offset: " << header.change_log_offset << endl;

        fileManager.closeFile();
        isInitialized = true;
        updateStats();
        return true;
    }

    // =============================================================
    // ✏️ WRITE FILE + CREATE VERSION ENTRY
    // =============================================================
    bool writeFileContent(const string& filePath, const string& fileData) {
    if (!sessionManager.isLoggedIn()) {
        cerr << "❌ Access Denied: You must be logged in to write files.\n";
        return false;
    }

    cout << "✏️ Writing file content as user: " << sessionManager.getActiveUsername() << endl;

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
    sessionManager.recordOperation();

    fileManager.closeFile();
    cout << "✅ File stored successfully by user: " << sessionManager.getActiveUsername() << "\n";
    return true;
}

    // =============================================================
    // 📖 READ FILE DATA
    // =============================================================
    bool readFileContent(uint32_t blockIndex, uint32_t dataLength) {
    if (!sessionManager.isLoggedIn()) {
        cerr << "❌ Access Denied: You must be logged in to read files.\n";
        return false;
    }

    cout << "📖 Reading data as user: " << sessionManager.getActiveUsername() << endl;

    fileManager.openFile(omniFileName, 4096);

    vector<char> buffer;
    fileManager.readFileData(dataStartOffset, blockIndex, 4096, buffer);
    fileManager.closeFile();

    string content(buffer.begin(), buffer.begin() + dataLength);
    cout << "📄 File content:\n" << content << endl;

    sessionManager.recordOperation();
    return true;
}

    // =============================================================
    // 🪶 CHANGE LOG SYSTEM
    // =============================================================
    void logChange(const string& path, const string& user, const string& action, uint64_t versionID) {
        fileManager.openFile(omniFileName, 4096);
        ChangeLogEntry entry{};
        strncpy(entry.filePath, path.c_str(), sizeof(entry.filePath) - 1);
        strncpy(entry.user, user.c_str(), sizeof(entry.user) - 1);
        strncpy(entry.action, action.c_str(), sizeof(entry.action) - 1);
        entry.timestamp = time(nullptr);
        entry.versionID = versionID;

        fileManager.writeChangeLog({entry}, header.change_log_offset);
        fileManager.closeFile();
    }

    void showChangeLog() {
        vector<ChangeLogEntry> log;
        fileManager.openFile(omniFileName, 4096);
        fileManager.readChangeLog(log, header.change_log_offset, 10);
        fileManager.closeFile();

        cout << "\n--- Change Log ---\n";
        for (auto& e : log)
            if (strlen(e.filePath) > 0)
                cout << e.filePath << " | " << e.action
                     << " | " << e.user << " | v" << e.versionID
                     << " | " << ctime((time_t*)&e.timestamp);
    }

    // =============================================================
    // 🧾 SAVE NEW FILE VERSION
    // =============================================================
    void saveFileVersion(const string& path, uint32_t blockIndex) {
        fileManager.openFile(omniFileName, 4096);

        OMNIHeader currentHeader;
        fileManager.readHeader(currentHeader);

        if (strcmp(currentHeader.magic, "OMNIFS01") != 0) {
            cerr << "❌ Header corrupted during version save!\n";
            fileManager.closeFile();
            return;
        }

        vector<VersionBlock> existingVersions;
        fileManager.readAllVersions(existingVersions, currentHeader.file_state_storage_offset);

        uint64_t versionOffset = currentHeader.file_state_storage_offset +
                                 (existingVersions.size() * sizeof(VersionBlock));

        VersionBlock vb{};
        strncpy(vb.filePath, path.c_str(), sizeof(vb.filePath) - 1);
        vb.versionID = static_cast<uint64_t>(time(nullptr));
        vb.startBlock = blockIndex;
        vb.blockCount = 1;
        vb.timestamp = vb.versionID;

        fileManager.writeVersionBlock(vb, versionOffset);
        fileManager.closeFile();

        cout << "✅ Saved version for " << path
             << " at offset " << versionOffset
             << " (vID " << vb.versionID << ")\n";
    }

    // =============================================================
    // 📜 LIST VERSIONS
    // =============================================================
    void listVersions() {
        fileManager.openFile(omniFileName, 4096);
        vector<VersionBlock> versions;
        fileManager.readAllVersions(versions, header.file_state_storage_offset);
        fileManager.closeFile();

        cout << "\n--- Available Versions (" << versions.size() << ") ---\n";
        if (versions.empty()) {
            cout << "No versions found.\n";
            return;
        }

        for (auto& v : versions)
            cout << v.filePath << " | VersionID: " << v.versionID
                 << " | Block: " << v.startBlock
                 << " | Time: " << ctime((time_t*)&v.timestamp);
    }

    // =============================================================
    // 🔁 REVERT TO OLD VERSION
    // =============================================================
    void revertToVersion(uint64_t versionID) {
        vector<VersionBlock> versions;
        fileManager.openFile(omniFileName, 4096);
        fileManager.readAllVersions(versions, header.file_state_storage_offset);
        fileManager.closeFile();

        for (auto& v : versions) {
            if (v.versionID == versionID) {
                cout << "\nRestoring version " << versionID << " of " << v.filePath << "...\n";
                readFileContent(v.startBlock, 4096);
                cout << "✅ Version restored.\n";
                return;
            }
        }
        cout << "❌ Version ID not found.\n";
    }



    void loginUser(const string& user, const string& pass) { sessionManager.login(user, pass); }
    void logoutUser() { sessionManager.logout(); }
    void showSession() const { sessionManager.printSession(); }

    void createUser(const string& username, const string& password, bool isAdminFlag) {
        if (!sessionManager.isLoggedIn()) {
            cerr << "❌ Access Denied: You must be logged in as Admin to add new users.\n";
            return;
        }
        if (!sessionManager.isAdmin()) {
            cerr << "🛑 Access Denied: Only Admins can add new users.\n";
            return;
        }

        if (userManager.addUser(username, password, isAdminFlag))
            cout << "✅ User '" << username << "' added successfully.\n";
        else
            cout << "⚠️ Could not add user (maybe already exists?).\n";
    }

    UserManager& getUserManager() { return userManager; }



};
