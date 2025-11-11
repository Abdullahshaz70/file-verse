#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "odf_types.hpp"

using namespace std;

class FileIOManager {
    fstream file;
    string fileName;
    uint64_t blockSize;

public:
    FileIOManager() = default;

    // =====================================================
    //  Create new .omni file
    // =====================================================
    bool createOmniFile(const string& name, uint64_t totalSize, uint64_t blockSize) {
        fileName = name;
        this->blockSize = blockSize;

        file.open(fileName, ios::out | ios::binary | ios::trunc);
        if (!file.is_open()) {
            cerr << "❌ Error: Could not create " << fileName << endl;
            return false;
        }

        vector<char> zeros(1024, 0);
        uint64_t written = 0;
        while (written < totalSize) {
            file.write(zeros.data(), zeros.size());
            written += zeros.size();
        }

        file.close();

        cout << "✅ Created .omni file: " << fileName
             << " (" << totalSize / 1024 << " KB)" << endl;
        return true;
    }

    // =====================================================
    //  Open file for read/write
    // =====================================================
    bool openFile(const string& name, uint64_t blockSz) {
        fileName = name;
        blockSize = blockSz;
        file.open(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "❌ Error: Could not open " << fileName << endl;
            return false;
        }
        return true;
    }

    // =====================================================
    //  Write header safely (always from start)
    // =====================================================
    bool writeHeader(const OMNIHeader& header) {
        if (!file.is_open()) {
            cerr << "❌ File not open for writing header!" << endl;
            return false;
        }

        cout << "🧭 DEBUG Writing header:\n";
        cout << " - Magic: '" << header.magic << "'\n";
        cout << " - Total Size: " << header.total_size << "\n";
        cout << " - Version Offset: " << header.file_state_storage_offset << "\n";

        file.seekp(0, ios::beg);
        file.write(reinterpret_cast<const char*>(&header), sizeof(OMNIHeader));
        file.flush();

        // Verify header integrity after write
        OMNIHeader verify;
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&verify), sizeof(OMNIHeader));

        if (strcmp(verify.magic, "OMNIFS01") != 0) {
            cerr << "❌ ERROR: Header verification failed! Magic: '" << verify.magic << "'\n";
            return false;
        }

        cout << "✅ Header written and verified successfully.\n";
        return true;
    }

    // =====================================================
    //  Safe header read (always resets pointer to start)
    // =====================================================
    bool readHeader(OMNIHeader& outHeader) {
        // Always close existing state and reopen fresh for consistent read
        if (file.is_open()) file.close();
        file.open(fileName, ios::in | ios::binary);
        if (!file.is_open()) {
            cerr << "❌ Error: Could not open " << fileName << " for reading header.\n";
            return false;
        }

        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&outHeader), sizeof(OMNIHeader));

        if (file.gcount() != sizeof(OMNIHeader)) {
            cerr << "❌ Error: Incomplete header read.\n";
            file.close();
            return false;
        }

        if (strcmp(outHeader.magic, "OMNIFS01") != 0) {
            cerr << "❌ ERROR: Invalid header magic read ('" << outHeader.magic << "').\n";
            file.close();
            return false;
        }

        cout << "✅ Header read successfully (Verified).\n";
        file.close();
        return true;
    }

    // =====================================================
    //  Write and read data blocks
    // =====================================================
    bool writeFileData(uint64_t dataRegionOffset, uint32_t blockIndex, uint64_t blockSize, const vector<char>& data) {
        if (!file.is_open()) {
            cerr << "❌ Error: .omni file not open for write.\n";
            return false;
        }

        file.seekp(dataRegionOffset + (blockIndex * blockSize), ios::beg);
        file.write(data.data(), min<uint64_t>(data.size(), blockSize));
        file.flush();
        cout << "💾 Wrote " << data.size() << " bytes to block #" << blockIndex << endl;
        return true;
    }

    bool readFileData(uint64_t dataRegionOffset, uint32_t blockIndex, uint64_t blockSize, vector<char>& outData) {
        if (!file.is_open()) {
            cerr << "❌ Error: .omni file not open for read.\n";
            return false;
        }

        outData.resize(blockSize);
        file.seekg(dataRegionOffset + (blockIndex * blockSize), ios::beg);
        file.read(outData.data(), blockSize);
        cout << "📖 Read block #" << blockIndex << " from .omni file.\n";
        return true;
    }

    // =====================================================
    //  Write and read user table
    // =====================================================
    bool writeUsers(const vector<UserInfo>& users, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (const auto& u : users)
            file.write(reinterpret_cast<const char*>(&u), sizeof(UserInfo));
        file.flush();
        cout << "👤 User table written successfully.\n";
        return true;
    }

    bool readUsers(vector<UserInfo>& users, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        users.resize(count);
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i)
            file.read(reinterpret_cast<char*>(&users[i]), sizeof(UserInfo));
        cout << "👤 User table read successfully.\n";
        return true;
    }

    // =====================================================
    //  Write and read free map
    // =====================================================
    bool writeFreeMap(const vector<bool>& freeMap, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (bool bit : freeMap) {
            char val = bit ? 1 : 0;
            file.write(&val, 1);
        }
        file.flush();
        cout << "🧱 Free space map written.\n";
        return true;
    }

    bool readFreeMap(vector<bool>& freeMap, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        freeMap.clear();
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i) {
            char val;
            file.read(&val, 1);
            freeMap.push_back(val);
        }
        cout << "🧱 Free space map read.\n";
        return true;
    }

    // =====================================================
    //  Write and read directory metadata
    // =====================================================
    bool writeFileEntries(const vector<FileEntry>& entries, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (const auto& e : entries)
            file.write(reinterpret_cast<const char*>(&e), sizeof(FileEntry));
        file.flush();
        cout << "📂 Directory metadata written successfully.\n";
        return true;
    }

    bool readFileEntries(vector<FileEntry>& entries, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        entries.resize(count);
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i)
            file.read(reinterpret_cast<char*>(&entries[i]), sizeof(FileEntry));
        cout << "📂 Directory metadata read successfully.\n";
        return true;
    }

    // =====================================================
    //  Change Log I/O
    // =====================================================
    bool writeChangeLog(const vector<ChangeLogEntry>& log, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (const auto& e : log)
            file.write(reinterpret_cast<const char*>(&e), sizeof(ChangeLogEntry));
        file.flush();
        cout << "🪶 Change log written successfully.\n";
        return true;
    }

    bool readChangeLog(vector<ChangeLogEntry>& log, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        log.resize(count);
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i)
            file.read(reinterpret_cast<char*>(&log[i]), sizeof(ChangeLogEntry));
        cout << "🪶 Change log read successfully.\n";
        return true;
    }

    // =====================================================
    //  Version Block I/O (Versioning System)
    // =====================================================
    bool writeVersionBlock(const VersionBlock& vb, uint64_t offset) {
        if (!file.is_open()) {
            cerr << "❌ File not open for writing version block.\n";
            return false;
        }

        if (offset < sizeof(OMNIHeader)) {
            cerr << "❌ Invalid version block offset: " << offset << endl;
            return false;
        }

        file.seekp(offset, ios::beg);
        file.write(reinterpret_cast<const char*>(&vb), sizeof(VersionBlock));
        file.flush();

        cout << "🧾 Version block written successfully at offset "
             << offset << " (vID " << vb.versionID << ")\n";
        return true;
    }

    bool readAllVersions(vector<VersionBlock>& list, uint64_t offset) {
        if (!file.is_open()) return false;

        file.seekg(offset, ios::beg);
        VersionBlock vb{};
        list.clear();

        for (int i = 0; i < 256; ++i) {
            if (!file.read(reinterpret_cast<char*>(&vb), sizeof(VersionBlock)))
                break;
            if (strlen(vb.filePath) > 0)
                list.push_back(vb);
        }

        if (file.eof()) file.clear();

        cout << "🧾 Version blocks read successfully: " << list.size() << endl;
        return true;
    }


    // ================================================
    // 🔐 Persistent User Table Management
    // ================================================

    bool saveUsers(const vector<UserInfo>& users, uint64_t offset) {
        if (!file.is_open()) {
            cerr << "❌ Error: File not open while saving users.\n";
            return false;
        }

        file.seekp(offset, ios::beg);
        for (const auto& user : users) {
            file.write(reinterpret_cast<const char*>(&user), sizeof(UserInfo));
        }
        file.flush();
        cout << "✅ Saved " << users.size() << " users to .omni file.\n";
        return true;
    }

bool loadUsers(vector<UserInfo>& users, uint64_t offset, uint32_t count) {
    if (!file.is_open()) {
        // Auto-open if needed
        file.open(fileName, ios::in | ios::binary);
        if (!file.is_open()) {
            cerr << "❌ Error: Could not open file while loading users.\n";
            return false;
        }
    }

    users.resize(count);
    file.seekg(offset, ios::beg);
    for (uint32_t i = 0; i < count; ++i)
        file.read(reinterpret_cast<char*>(&users[i]), sizeof(UserInfo));

    cout << "✅ Loaded " << count << " users from .omni file.\n";
    return true;
}




    // =====================================================
    //  Close file cleanly
    // =====================================================
    void closeFile() {
        if (file.is_open()) {
            file.close();
            cout << "🧹 Closed .omni file: " << fileName << endl;
        }
    }
};
