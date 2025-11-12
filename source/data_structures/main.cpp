#include<iostream>

#include "../include/core/OFSCore.hpp"

using namespace std;


int main1() {
    UserManager um;
    SessionManager session(&um);
    OFSCore ofs(&um, 256);   // ✅ pass the same UserManager pointer
    ofs.attachSession(&session);

    cout << "=============================\n";
    cout << "🧩 Persistent User Test\n";
    cout << "=============================\n\n";

    int choice;
    cout << "1. Format new OFS\n";
    cout << "2. Load existing OFS\n";
    cout << "3. Add new user\n";
    cout << "4. Login as user\n";
    cout << "5. Logout\n";
    cout << "6. List Users\n";
    cout << "0. Exit\n";

    while (true) {
        cout << "\nEnter choice: ";
        cin >> choice;

        if (choice == 1) ofs.format();
        else if (choice == 2) ofs.loadSystem();
        else if (choice == 3) {
            string u, p; bool isAdmin;
            cout << "Enter username: "; cin >> u;
            cout << "Enter password: "; cin >> p;
            cout << "Is Admin? (1/0): "; cin >> isAdmin;
            ofs.createUser(u, p, isAdmin);
        }
        else if (choice == 4) {
            string u, p;
            cout << "Enter username: "; cin >> u;
            cout << "Enter password: "; cin >> p;
            session.login(u, p);
        }
        else if (choice == 5) session.logout();
        else if (choice == 6) ofs.getUserManager().print();
        else if (choice == 0) break;
    }

    cout << "\n✅ Test complete.\n";
    return 0;
}


int main2() {
    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);   // 256 blocks 
    ofs.attachSession(&session);

    cout << "=============================\n";
    cout << "🌐 Omni File System - Final Local Test\n";
    cout << "=============================\n\n";

    int choice;
    while (true) {
        cout << "\n=========== MAIN MENU ===========\n"
             << "1. Format new OFS (Admin)\n"
             << "2. Load existing OFS\n"
             << "3. Add new user (Admin)\n"
             << "4. Login as user\n"
             << "5. Logout\n"
             << "6. Write sample file\n"
             << "7. Read file data\n"
             << "8. View system stats\n"
             << "9. View change log\n"
             << "10. Modify file (new version)\n"
             << "11. List all versions\n"
             << "12. Revert to version\n"
             << "13. List users (AVL Inorder)\n"
             << "14. List My Files\n"
             <<"15. List All Files (Admin)\n"
             "16. Create new directory\n"
             "17. Create new file\n"
             "18. Show my directory tree\n"
             << "0. Exit\n"
             << "=================================\n"
             << "Enter choice: ";

        cin >> choice;
        cin.ignore();

        if (choice == 0) break;

        switch (choice) {
        case 1:
            ofs.format();
            break;

        case 2:
            ofs.loadSystem();
            break;

        case 3: {
            string u, p; bool isAdmin;
            cout << "Enter username: "; getline(cin, u);
            cout << "Enter password: "; getline(cin, p);
            cout << "Is Admin? (1/0): "; cin >> isAdmin; cin.ignore();
            ofs.createUser(u, p, isAdmin);
            break;
        }

        case 4: {
            string u, p;
            cout << "Enter username: "; getline(cin, u);
            cout << "Enter password: "; getline(cin, p);
            session.login(u, p);
            break;
        }

        case 5:
            session.logout();
            break;

        case 6:
            ofs.writeFileContent("/Documents/readme.txt",
                                 "This is version 1 of readme.txt");
            break;

        case 7:
            ofs.readFileContent(0, 64);
            break;

        case 8:
            cout << "\n--- System Stats ---\n";
            // stats already printed by updateStats()
            break;

        case 9:
            ofs.showChangeLog();
            break;

        case 10:
            ofs.writeFileContent("/Documents/readme.txt",
                                 "This is version 2 of readme.txt");
            break;

        case 11:
            ofs.listVersions();
            break;

        case 12: {
            uint64_t vid;
            cout << "Enter version ID: ";
            cin >> vid;
            ofs.revertToVersion(vid);
            break;
        }

        case 13:
            userMgr.print();
            break;
        case 14:
            ofs.listMyFiles();
            break;

        case 15:
            ofs.listAllFiles();
        break;
        case 16: {
            string path;
            cout << "Enter directory path (relative to home): ";
            getline(cin, path);
            ofs.createDirectory(path);
            break;
        }

    case 17: {
            string filePath, content;
            cout << "Enter file path (relative to home, e.g. /Projects/notes.txt): ";
            getline(cin, filePath);
            cout << "Enter file content (end with ENTER): ";
            getline(cin, content);
            ofs.createFile(filePath, content);
            break;
        }

    case 18:
        ofs.showMyDirectoryTree();
        break;


        default:
            cout << "⚠️  Invalid option.\n";
        }
        }

        cout << "\n✅ Final Local Test Complete.\n";
        return 0;
    }



