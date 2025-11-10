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
};
