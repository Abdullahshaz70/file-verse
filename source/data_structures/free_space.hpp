#include<iostream>
#include<vector>

using namespace std;

class FreeSpace{

    vector<bool> memory;
    int totalBlocks;

public:
    FreeSpace(int _totalBlocks):totalBlocks(_totalBlocks) , memory(totalBlocks , true){}
    ~FreeSpace(){reset();}
    int allocateBlock(){
        for (int i = 0; i < totalBlocks; ++i) {
        if (memory[i]) {
            memory[i] = false; 
            return i;            
        }
    }
    std::cerr << "No free blocks available!\n";
    return -1;
    }
    
    void freeBlock(int index){
        if(index<0 || index> totalBlocks){
            cout << "Invalid block index!\n";
            return;
        }

        memory[index] = true;
    }
    
    int getFreeCount() const{
        int count = 0;
        for(int i = 0 ; i < totalBlocks ; i++)
            if(memory[i]==true)
                count++;
        return count;
    }
    
    void print() const{
        // for(int i = 0  ; i < totalBlocks ; i++)
        //     cout<<memory[i]<<" ";

        cout << "\nFree Space Map:\n";
        for (int i = 0; i < totalBlocks; ++i) {
        cout << (memory[i] ? "0" : "1");
        if ((i + 1) % 10 == 0) cout << " "; 
        }
        cout << "\n(0 = free, 1 = used)\n";
    }
    
    void reset(){
        for(int i = 0 ; i < totalBlocks ; i++)
            memory[i] = true;
    }


};

