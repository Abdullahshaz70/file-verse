#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "OFSCore.hpp"

using namespace std;

class FileIOManager {
    fstream file;
    string fileName;
    uint64_t blockSize;

public:
    FileIOManager() = default;

    bool createOmniFile(const string& name, uint64_t totalSize, uint64_t blockSize) {
        fileName = name;
        this->blockSize = blockSize;

        file.open(fileName, ios::out | ios::binary | ios::trunc);
        if (!file.is_open()) {
            cout << "Error: Could not create " << fileName << endl;
            return false;
        }

        vector<char> zeros(1024, 0);
        uint64_t written = 0;
        while (written < totalSize) {
            file.write(zeros.data(), zeros.size());
            written += zeros.size();
        }

        file.close();

        cout << "Created .omni file: " << fileName
             << " (" << totalSize / 1024 << " KB)" << endl;
        return true;
    }

    bool openFile(const string& name, uint64_t blockSz) {
        fileName = name;
        blockSize = blockSz;
        file.open(fileName, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            cerr << "Error: Could not open " << fileName << endl;
            return false;
        }
        return true;
    }

    bool writeHeader(const OMNIHeader& header) {
        if (!file.is_open()) {
            cout << "File not open!" << endl;
            return false;
        }

        file.seekp(0, ios::beg);
        file.write(reinterpret_cast<const char*>(&header), sizeof(OMNIHeader));
        file.flush();
        cout << "Header written successfully.\n";
        return true;
    }

    bool readHeader(OMNIHeader& outHeader) {
        if (!file.is_open()) {
            cerr << "File not open.\n";
            return false;
        }
        file.seekg(0, ios::beg);
        file.read(reinterpret_cast<char*>(&outHeader), sizeof(OMNIHeader));
        cout << "Header read successfully.\n";
        return true;
    }

    bool writeBlock(uint32_t blockIndex, const vector<char>& data) {
        if (!file.is_open()) {
            cerr << "File not open.\n";
            return false;
        }
        file.seekp(sizeof(OMNIHeader) + blockIndex * blockSize, ios::beg);
        file.write(data.data(), min<uint64_t>(data.size(), blockSize));
        file.flush();
        return true;
    }

    bool readBlock(uint32_t blockIndex, vector<char>& data) {
        if (!file.is_open()) {
            cerr << "File not open.\n";
            return false;
        }
        data.resize(blockSize);
        file.seekg(sizeof(OMNIHeader) + blockIndex * blockSize, ios::beg);
        file.read(data.data(), blockSize);
        return true;
    }

    void closeFile() {
        if (file.is_open()) {
            file.close();
            cout << "Closed .omni file: " << fileName << endl;
        }
    }

    bool writeUsers(const vector<UserInfo>& users, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (const auto& user : users)
            file.write(reinterpret_cast<const char*>(&user), sizeof(UserInfo));
        file.flush();
        cout << "User table written successfully.\n";
        return true;
    }

    bool readUsers(vector<UserInfo>& users, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        users.resize(count);
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i)
            file.read(reinterpret_cast<char*>(&users[i]), sizeof(UserInfo));
        cout << "User table read successfully.\n";
        return true;
    }

    bool writeFreeMap(const vector<bool>& freeMap, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (bool bit : freeMap) {
            char value = bit ? 1 : 0;
            file.write(&value, 1);
        }
        file.flush();
        cout << "Free space map written.\n";
        return true;
    }

    bool readFreeMap(vector<bool>& freeMap, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        freeMap.clear();
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i) {
            char value;
            file.read(&value, 1);
            freeMap.push_back(value);
        }
        cout << "Free space map read.\n";
        return true;
    }


    bool writeFileEntries(const vector<FileEntry>& entries, uint64_t offset) {
        if (!file.is_open()) return false;
        file.seekp(offset, ios::beg);
        for (const auto& e : entries)
            file.write(reinterpret_cast<const char*>(&e), sizeof(FileEntry));
        file.flush();
        cout << "Directory metadata written successfully.\n";
        return true;
    }      

    bool readFileEntries(vector<FileEntry>& entries, uint64_t offset, uint32_t count) {
        if (!file.is_open()) return false;
        entries.resize(count);
        file.seekg(offset, ios::beg);
        for (uint32_t i = 0; i < count; ++i)
            file.read(reinterpret_cast<char*>(&entries[i]), sizeof(FileEntry));
        cout << "Directory metadata read successfully.\n";
        return true;
    }
    
    bool writeFileData(uint64_t dataRegionOffset, uint32_t blockIndex, uint64_t blockSize, const vector<char>& data) {
        if (!file.is_open()) {
            cerr << "Error: .omni file not open for write.\n";
            return false;
        }


        file.seekp(dataRegionOffset + (blockIndex * blockSize), ios::beg);
        file.write(data.data(), min<uint64_t>(data.size(), blockSize));
        file.flush();
        cout << "Wrote " << data.size() << " bytes to block #" << blockIndex << endl;
        return true;
    }

    bool readFileData(uint64_t dataRegionOffset, uint32_t blockIndex, uint64_t blockSize, vector<char>& outData) {
        if (!file.is_open()) {
            cerr << "Error: .omni file not open for read.\n";
            return false;
        }

        outData.resize(blockSize);
        file.seekg(dataRegionOffset + (blockIndex * blockSize), ios::beg);
        file.read(outData.data(), blockSize);
        cout << "Read block #" << blockIndex << " from .omni file.\n";
        return true;
    }



};
