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
private:
    UserManager* userManager;
    DirectoryTree dirTree;
    FreeSpace spaceManager;
    FileIOManager fileManager;

    OMNIHeader header{};
    FSStats stats{};
    uint64_t totalBlocks = 2048;
    uint64_t dataStartOffset = 0;
    bool isInitialized = false;
    string omniFileName = "filesystem.omni";

    vector<UserInfo> userTable;
    uint64_t userTableOffset = sizeof(OMNIHeader);
    SessionManager* session = nullptr;

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
    OFSCore(UserManager* um, int blocks = 2048)
        : userManager(um), spaceManager(blocks),
          totalBlocks(blocks), isInitialized(false), session(nullptr)
    {
        uint64_t blockSize = 4096;
        uint64_t totalSize = blocks * blockSize;
        header = OMNIHeader(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
        stats = FSStats(totalSize, 0, totalSize);

        userManager->addUser("admin", "admin123", true);
        cout << "Default admin (admin / admin123) created.\n";
        cout << "OFSCore initialized with " << blocks
             << " blocks (" << totalSize / 1024 << " KB)." << endl;
    }

    ~OFSCore() { cout << "OFSCore shutting down." << endl; }
    void attachSession(SessionManager* s) { session = s; }

    // ==============================
    // 🧾 FORMAT
    // ==============================
    void format() {
        if (!session || !session->isActive() || !session->isAdminUser()) {
            cerr << "❌ Access Denied: Please log in as an Admin to format the system.\n";
            return;
        }

        cout << "🧹 Formatting OFS...\n";
        spaceManager.reset();
        dirTree.reset();

        const uint64_t blockSize = 4096;
        const uint64_t totalSize = totalBlocks * blockSize;

        fileManager.createOmniFile(omniFileName, totalSize, blockSize);
        fileManager.openFile(omniFileName, blockSize);

        memset(&header, 0, sizeof(header));
        strncpy(header.magic, "OMNIFS01", sizeof(header.magic));
        header.format_version = 0x00010000;
        header.total_size = totalSize;
        header.header_size = sizeof(OMNIHeader);
        header.block_size = blockSize;
        strncpy(header.student_id, "2022-CS-7062", sizeof(header.student_id) - 1);
        strncpy(header.submission_date, "2025-11-09", sizeof(header.submission_date) - 1);

        fileManager.writeHeader(header);

        const uint64_t userTableOffset = sizeof(OMNIHeader);
        vector<UserInfo> emptyUsers(10);
        fileManager.writeUsers(emptyUsers, userTableOffset);

        const uint64_t freeMapOffset = userTableOffset + (emptyUsers.size() * sizeof(UserInfo));
        vector<bool> freeMap(totalBlocks, false);
        fileManager.writeFreeMap(freeMap, freeMapOffset);

        vector<FileEntry> entries;
        dirTree.exportToEntries(entries);
        const uint64_t metaOffset = freeMapOffset + freeMap.size();
        fileManager.writeFileEntries(entries, metaOffset);

        dataStartOffset = metaOffset + (entries.size() * sizeof(FileEntry));
        const uint64_t remaining = (totalSize > dataStartOffset ? totalSize - dataStartOffset : 0);
        const uint64_t dataRegionSize = static_cast<uint64_t>(remaining * 9 / 10);
        uint64_t versionStart = dataStartOffset + dataRegionSize;
        if (versionStart > totalSize) versionStart = totalSize;

        const uint64_t maxVBsBySpace =
            (versionStart < totalSize ? (totalSize - versionStart) / sizeof(VersionBlock) : 0);
        const uint64_t maxVBs = min<uint64_t>(256, maxVBsBySpace);
        const uint64_t versionBytes = maxVBs * sizeof(VersionBlock);

        uint64_t changeLogOffset = versionStart + versionBytes;
        if (changeLogOffset > totalSize) changeLogOffset = totalSize;

        header.user_table_offset = static_cast<uint32_t>(userTableOffset);
        header.max_users = 10;
        header.file_state_storage_offset = static_cast<uint32_t>(versionStart);
        header.change_log_offset = static_cast<uint32_t>(changeLogOffset);

        cout << "🧭 DEBUG OFFSETS:\n";
        cout << "Header start          : 0\n";
        cout << "UserTable start       : " << userTableOffset << "\n";
        cout << "FreeMap start         : " << freeMapOffset << "\n";
        cout << "Metadata start        : " << metaOffset << "\n";
        cout << "Data start offset     : " << dataStartOffset << "\n";
        cout << "Version storage offset: " << header.file_state_storage_offset << "\n";
        cout << "Change log offset     : " << header.change_log_offset << "\n";

        fileManager.seekToStart();
        fileManager.writeHeader(header);

        userTable.assign(10, UserInfo());
        strncpy(userTable[0].username, "admin", sizeof(userTable[0].username) - 1);
        strncpy(userTable[0].password_hash, "admin123", sizeof(userTable[0].password_hash) - 1);
        userTable[0].role = UserRole::ADMIN;
        userTable[0].created_time = time(nullptr);
        userTable[0].is_active = 1;
        fileManager.saveUsers(userTable, userTableOffset);

        fileManager.closeFile();
        isInitialized = true;
        cout << "✅ OFS formatted successfully by Admin.\n";
        updateStats();
    }

    // ==============================
    // ✏️ WRITE FILE (fixed path)
    // ==============================
  bool writeFileContent(const string& filePath, const string& fileData) {
    if (!session || !session->isLoggedIn()) {
        cerr << "❌ Access Denied: You must be logged in to write files.\n";
        return false;
    }

    string username = session->getCurrentUser();
    string userDir = "/home/" + username;
    string actualPath = userDir + filePath;  // ✅ no extra /Documents now

    cout << "✏️ Writing file content as user: " << username << endl;
    cout << "📁 Target path: " << actualPath << endl;

    if (!fileManager.openFile(omniFileName, 4096)) {
        cerr << "❌ Could not open .omni for write.\n";
        return false;
    }

    const uint64_t blockSize = 4096;

    int blockIndex = spaceManager.allocateBlock();
    if (blockIndex == -1) {
        cerr << "❌ No free space available.\n";
        fileManager.closeFile();
        return false;
    }

    vector<char> buffer(fileData.begin(), fileData.end());
    if (!fileManager.writeFileData(dataStartOffset, blockIndex, blockSize, buffer)) {
        cerr << "❌ Failed to write file data.\n";
        fileManager.closeFile();
        return false;
    }

    cout << "💾 Wrote " << buffer.size() << " bytes to block #" << blockIndex
         << " (offset " << (dataStartOffset + static_cast<uint64_t>(blockIndex) * blockSize) << ")\n";

    // Update free map
    vector<bool> freeMap = spaceManager.getMap();
    const uint64_t freeMapOffset = sizeof(OMNIHeader) + (10 * sizeof(UserInfo));
    fileManager.writeFreeMap(freeMap, freeMapOffset);

    updateStats();
    session->recordOperation();

    // Record a version
    saveFileVersion(filePath, blockIndex);

    fileManager.closeFile();
    cout << "✅ File stored successfully by user: " << username << "\n";
    return true;
}

        // ==============================
    // 📂 LOAD EXISTING SYSTEM
    // ==============================
    bool loadSystem() {
        cout << "\nLoading OFS from " << omniFileName << "...\n";
        if (!fileManager.openFile(omniFileName, 4096)) {
            cerr << "❌ Error: Could not open .omni file.\n";
            return false;
        }

        OMNIHeader tmp;



        if (!fileManager.readHeader(tmp)) {
            fileManager.closeFile();
            return false;
        }
        header = tmp;

        if (strcmp(header.magic, "OMNIFS01") != 0) {
            cerr << "❌ Corrupted or invalid .omni file.\n";
            fileManager.closeFile();
            return false;
        }

        if (!fileManager.loadUsers(userTable, userTableOffset, 10)) {
            cerr << "⚠️ Warning: Could not load users.\n";
        } else {
            for (const auto& u : userTable) {
                if (strlen(u.username) > 0)
                    userManager->addUser(u.username, u.password_hash,
                                        u.role == UserRole::ADMIN);
            }
        }

        fileManager.closeFile();
        isInitialized = true;
        updateStats();
        return true;


    }


   // =============================================================
// 📖 READ FILE CONTENT (prints actual file text)
// =============================================================
bool readFileContent(uint32_t blockIndex, uint32_t dataLength = 256) {
    if (!session || !session->isLoggedIn()) {
        cerr << "❌ Access Denied: You must be logged in to read files.\n";
        return false;
    }

    cout << "📖 Reading data as user: " << session->getCurrentUser() << endl;

    if (!fileManager.openFile(omniFileName, 4096)) {
        cerr << "❌ Could not open .omni file.\n";
        return false;
    }

    vector<char> buffer;
    fileManager.readFileData(dataStartOffset, blockIndex, 4096, buffer);
    fileManager.closeFile();

    // Trim trailing nulls
    string content(buffer.data(), buffer.data() + strnlen(buffer.data(), buffer.size()));

    cout << "\n📄 === File Content (Block #" << blockIndex << ") ===\n";
    cout << content << "\n";
    cout << "============================================\n";

    session->recordOperation();
    return true;
}

        // ==============================
    // 👥 USER CREATION
    // ==============================
    void createUser(const string& username, const string& password, bool isAdmin) {
        if (!session || !session->isAdminUser()) {
            cerr << "❌ Access Denied: Only Admin can create new users.\n";
            return;
        }

        if (userManager->addUser(username, password, isAdmin)) {
            cout << "✅ User '" << username << "' created successfully.\n";

            for (auto& u : userTable) {
                if (!u.is_active) {
                    strncpy(u.username, username.c_str(), sizeof(u.username) - 1);
                    strncpy(u.password_hash, password.c_str(), sizeof(u.password_hash) - 1);
                    u.role = isAdmin ? UserRole::ADMIN : UserRole::NORMAL;
                    u.created_time = time(nullptr);
                    u.is_active = 1;
                    break;
                }
            }

            fileManager.openFile(omniFileName, 4096);
            fileManager.saveUsers(userTable, userTableOffset);

            dirTree.createUserHome(username);
            cout << "🏠 Home directory created for user: /home/" << username << "\n";



            fileManager.closeFile();
        }
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



// =============================================================
// 🧾 SAVE NEW FILE VERSION  (fixed header read issue)
// =============================================================
void saveFileVersion(const string& path, uint32_t blockIndex) {
    // Open file cleanly for version append
    if (!fileManager.openFile(omniFileName, 4096)) {
        cerr << "❌ Could not open .omni to save version.\n";
        return;
    }

    OMNIHeader current{};
    
    // ✅ Read header directly from start (without reopening or clearing stream)
    fileManager.seekToStart();
    fileManager.File().read(reinterpret_cast<char*>(&current), sizeof(OMNIHeader));
    if (strncmp(current.magic, "OMNIFS01", 8) != 0) {
        cerr << "❌ Header read failed — invalid magic!\n";
        fileManager.closeFile();
        return;
    }

    // --- Load existing versions safely ---
    vector<VersionBlock> existing;
    fileManager.readAllVersions(existing, current.file_state_storage_offset);

    uint64_t versionOffset =
        current.file_state_storage_offset + (existing.size() * sizeof(VersionBlock));

    // --- Prepare version record ---
    VersionBlock vb{};
    memset(&vb, 0, sizeof(vb));
    strncpy(vb.filePath, path.c_str(), sizeof(vb.filePath) - 1);
    vb.versionID  = static_cast<uint64_t>(time(nullptr));
    vb.startBlock = blockIndex;
    vb.blockCount = 1;
    vb.timestamp  = vb.versionID;

    // --- Write version record ---
    if (!fileManager.writeVersionBlock(vb, versionOffset)) {
        cerr << "❌ Failed to write version block.\n";
        fileManager.closeFile();
        return;
    }

    fileManager.closeFile();
    cout << "✅ Saved version for " << path
         << " at offset " << versionOffset
         << " (vID " << vb.versionID << ")\n";
}

    // ==============================
    // Session Helpers
    // ==============================
    void loginUser(const string& user, const string& pass) {
        if (session) session->login(user, pass);
    }

    void logoutUser() {
        if (session) session->logout();
    }

    void showSession() const {
        if (session) session->printSession();
    }

    UserManager& getUserManager() { return *userManager; }


    // =============================================================
    // 🧩 VERIFY FILE STRUCTURE
    // =============================================================
    void verifyFileStructure() {
        cout << "🧩 Verifying file structure...\n";

        if (!fileManager.openFile(omniFileName, 4096)) {
            cerr << "❌ Could not open .omni file for verification.\n";
            return;
        }

        OMNIHeader verify{};
        if (!fileManager.readHeader(verify)) {
            cerr << "❌ Header could not be read — corrupted file.\n";
            fileManager.closeFile();
            return;
        }

        // --- Header Validation ---
        bool headerOK = strcmp(verify.magic, "OMNIFS01") == 0;
        bool versionOK = verify.format_version == 0x00010000;
        bool blockSizeOK = verify.block_size == 4096;
        bool totalSizeOK = verify.total_size > 0;

        cout << "\n--- HEADER INFO ---\n";
        cout << "Magic: " << verify.magic << "\n";
        cout << "Format Version: 0x" << hex << verify.format_version << dec << "\n";
        cout << "Block Size: " << verify.block_size << "\n";
        cout << "Total Size: " << verify.total_size / 1024 << " KB\n";

        if (!headerOK || !versionOK || !blockSizeOK || !totalSizeOK) {
            cerr << "❌ Header verification failed.\n";
            fileManager.closeFile();
            return;
        }

        // --- Offsets Consistency ---
        cout << "\n--- REGION OFFSETS ---\n";
        cout << "User Table Offset: " << verify.user_table_offset << endl;
        cout << "Version Storage Offset: " << verify.file_state_storage_offset << endl;
        cout << "Change Log Offset: " << verify.change_log_offset << endl;

        if (verify.file_state_storage_offset <= verify.header_size) {
            cerr << "⚠️ Version storage overlaps header region!\n";
        }
        if (verify.change_log_offset <= verify.file_state_storage_offset) {
            cerr << "⚠️ Change log overlaps version region!\n";
        }

        // --- Users Verification ---
        vector<UserInfo> testUsers;
        if (fileManager.loadUsers(testUsers, sizeof(OMNIHeader), 10)) {
            cout << "\n--- USER TABLE (Active Entries) ---\n";
            for (const auto& u : testUsers)
                if (strlen(u.username) > 0)
                    cout << "👤 " << u.username << " | Role: "
                        << (u.role == UserRole::ADMIN ? "Admin" : "User")
                        << " | Active: " << (u.is_active ? "Yes" : "No") << endl;
        }

        // --- Versions Check ---
        vector<VersionBlock> versions;
        fileManager.readAllVersions(versions, verify.file_state_storage_offset);
        cout << "\n--- VERSION STORAGE ---\n";
        cout << "Total Versions: " << versions.size() << endl;

        // --- Log Check ---
        vector<ChangeLogEntry> log;
        fileManager.readChangeLog(log, verify.change_log_offset, 10);
        cout << "\n--- CHANGE LOG ENTRIES ---\n";
        for (auto& e : log)
            if (strlen(e.filePath) > 0)
                cout << e.filePath << " | " << e.user << " | " << e.action
                    << " | v" << e.versionID << " | " << ctime((time_t*)&e.timestamp);

        fileManager.closeFile();
        cout << "\n✅ File structure verification complete.\n";
    }


    // ============================================
// 🧭 Directory Viewing Functions
// ============================================
void listMyFiles() {
    if (!session || !session->isLoggedIn()) {
        cerr << "❌ Login required to list your files.\n";
        return;
    }
    dirTree.listUserFiles(session->getCurrentUser());
}

void listAllFiles() {
    if (!session || !session->isAdminUser()) {
        cerr << "❌ Admin access required to view all files.\n";
        return;
    }
    dirTree.listAll();
}


// =============================================================
// 🏗️ Create a new directory for current user
// =============================================================
void createDirectory(const string& path) {
    if (!session || !session->isLoggedIn()) {
        cerr << "❌ Login required to create directory.\n";
        return;
    }

    string username = session->getCurrentUser();
    string cleanPath = (path[0] == '/') ? path.substr(1) : path;
    string base = "/home/" + username;
    string fullPath = base + "/" + cleanPath;

    if (dirTree.createDirectory(base, cleanPath)) {
        cout << "📁 Directory created at: " << fullPath << endl;
    } else {
        cerr << "⚠️ Failed to create directory at " << fullPath << endl;
    }
}

// =============================================================
// 📝 Create a new file and write content (custom path)
// =============================================================
void createFile(const string& relativePath, const string& content) {
    if (!session || !session->isLoggedIn()) {
        cerr << "❌ Login required to create file.\n";
        return;
    }

    string username = session->getCurrentUser();
    string cleanPath = (relativePath[0] == '/') ? relativePath.substr(1) : relativePath;
    string base = "/home/" + username;
    string fullPath = base + "/" + cleanPath;

    // Ensure directory exists before file write
    size_t lastSlash = cleanPath.find_last_of('/');
    if (lastSlash != string::npos) {
        string dirPart = cleanPath.substr(0, lastSlash);
        dirTree.createDirectory(base, dirPart);
    }

    // Write directly without adding "Documents" again
    if (writeFileContent("/" + cleanPath, content)) {
        cout << "✅ File created successfully at: " << fullPath << endl;
    } else {
        cerr << "❌ Failed to create file at: " << fullPath << endl;
    }
}

    // =============================================================
    // 🌳 Show current user's directory tree
    // =============================================================
    void showMyDirectoryTree() {
        if (!session || !session->isLoggedIn()) {
            cerr << "❌ Login required to view directory tree.\n";
            return;
        }
        cout << "\n📂 Directory structure for user: " << session->getCurrentUser() << endl;
        dirTree.listUserFiles(session->getCurrentUser());
    }


};

