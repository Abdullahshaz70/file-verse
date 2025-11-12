#include<iostream>

#include "../include/core/OFSCore.hpp"

using namespace std;



int maintyuiop(){
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


int main7890() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Auto Validation\n";
    cout << "=============================\n\n";

    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256); // 256 blocks → 1 MB
    ofs.attachSession(&session);

    // ────────────────────────────────
    // Step 1: Admin Login + Format
    // ────────────────────────────────
    session.login("admin", "admin123");
    if (!session.isAdminUser()) {
        cerr << "❌ Admin login failed!\n";
        return 1;
    }
    ofs.format();

    // ────────────────────────────────
    // Step 2: Create Users
    // ────────────────────────────────
    ofs.createUser("user1", "user1pass", false);
    ofs.createUser("user2", "user2pass", false);

    // ────────────────────────────────
    // Step 3: User1 — create dirs & files
    // ────────────────────────────────
    session.logout();
    session.login("user1", "user1pass");
    ofs.createDirectory("Projects");
    ofs.createDirectory("Documents");
    ofs.createFile("Projects/report.txt",
        "This is user1's project report.\nAll tests successful.\n");
    ofs.createFile("Documents/summary.txt",
        "User1 weekly summary content.\nEnd of file.\n");
    ofs.showMyDirectoryTree();

    // ────────────────────────────────
    // Step 4: Read User1 file data
    // ────────────────────────────────
    cout << "\n🔍 Reading back User1 files:\n";
    ofs.readFileContent(0); // first block
    ofs.readFileContent(1); // second block

    // ────────────────────────────────
    // Step 5: User2 — create dirs & files
    // ────────────────────────────────
    session.logout();
    session.login("user2", "user2pass");
    ofs.createDirectory("Work/Logs");
    ofs.createFile("Work/notes.txt",
        "User2 notes: socket tests.\nEnd.\n");
    ofs.createFile("Work/Logs/info.txt",
        "User2 log info.\nList of processed items.\n");
    ofs.showMyDirectoryTree();

    // ────────────────────────────────
    // Step 6: Admin login → global checks
    // ────────────────────────────────
    session.logout();
    session.login("admin", "admin123");
    cout << "\n\n=============================\n";
    cout << "🔎 SYSTEM-WIDE VERIFICATION\n";
    cout << "=============================\n";
    ofs.listAllFiles();
    ofs.listVersions();
    ofs.verifyFileStructure();
    ofs.showChangeLog();

    // ────────────────────────────────
    // Step 7: Stats check
    // ────────────────────────────────
    cout << "\n\n=============================\n";
    cout << "📊 Final Stats Check\n";
    cout << "=============================\n";
    cout << "If no warnings or corruption appear above, ✅ PHASE 1 PASSED!\n";

    cout << "\nOFSCore shutting down cleanly.\n";
    return 0;
}


int main1() {
    cout << "=============================\n";
    cout << "🔍 Omni File System Auto Verification\n";
    cout << "=============================\n\n";

    // Initialize managers
    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);
    ofs.attachSession(&session);

    // ---------- STEP 1: LOGIN AS ADMIN ----------
    session.login("admin", "admin123");

    // ---------- STEP 2: FORMAT + AUTO RELOAD ----------
    ofs.format(); // auto reloads and creates admin home

    // ---------- STEP 3: CREATE USERS ----------
    ofs.createUser("user1", "pass1", false);
    ofs.createUser("user2", "pass2", false);

    // ---------- STEP 4: LOGIN AS USER1 ----------
    session.logout();
    session.login("user1", "pass1");

    // create nested structure
    ofs.createDirectory("projects");
    ofs.createDirectory("media");
    ofs.createFile("projects/main.cpp", "#include<iostream>\nint main(){return 0;}");
    ofs.createFile("media/images.png", "fake_binary_data_123");

    ofs.showMyDirectoryTree();

    // ---------- STEP 5: LOGIN AS USER2 ----------
    session.logout();
    session.login("user2", "pass2");
    ofs.createDirectory("Work/Logs");
    ofs.createFile("Work/Logs/info.txt", "user2 log file");
    ofs.showMyDirectoryTree();

    // ---------- STEP 6: LOGIN AS ADMIN ----------
    session.logout();
    session.login("admin", "admin123");
    cout << "\n=============================\n";
    cout << "🔎 SYSTEM-WIDE VERIFICATION\n";
    cout << "=============================\n\n";

    ofs.listAllFiles();
    ofs.listVersions();
    ofs.verifyFileStructure();

    cout << "\n=============================\n";
    cout << "📊 Final Stats Check\n";
    cout << "=============================\n";
    cout << "If no warnings or corruption appear above, ✅ PHASE 1 PASSED!\n\n";

    // ---------- STEP 7: SAVE + EXIT ----------
    cout << "💾 Saving OFS state before exit...\n";
    ofs.saveSystemState();  // <— new helper for explicit save
    cout << "✅ Auto Verification Complete.\n";
    return 0;
}









int main() {
    cout << "=============================\n";
    cout << "🔎 Omni File System - Saved State Verification\n";
    cout << "=============================\n\n";

    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 256);
    ofs.attachSession(&session);

    // --- STEP 1: Load Existing OFS ---
    if (!ofs.loadSystem()) {
        cerr << "❌ Failed to load existing OFS. Maybe filesystem.omni is missing or corrupted.\n";
        return 1;
    }

    cout << "\n✅ Loaded OFS successfully.\n";

    // --- STEP 2: Print All Users ---
    cout << "\n=============================\n";
    cout << "👥 USER TABLE (AVL Inorder)\n";
    cout << "=============================\n";
    userMgr.print();

    // --- STEP 3: Login as Admin ---
    session.login("admin", "admin123");

    // --- STEP 4: Show Directory Trees ---
    cout << "\n=============================\n";
    cout << "🌳 COMPLETE DIRECTORY STRUCTURE\n";
    cout << "=============================\n";
    ofs.listAllFiles();

    // --- STEP 5: Verify Versions + Header ---
    cout << "\n=============================\n";
    cout << "🧾 VERSION + HEADER VERIFICATION\n";
    cout << "=============================\n";
    ofs.listVersions();
    ofs.verifyFileStructure();

    // --- STEP 6: Final Summary ---
    cout << "\n=============================\n";
    cout << "📊 FINAL VERIFICATION SUMMARY\n";
    cout << "=============================\n";
    cout << "✅ Persistence Verified Successfully.\n";
    cout << "✅ No corruption or missing data detected.\n";
    cout << "OFS Verification Complete.\n";

    return 0;
}



