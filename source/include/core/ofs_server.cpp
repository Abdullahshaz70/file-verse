#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "user_manager.hpp"
#include "session_manger.hpp"
#include "OFSCore.hpp"

using namespace std;

static const int SERVER_PORT = 9090;

// ---------------------------
// Small helpers
// ---------------------------

vector<string> splitByPipe(const string &s) {
    vector<string> parts;
    string part;
    stringstream ss(s);
    while (getline(ss, part, '|')) {
        parts.push_back(part);
    }
    return parts;
}

string trimNewlines(string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        s.pop_back();
    return s;
}

// NEW: Normalize path so commands like /test.txt map to /home/user/test.txt
string normalizePath(const string& path, const string& username) {
    // If the user provided a full path, use it directly
    if (path.rfind("/home/", 0) == 0)
        return path;

    // If it starts with / then append after /home/user
    if (!path.empty() && path[0] == '/')
        return "/home/" + username + path;

    // Default: treat it as a filename
    return "/home/" + username + "/" + path;
}

// ---------------------------
// Per-client handler
// ---------------------------

void handleClient(int clientSock) {
    cout << "🎧 New client connected (fd=" << clientSock << ")\n";

    // Each client gets its own OFS + Session + UserManager view
    UserManager userManager;
    SessionManager session(&userManager);
    OFSCore ofs(&userManager);
    ofs.attachSession(&session);

    // Try to load existing system (if .omni exists)
    if (!ofs.loadSystem()) {
        cout << "⚠️ Could not load existing OFS. "
                "You probably need to FORMAT as admin once.\n";
    }

    char buffer[4096];
    bool running = true;

    while (running) {
        ssize_t n = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            cout << "❌ Client disconnected (fd=" << clientSock << ")\n";
            break;
        }

        buffer[n] = '\0';
        string req = trimNewlines(string(buffer));
        if (req.empty()) continue;

        cout << "[CLIENT " << clientSock << "] => " << req << endl;
        vector<string> parts = splitByPipe(req);

        string cmd = parts[0];
        string response;

        // ----------- COMMANDS -------------

        if (cmd == "LOGIN") {
            if (parts.size() < 3) {
                response = "ERR|Usage: LOGIN|username|password\n";
            } else {
                string user = parts[1];
                string pass = parts[2];
                bool ok = session.login(user, pass);
                ofs.attachSession(&session);
                response = ok ? "OK|LOGIN\n" : "ERR|Invalid credentials\n";
            }
        }

        else if (cmd == "LOGOUT") {
            session.logout();
            response = "OK|LOGOUT\n";
        }

        else if (cmd == "CREATE_USER") {
            if (!session.isAdminUser()) {
                response = "ERR|Only admin can create users\n";
            } else if (parts.size() < 4) {
                response = "ERR|Usage: CREATE_USER|username|password|role\n";
            } else {
                string user = parts[1];
                string pass = parts[2];
                bool isAdmin = (parts[3] == "admin" ||
                                parts[3] == "ADMIN" ||
                                parts[3] == "1");

                ofs.createUser(user, pass, isAdmin);
                response = "OK|USER_CREATED\n";
            }
        }

        else if (cmd == "CREATE_FILE") {
            if (!session.isLoggedIn()) {
                response = "ERR|Login required\n";
            } else if (parts.size() < 3) {
                response = "ERR|Usage: CREATE_FILE|/path|content\n";
            } else {
                string fullPath = normalizePath(parts[1], session.getCurrentUser());
                string content  = parts[2];
                ofs.createFile(fullPath, content);
                response = "OK|FILE_CREATED\n";
            }
        }

        else if (cmd == "DELETE_FILE") {
            if (!session.isLoggedIn()) {
                response = "ERR|Login required\n";
            } else if (parts.size() < 2) {
                response = "ERR|Usage: DELETE_FILE|/path\n";
            } else {
                string fullPath = normalizePath(parts[1], session.getCurrentUser());
                bool ok = ofs.deleteFile(fullPath);
                response = ok ? "OK|FILE_DELETED\n" : "ERR|DELETE_FAILED\n";
            }
        }

        else if (cmd == "FORMAT") {
            if (!session.isAdminUser()) {
                response = "ERR|Only admin can FORMAT\n";
            } else {
                ofs.format();
                response = "OK|FORMATTED\n";
            }
        }

        else if (cmd == "QUIT") {
            response = "OK|BYE\n";
            running = false;
        }

        else {
            response = "ERR|Unknown command\n";
        }

        send(clientSock, response.c_str(), response.size(), 0);
        cout << "[SERVER] => " << trimNewlines(response) << endl;
    }

    close(clientSock);
    cout << "🔌 Closed client socket (fd=" << clientSock << ")\n";
}

// ---------------------------
// main: listen + accept threads
// ---------------------------

int start_server() {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(serverSock);
        return 1;
    }

    if (listen(serverSock, 10) < 0) {
        perror("listen");
        close(serverSock);
        return 1;
    }

    cout << "🚀 OFS server listening on port " << SERVER_PORT << "...\n";

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t len = sizeof(clientAddr);
        int clientSock = accept(serverSock, (sockaddr*)&clientAddr, &len);
        if (clientSock < 0) {
            perror("accept");
            continue;
        }

        cout << "✅ Accepted connection from "
             << inet_ntoa(clientAddr.sin_addr)
             << ":" << ntohs(clientAddr.sin_port)
             << " (fd=" << clientSock << ")\n";

        thread t(handleClient, clientSock);
        t.detach();
    }

    close(serverSock);
    return 0;
}
