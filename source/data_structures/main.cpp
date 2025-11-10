#include<iostream>
#include "../include/core/OFSCore.hpp"
#include "../include/core/file_io_manager.hpp"


using namespace std;

int main1() {
    UserManager um;
    um.addUser("Ali", "123", false);
    um.addUser("Zara", "456", true);
    um.addUser("Ahmed", "789", false);

    cout << "\nAll Users:\n";
    um.print();

    cout << "\nAuthenticate Zara: "
         << (um.authenticate("Zara", "456") ? "Success" : "Fail") << endl;

    um.removeUser("Ali");
    cout << "\nAfter removing Ali:\n";
    um.print();

    return 0;
}

int main2() {
    DirectoryTree dt;

    
    dt.createDirectory("/", "Documents");
    dt.createDirectory("/Documents", "Assignments");
    dt.createDirectory("/Documents", "Notes");

    dt.createFile("/Documents", "todo.txt", "Finish OFS project!");
    dt.createFile("/Documents/Assignments", "lab1.txt", "Lab 1: AVL Trees");
    dt.createFile("/Documents/Assignments", "lab2.txt", "Lab 2: File System");

    cout << "\nListing root directory:\n";
    dt.list("/");

    cout << "\nListing Documents directory:\n";
    dt.list("/Documents");

    cout << "\nListing Assignments directory:\n";
    dt.list("/Documents/Assignments");


    cout << "\nDeleting lab1.txt...\n";
    dt.deleteNode("/Documents/Assignments/lab1.txt");

    cout << "\nAfter deletion:\n";
    dt.list("/Documents/Assignments");

    dt.createDirectory("/Documents", "Images");
    dt.createFile("/Documents/Images", "logo.png", "PNG DATA");

    cout << "\nListing Documents after adding Images:\n";
    dt.list("/Documents");

    return 0;
}

int main3() {
    FreeSpace fsm(20); 

    cout << "Initial free count: " << fsm.getFreeCount() << "\n";
    fsm.print();

    int b1 = fsm.allocateBlock();
    int b2 = fsm.allocateBlock();
    int b3 = fsm.allocateBlock();

    cout << "\nAllocated blocks: " << b1 << ", " << b2 << ", " << b3 << "\n";
    cout << "Free count: " << fsm.getFreeCount() << "\n";
    fsm.print();

    fsm.freeBlock(b2);
    cout << "\nAfter freeing block " << b2 << ":\n";
    fsm.print();

    fsm.reset();
    cout << "\nAfter format (all free again):\n";
    fsm.print();

    return 0;
}

int main4() {
    cout << "Starting program..." << endl;
    OFSCore ofs(20);

    cout << "Calling format..." << endl;
    ofs.format();

    cout << "Creating user..." << endl;
    ofs.createUser("admin", "123", true);

    cout << "Logging in..." << endl;
    ofs.login("admin", "123");

    cout << "Creating file..." << endl;
    ofs.createFile("/", "readme.txt", "Welcome to OFS!");

    cout << "Listing directory..." << endl;
    ofs.listDirectory("/");

    cout << "All done!" << endl;

    return 0;
}



int main5() {
    cout << "Starting FileIOManager test..." << endl;

    const uint64_t totalSize = 1024 * 1024;
    const uint64_t blockSize = 4096;

    FileIOManager io;

    if (!io.createOmniFile("filesystem.omni", totalSize, blockSize)) {
        cerr << "Failed to create .omni file!" << endl;
        return 1;
    }

    if (!io.openFile("filesystem.omni", blockSize)) {
        cerr << "Failed to open .omni file!" << endl;
        return 1;
    }

    OMNIHeader header(0x00010000, totalSize, sizeof(OMNIHeader), blockSize);
    strcpy(header.magic, "OMNIFS01");
    strcpy(header.student_id, "2022-CS-7062");
    strcpy(header.submission_date, "2025-11-09");

    io.writeHeader(header);

    OMNIHeader readHeader;
    io.readHeader(readHeader);

    cout << "\n--- Header Verification ---\n";
    cout << "Magic: " << readHeader.magic << endl;
    cout << "Format Version: " << readHeader.format_version << endl;
    cout << "Total Size: " << readHeader.total_size << " bytes" << endl;
    cout << "Block Size: " << readHeader.block_size << " bytes" << endl;
    cout << "Student ID: " << readHeader.student_id << endl;
    cout << "Submission Date: " << readHeader.submission_date << endl;
    cout << "---------------------------\n";

    io.closeFile();

    cout << "\nFileIOManager test completed successfully!\n";
    return 0;
}

int main6(){
    FileIOManager io;
    io.createOmniFile("filesystem.omni", 1024*1024, 4096);
    io.openFile("filesystem.omni", 4096);

    OMNIHeader header(0x00010000, 1024*1024, sizeof(OMNIHeader), 4096);
    strcpy(header.magic, "OMNIFS01");
    strcpy(header.student_id, "2022-CS-7062");
    strcpy(header.submission_date, "2025-11-09");

    io.writeHeader(header);
    OMNIHeader verify;
    io.readHeader(verify);
    io.closeFile();

    return 0;

}

int main7() {
    cout << "Starting OFSCore persistent test...\n";

    OFSCore ofs(20);

    int choice;
    cout << "1. Format new OFS\n";
    cout << "2. Load existing OFS\n";
    cout << "Enter choice: ";
    cin >> choice;

    if (choice == 1) {
        ofs.format();   
    } else {
        ofs.loadSystem();  
    }

    cout << "\nOperation completed successfully.\n";
    return 0;
}




int main() {
    cout << "1. Format new OFS\n";
    cout << "2. Load existing OFS\n";
    int c; cin >> c;
    OFSCore ofs(20);
    if (c == 1) ofs.format();
    else ofs.loadSystem();
}


