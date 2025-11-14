
#include <iostream>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

static const int SERVER_PORT = 9090;
static const char* SERVER_IP = "127.0.0.1";

string sendCommand(int sock, const string &cmd) {
    string msg = cmd + "\n";
    send(sock, msg.c_str(), msg.size(), 0);

    char buffer[8192];
    ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) return "❌ Server closed connection";

    buffer[n] = '\0';
    return string(buffer);
}

int start_client() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv.sin_addr);

    if (connect(sock, (sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    cout << "=============================\n";
    cout << "🌐 Omni File System - CLIENT MENU\n";
    cout << "=============================\n";

    while (true) {
        cout << "\n=========== MAIN MENU ===========\n"
             << "1. Format OFS (Admin)\n"
             << "2. Load existing OFS\n"
             << "3. Create new user (Admin)\n"
             << "4. Login\n"
             << "5. Logout\n"
             << "6. Write sample file\n"
             << "7. Read file block\n"
             << "8. Show system stats\n"
             << "9. Show change log\n"
             << "13. List all users\n"
             << "14. List MY files\n"
             << "15. List ALL files (Admin)\n"
             << "16. Create directory\n"
             << "17. Create file\n"
             << "18. Show my directory tree\n"
             << "19. Delete file\n"
             << "20. Delete directory\n"
             << "0. Quit\n"
             << "=================================\n"
             << "Enter choice: ";

        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 0) {
            cout << sendCommand(sock, "QUIT");
            break;
        }

        string a, b, c;

        switch (choice) {
        case 1:
            cout << sendCommand(sock, "FORMAT");
            break;

        case 2:
            cout << sendCommand(sock, "LOAD");
            break;

        case 3:
            cout << "Username: "; getline(cin, a);
            cout << "Password: "; getline(cin, b);
            cout << "Admin? (1/0): "; getline(cin, c);
            cout << sendCommand(sock, "CREATE_USER|" + a + "|" + b + "|" + c);
            break;

        case 4:
            cout << "Username: "; getline(cin, a);
            cout << "Password: "; getline(cin, b);
            cout << sendCommand(sock, "LOGIN|" + a + "|" + b);
            break;

        case 5:
            cout << sendCommand(sock, "LOGOUT");
            break;

        case 6:
            cout << sendCommand(sock,
                "CREATE_FILE|/Documents/readme.txt|This is sample data");
            break;

        case 7:
            cout << "Block index: "; getline(cin, a);
            cout << sendCommand(sock, "READ_BLOCK|" + a);
            break;

        case 8:
            cout << sendCommand(sock, "STATS");
            break;

        case 9:
            cout << sendCommand(sock, "SHOW_CHANGE_LOG");
            break;

        case 13:
            cout << sendCommand(sock, "LIST_USERS");
            break;

        case 14:
            cout << sendCommand(sock, "LIST_MY_FILES");
            break;

        case 15:
            cout << sendCommand(sock, "LIST_ALL_FILES");
            break;

        case 16:
            cout << "Directory path: ";
            getline(cin, a);
            cout << sendCommand(sock, "CREATE_DIR|" + a);
            break;

        case 17:
            cout << "File path: ";
            getline(cin, a);
            cout << "Content: ";
            getline(cin, b);
            cout << sendCommand(sock, "CREATE_FILE|" + a + "|" + b);
            break;

        case 18:
            cout << sendCommand(sock, "SHOW_TREE");
            break;

        case 19:
            cout << "File path: ";
            getline(cin, a);
            cout << sendCommand(sock, "DELETE_FILE|" + a);
            break;

        case 20:
            cout << "Directory path: ";
            getline(cin, a);
            cout << sendCommand(sock, "DELETE_DIR|" + a);
            break;

        default:
            cout << "⚠ Invalid choice\n";
        }
    }

    close(sock);
    return 0;
}