int main3() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Structure Check\n";
    cout << "=============================\n\n";

    // --- Core setup ---
    UserManager userManager;
    SessionManager session(&userManager);
    OFSCore ofs(&userManager);

    ofs.attachSession(&session);

    // --- Optional login (not required for verify) ---
    session.login("admin", "admin123");

    // --- Perform verification ---
    ofs.verifyFileStructure();

    cout << "\n✅ Structure verification complete.\n";
    return 0;
}


int main4() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Auto Verification Run\n";
    cout << "=============================\n\n";

    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);
    ofs.attachSession(&session);

    // ------------------------------------------------------
    // 1️⃣ Admin Login and Format
    // ------------------------------------------------------
    cout << "\n=== STEP 1: Admin Login and Format ===\n";
    session.login("admin", "admin123");
    ofs.format();

    // ------------------------------------------------------
    // 2️⃣ Create users
    // ------------------------------------------------------
    cout << "\n=== STEP 2: Create Users (user1, user2) ===\n";
    ofs.createUser("user1", "pass1", false);
    ofs.createUser("user2", "pass2", false);
    session.logout();

    // ------------------------------------------------------
    // 3️⃣ User1 Creates Directory + Files
    // ------------------------------------------------------
    cout << "\n=== STEP 3: user1 Directory + File Creation ===\n";
    session.login("user1", "pass1");
    ofs.createDirectory("/Projects");
    ofs.createFile("/Projects/report.txt",
        "This is user1’s first report.\n"
        "Line 2: Detailed log entry for user1.\n"
        "Line 3: Some computations here.\n"
        "Line 4: End of user1 report.\n");
    ofs.createFile("/Documents/summary.txt",
        "Summary File - User1\n"
        "Data analysis results are complete.\n"
        "Graphs stored in /Projects.\n"
        "EOF.\n");
    ofs.showMyDirectoryTree();
    session.logout();

    // ------------------------------------------------------
    // 4️⃣ User2 Creates Directory + Files
    // ------------------------------------------------------
    cout << "\n=== STEP 4: user2 Directory + File Creation ===\n";
    session.login("user2", "pass2");
    ofs.createDirectory("/Work");
    ofs.createFile("/Work/notes.txt",
        "User2 Notes:\n"
        "1. Task A - Completed.\n"
        "2. Task B - In Progress.\n"
        "3. Task C - Pending Review.\n");
    ofs.createFile("/Documents/info.txt",
        "Info File for user2.\n"
        "Contains multiple entries.\n"
        "All test cases are executed.\n"
        "EOF.\n");
    ofs.showMyDirectoryTree();
    session.logout();

    // ------------------------------------------------------
    // 5️⃣ Admin Verification
    // ------------------------------------------------------
    cout << "\n=== STEP 5: Admin Verification ===\n";
    session.login("admin", "admin123");
    ofs.listAllFiles();
    ofs.listVersions();
    ofs.verifyFileStructure();
    session.logout();

    cout << "\n✅ Auto verification complete.\n";
    cout << "OFSCore shutting down.\n";
    return 0;
}




int main() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Auto Read-Back Test\n";
    cout << "=============================\n\n";

    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);
    ofs.attachSession(&session);

    // Step 1: Admin formats system
    session.login("admin", "admin123");
    ofs.format();

    // Step 2: Create two users
    ofs.createUser("user1", "pass1", false);
    ofs.createUser("user2", "pass2", false);
    session.logout();

    // Step 3: user1 writes 2 files
    session.login("user1", "pass1");
    ofs.createDirectory("/Projects");
    ofs.createFile("/Projects/report.txt",
R"(This is user1's project report.
It contains detailed analysis of the system.
All tests have passed successfully.
End of user1's report.)");

    ofs.createDirectory("/Documents");
    ofs.createFile("/Documents/summary.txt",
R"(User1 summary file.
Contains short notes and ideas.
Will be updated weekly.
End of summary.)");

    // Read back both files
    ofs.readFileContent(0);
    ofs.readFileContent(1);
    session.logout();

    // Step 4: user2 writes 2 files
    session.login("user2", "pass2");
    ofs.createDirectory("/Work");
    ofs.createFile("/Work/notes.txt",
R"(This is user2's work notes.
Today's topic: socket implementation.
Next task: integrate server-client model.
End of notes.)");

    ofs.createDirectory("/Documents");
    ofs.createFile("/Documents/info.txt",
R"(User2 information log.
Lists files processed today.
Includes pending tasks summary.
End of info.)");

    // Read back both files
    ofs.readFileContent(2);
    ofs.readFileContent(3);
    session.logout();

    // Step 5: Admin checks structure
    session.login("admin", "admin123");
    ofs.listAllFiles();
    ofs.verifyFileStructure();

    cout << "\n✅ Auto read-back verification complete.\n";
    return 0;
}





