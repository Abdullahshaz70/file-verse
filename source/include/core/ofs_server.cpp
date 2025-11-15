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


UserManager gUserMgr;           
OFSCore     gOFS(&gUserMgr);    
#define WITH_SESSION(sess) gOFS.attachSession(sess)

vector<string> split(const string& s, char delim) {
    vector<string> p;
    string temp;
    stringstream ss(s);
    while (getline(ss, temp, delim)) p.push_back(temp);
    return p;
}

void handleClient(int clientSock) {
    SessionManager session(&gUserMgr);   
    char buffer[8192];

    while (true) {
        ssize_t n = recv(clientSock, buffer, sizeof(buffer)-1, 0);
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
            WITH_SESSION(&session);
            gOFS.format();
            reply = "OK|FORMATTED\n";
        }

        
        else if (cmd == "LOAD") {
            WITH_SESSION(&session);
            reply = gOFS.loadSystem() ? "OK|LOADED\n" : "ERR|LOAD_FAILED\n";
        }

        
        else if (cmd == "CREATE_USER") {
            WITH_SESSION(&session);
            gOFS.createUser(parts[1], parts[2], parts[3] == "1");
            reply = "OK|USER_CREATED\n";
        }

        
        else if (cmd == "CREATE_FILE") {
            WITH_SESSION(&session);
            gOFS.createFile(parts[1], parts[2]);
            reply = "OK|FILE_CREATED\n";
        }

        
        else if (cmd == "DELETE_FILE") {
            WITH_SESSION(&session);
            bool ok = gOFS.deleteFile(parts[1]);
            reply = ok ? "OK|FILE_DELETED\n" : "ERR|DELETE_FAILED\n";
        }

        
        else if (cmd == "READ_BLOCK") {
            WITH_SESSION(&session);
            stringstream out;
            streambuf *old = cout.rdbuf(out.rdbuf());

            gOFS.readFileContent(stoi(parts[1]));

            cout.rdbuf(old);
            reply = out.str();
        }

        
        else if (cmd == "CREATE_DIR") {
            WITH_SESSION(&session);
            gOFS.createDirectory(parts[1]);
            reply = "OK|DIR_CREATED\n";
        }

    
        else if (cmd == "DELETE_DIR") {
            WITH_SESSION(&session);
            bool ok = gOFS.deleteDirectory(parts[1]);
            reply = ok ? "OK|DIR_DELETED\n" : "ERR|DELETE_FAILED\n";
        }


        else if (cmd == "LIST_MY_FILES") {
            WITH_SESSION(&session);
            stringstream out;
            streambuf *old = cout.rdbuf(out.rdbuf());

            gOFS.listMyFiles();

            cout.rdbuf(old);
            reply = out.str();
        }


        else if (cmd == "LIST_ALL_FILES") {
            WITH_SESSION(&session);
            stringstream out;
            streambuf *old = cout.rdbuf(out.rdbuf());

            gOFS.listAllFiles();

            cout.rdbuf(old);
            reply = out.str();
        }


        else if (cmd == "SHOW_TREE") {
            WITH_SESSION(&session);
            stringstream out;
            streambuf *old = cout.rdbuf(out.rdbuf());

            gOFS.showMyDirectoryTree();

            cout.rdbuf(old);
            reply = out.str();
        }


        else if (cmd == "SHOW_CHANGE_LOG") {
            WITH_SESSION(&session);
            stringstream out;
            streambuf *old = cout.rdbuf(out.rdbuf());

            gOFS.showChangeLog();

            cout.rdbuf(old);
            reply = out.str();
        }

        else {
            reply = "ERR|UNKNOWN_COMMAND\n";
        }

        send(clientSock, reply.c_str(), reply.size(), 0);
    }

    close(clientSock);
}

int start_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(SERVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (sockaddr*)&addr, sizeof(addr));
    listen(sock, 50);

    cout << "🚀 OFS SERVER running on port " << SERVER_PORT << "\n";


    {
        SessionManager boot(&gUserMgr);
        WITH_SESSION(&boot);
        gOFS.loadSystem();
    }

    while (true) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int clientSock = accept(sock, (sockaddr*)&client, &len);
        thread(handleClient, clientSock).detach();
    }
}
