#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "user_manager.hpp"
#include "session_manger.hpp"
#include "OFSCore.hpp"

using namespace std;
static const int SERVER_PORT = 9090;

// -----------------------------------
// Helper split
vector<string> split(const string& s, char delim) {
    vector<string> parts;
    string temp;
    stringstream ss(s);
    while (getline(ss, temp, delim)) parts.push_back(temp);
    return parts;
}

// -----------------------------------
void handleClient(int clientSock) {
    UserManager userMgr;
    SessionManager session(&userMgr);
    OFSCore ofs(&userMgr, 2048);
    ofs.attachSession(&session);

    ofs.loadSystem();

    char buffer[8192];

    while (true) {
        ssize_t n = recv(clientSock, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        string req(buffer);
        req.erase(remove(req.begin(), req.end(), '\n'), req.end());

        auto parts = split(req, '|');
        string cmd = parts[0];

        string reply = "";

        if (cmd == "QUIT") {
            reply = "OK|BYE\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            break;
        }

        else if (cmd == "LOGIN") {
            bool ok = session.login(parts[1], parts[2]);
            reply = ok ? "OK|LOGIN\n" : "ERR|LOGIN_FAILED\n";
        }

        else if (cmd == "LOGOUT") {
            session.logout();
            reply = "OK|LOGOUT\n";
        }

        else if (cmd == "FORMAT") {
            ofs.format();
            reply = "OK|FORMATTED\n";
        }

        else if (cmd == "CREATE_USER") {
            ofs.createUser(parts[1], parts[2], parts[3] == "1");
            reply = "OK|USER_CREATED\n";
        }

        else if (cmd == "CREATE_FILE") {
            ofs.createFile(parts[1], parts[2]);
            reply = "OK|FILE_CREATED\n";
        }

        else if (cmd == "DELETE_FILE") {
            bool ok = ofs.deleteFile(parts[1]);
            reply = ok ? "OK|FILE_DELETED\n" : "ERR|DELETE_FAILED\n";
        }

        else if (cmd == "READ_BLOCK") {
            stringstream out;
            streambuf* old = cout.rdbuf(out.rdbuf());

            ofs.readFileContent(stoi(parts[1]), 4096);

            cout.rdbuf(old);
            reply = out.str();
        }


        else if (cmd == "CREATE_DIR") {
            ofs.createDirectory(parts[1]);
            reply = "OK|DIR_CREATED\n";
        }

        else if (cmd == "DELETE_DIR") {
            bool ok = ofs.deleteDirectory(parts[1]);
            reply = ok ? "OK|DIR_DELETED\n" : "ERR|DELETE_FAILED\n";
        }

        else if (cmd == "LIST_MY_FILES") {
            // capture cout into string
            stringstream out;
            streambuf* old = cout.rdbuf(out.rdbuf());

            ofs.listMyFiles();

            cout.rdbuf(old);
            reply = out.str();
        }

        else if (cmd == "LIST_ALL_FILES") {
            stringstream out;
            streambuf* old = cout.rdbuf(out.rdbuf());
            ofs.listAllFiles();
            cout.rdbuf(old);
            reply = out.str();
        }

        else if (cmd == "SHOW_TREE") {
            stringstream out;
            streambuf* old = cout.rdbuf(out.rdbuf());
            ofs.showMyDirectoryTree();
            cout.rdbuf(old);
            reply = out.str();
        }

        else if (cmd == "SHOW_CHANGE_LOG") {
            stringstream out;
            streambuf* old = cout.rdbuf(out.rdbuf());
            ofs.showChangeLog();
            cout.rdbuf(old);
            reply = out.str();
        }

        else reply = "ERR|UNKNOWN_COMMAND\n";

        send(clientSock, reply.c_str(), reply.size(), 0);
    }

    close(clientSock);
}

// -----------------------------------
int start_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (sockaddr*)&addr, sizeof(addr));
    listen(sock, 10);

    cout << "🚀 OFS SERVER running on port 9090\n";

    while (true) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int c = accept(sock, (sockaddr*)&client, &len);

        thread(handleClient, c).detach();
    }
}
