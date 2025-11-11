#include<iostream>
#include "../include/core/OFSCore.hpp"
#include "../include/core/file_io_manager.hpp"


using namespace std;


int main() {
    cout << "=============================\n";
    cout << "🌐 Omni File System - DeltaVault Phase\n";
    cout << "=============================\n\n";

    OFSCore ofs(2048);

    while (true) {
        cout << "\n=========== MAIN MENU ===========\n";
        cout << "1. Format new OFS\n";
        cout << "2. Load existing OFS\n";
        cout << "3. Write sample file data\n";
        cout << "4. Read sample file data\n";
        cout << "5. View system stats\n";
        cout << "6. View change log\n";
        cout << "7. Modify file & save version\n";
        cout << "8. View file versions\n";
        cout << "9. Revert file to version\n";
        cout << "10. Verify file structure\n";
        cout << "0. Exit\n";
        cout << "================================\n";
        cout << "Enter choice: ";

        int choice;
        cin >> choice;
        cin.ignore(); // Clear newline

        switch (choice) {
            case 1:
                ofs.format();
                break;

            case 2:
                ofs.loadSystem();
                break;

            case 3:
                cout << "\n📄 Writing sample file...\n";
                ofs.writeFileContent("/Documents/readme.txt", 
                    "This is sample data for version testing. [v1]");
                break;

            case 4:
                cout << "\n📖 Reading file block #0...\n";
                ofs.readFileContent(0, 64);
                break;

            case 5:
                ofs.loadSystem(); // refresh stats
                break;

            case 6:
                ofs.showChangeLog();
                break;

            case 7: {
                cout << "\n✏️ Modifying file and saving new version...\n";
                ofs.writeFileContent("/Documents/readme.txt", 
                    "This is the modified content. [v2]");
                break;
            }

            case 8:
                ofs.listVersions();
                break;

            case 9: {
                uint64_t versionID;
                cout << "Enter Version ID to revert: ";
                cin >> versionID;
                ofs.revertToVersion(versionID);
                break;
            }

            case 10:
                cout << "🧩 Verifying file structure...\n";
                ofs.loadSystem(); // ensures header is reloaded
                break;

            case 0:
                cout << "🚪 Exiting...\n";
                return 0;

            default:
                cout << "❌ Invalid option, please try again.\n";
        }
    }

    return 0;
}














