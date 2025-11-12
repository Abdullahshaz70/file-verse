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














int main() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Structure Check\n";
    cout << "=============================\n\n";

    // Initialize managers
    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);   // 256 blocks = 1MB
    ofs.attachSession(&session);

    // 1️⃣ Login as admin (default)
    session.login("admin", "admin123");

    // 2️⃣ Format new filesystem
    ofs.format();

    // 3️⃣ Create two normal users
    ofs.createUser("user1", "pass1", false);
    ofs.createUser("user2", "pass2", false);

    // 4️⃣ Login as user1 and write sample file
    session.logout();
    session.login("user1", "pass1");
    ofs.writeFileContent("/Documents/readme.txt", "This is version 1 of readme.txt");

    // 5️⃣ Write a modified version
    ofs.writeFileContent("/Documents/readme.txt", "This is version 2 of readme.txt");

    // 6️⃣ Login as user2 and write their file
    session.logout();
    session.login("user2", "pass2");
    ofs.writeFileContent("/Documents/readme.txt", "This is user2’s document.");

    // 7️⃣ Logout and login as admin again
    session.logout();
    session.login("admin", "admin123");

    // 8️⃣ Verify structure
    ofs.verifyFileStructure();

    ofs.listAllFiles();

    cout << "\n✅ Structure verification complete.\n";
    return 0;
}
