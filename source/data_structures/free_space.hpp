#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class FreeSpace {
private:
    vector<bool> freeMap;
    int totalBlocks;

public:
    FreeSpace(int total = 256) : totalBlocks(total) {
        freeMap.assign(totalBlocks, false); 
    }

    void reset() {
        fill(freeMap.begin(), freeMap.end(), false); 
    }

    int allocateBlock() {
        for (int i = 0; i < totalBlocks; ++i) {
            if (!freeMap[i]) {
                freeMap[i] = true;
                return i;
            }
        }
        return -1; 
    }

    void freeBlock(int index) {
        if (index >= 0 && index < totalBlocks)
            freeMap[index] = false;
    }

    int getFreeCount() const {
        return count(freeMap.begin(), freeMap.end(), false);
    }

    vector<bool> getMap() const {
        return freeMap;
    }

    void setMap(const vector<bool>& map) {
        freeMap = map;
    }

    void print() const {
        cout << "\nFree Space Map:\n";
        for (int i = 0; i < totalBlocks; ++i) {
            cout << (freeMap[i] ? '1' : '0');
            if ((i + 1) % 10 == 0) cout << ' ';
        }
        cout << "\n(0 = free, 1 = used)\n";
    }
};
