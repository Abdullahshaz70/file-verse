#include<iostream>
#include "../include/core/OFSCore.hpp"

using namespace std;


// int main1() {
//     cout << "=============================\n";
//     cout << "🌐 Omni File System - DeltaVault Phase\n";
//     cout << "=============================\n\n";

//     OFSCore ofs(2048);

//     while (true) {
//         cout << "\n=========== MAIN MENU ===========\n";
//         cout << "1. Format new OFS\n";
//         cout << "2. Load existing OFS\n";
//         cout << "3. Write sample file data\n";
//         cout << "4. Read sample file data\n";
//         cout << "5. View system stats\n";
//         cout << "6. View change log\n";
//         cout << "7. Modify file & save version\n";
//         cout << "8. View file versions\n";
//         cout << "9. Revert file to version\n";
//         cout << "10. Verify file structure\n";
//         cout << "0. Exit\n";
//         cout << "================================\n";
//         cout << "Enter choice: ";

//         int choice;
//         cin >> choice;
//         cin.ignore(); // Clear newline

//         switch (choice) {
//             case 1:
//                 ofs.format();
//                 break;

//             case 2:
//                 ofs.loadSystem();
//                 break;

//             case 3:
//                 cout << "\n📄 Writing sample file...\n";
//                 ofs.writeFileContent("/Documents/readme.txt", 
//                     "This is sample data for version testing. [v1]");
//                 break;

//             case 4:
//                 cout << "\n📖 Reading file block #0...\n";
//                 ofs.readFileContent(0, 64);
//                 break;

//             case 5:
//                 ofs.loadSystem(); // refresh stats
//                 break;

//             case 6:
//                 ofs.showChangeLog();
//                 break;

//             case 7: {
//                 cout << "\n✏️ Modifying file and saving new version...\n";
//                 ofs.writeFileContent("/Documents/readme.txt", 
//                     "This is the modified content. [v2]");
//                 break;
//             }

//             case 8:
//                 ofs.listVersions();
//                 break;

//             case 9: {
//                 uint64_t versionID;
//                 cout << "Enter Version ID to revert: ";
//                 cin >> versionID;
//                 ofs.revertToVersion(versionID);
//                 break;
//             }

//             case 10:
//                 cout << "🧩 Verifying file structure...\n";
//                 ofs.loadSystem(); // ensures header is reloaded
//                 break;

//             case 0:
//                 cout << "🚪 Exiting...\n";
//                 return 0;

//             default:
//                 cout << "❌ Invalid option, please try again.\n";
//         }
//     }

//     return 0;
// }

// int main2() {
//     UserManager um;

//     um.addUser("admin", "12345", true);
//     um.addUser("user1", "hello", false);
//     um.print();

//     um.authenticate("admin", "12345");   // ✅ correct
//     um.authenticate("user1", "wrong");   // ❌ incorrect


//     return 0;
// }

// int main3() {
//     cout << "=============================\n";
//     cout << "👤 User + Session Manager Test\n";
//     cout << "=============================\n\n";

//     UserManager userManager;
//     SessionManager sessionManager(&userManager);

//     // Step 1: Add Users
//     cout << "\n--- Adding Users ---\n";
//     userManager.addUser("admin", "admin123", true);
//     userManager.addUser("user1", "userpass", false);
//     userManager.addUser("user2", "test123", false);

//     cout << "\n✅ Current Users (AVL Inorder):\n";
//     userManager.print();

//     // Step 2: Try logins
//     cout << "\n--- Testing Logins ---\n";
//     sessionManager.login("admin", "admin123");   // ✅ should succeed (Admin)
//     sessionManager.printSession();

//     cout << "\nAttempting invalid login:\n";
//     sessionManager.login("user1", "wrongpass");  // ❌ should fail

//     cout << "\nLogging out admin...\n";
//     sessionManager.logout();

//     cout << "\nLogging in as user1...\n";
//     sessionManager.login("user1", "userpass");   // ✅ should succeed (Normal)
//     sessionManager.printSession();

//     // Step 3: Simulate some operations
//     cout << "\nSimulating 3 file operations by user1...\n";
//     for (int i = 0; i < 3; ++i) sessionManager.recordOperation();
//     sessionManager.printSession();

//     // Step 4: Logout
//     cout << "\nLogging out user1...\n";
//     sessionManager.logout();

//     cout << "\n✅ Test complete.\n";
//     return 0;
// }

// int main4() {
//     OFSCore ofs(256);
//     cout << "=== RBAC Test ===\n";

//     ofs.getUserManager().addUser("admin", "admin123", true);

//     ofs.loginUser("admin", "admin123"); 
//     ofs.createUser("user1", "userpass", false);
//     ofs.logoutUser();

//     ofs.loginUser("user1", "userpass");
//     ofs.format(); 
//     ofs.logoutUser();

//     cout << "✅ RBAC Test Done.\n";

//     return 0;
// }



int main() {
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




