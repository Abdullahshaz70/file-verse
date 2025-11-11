// #include<iostream>
// #include<vector>

// using namespace std;

// class FreeSpace{

//     vector<bool> memory;
//     int totalBlocks;

// public:
//     FreeSpace(int totalBlocks)
//     : totalBlocks(totalBlocks), memory(totalBlocks, true) {}

//     ~FreeSpace(){
//         // cout << "[Debug] FreeSpaceManager destructor called\n";
//         reset();
//     }
//     int allocateBlock(){
//         for (int i = 0; i < totalBlocks; ++i) {
//         if (memory[i]) {
//             memory[i] = false; 
//             return i;            
//         }
//     }
//     std::cerr << "No free blocks available!\n";
//     return -1;
//     }
    
//     void freeBlock(int index){
//         if(index<0 || index> totalBlocks){
//             cout << "Invalid block index!\n";
//             return;
//         }

//         memory[index] = true;
//     }
    
//     int getFreeCount() const{
//         int count = 0;
//         for(int i = 0 ; i < totalBlocks ; i++)
//             if(memory[i]==true)
//                 count++;
//         return count;
//     }
    
//     void print() const{
//         // for(int i = 0  ; i < totalBlocks ; i++)
//         //     cout<<memory[i]<<" ";

//         cout << "\nFree Space Map:\n";
//         for (int i = 0; i < totalBlocks; ++i) {
//         cout << (memory[i] ? "0" : "1");
//         if ((i + 1) % 10 == 0) cout << " "; 
//         }
//         cout << "\n(0 = free, 1 = used)\n";
//     }
    
//     void reset(){
//         for(int i = 0 ; i < totalBlocks ; i++)
//             memory[i] = true;
//     }

//     vector<bool> getMap() const {
//         return memory;
//     }

//     void setMap(const vector<bool>& newMap) {
//         memory = newMap;
//     }

// };


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
