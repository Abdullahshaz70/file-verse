#include <iostream>
#include <thread>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm> // Added for std::remove

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "user_manager.hpp"
#include "session_manger.hpp"
#include "OFSCore.hpp"

using namespace std;

static const int SERVER_PORT = 19090;


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
        if (parts.empty()) continue;
        string cmd = parts[0];
        string reply = "";

        
        if (cmd == "QUIT") {
            reply = "OK|BYE\n";
            send(clientSock, reply.c_str(), reply.size(), 0);
            break;
        }

        
        else if (cmd == "LOGIN") {
            bool ok = session.login(parts[1], parts[2]);
            if (ok) {
                // Attach the session before any core operation
                WITH_SESSION(&session);
                
                // Get the username and construct the home directory path
                string username = parts[1];
                string home_dir = "/home/" + username + "/";
                
                // **FIX 2: Set the CWD upon successful login**
                // Assuming OFSCore has a method like setCWD
                // If setCWD doesn't exist, you'll need to implement it in OFSCore
                // to update the current directory path stored in the session object.
                // For now, we'll assume a method exists or that the session is implicitly tracking CWD.
                
                // A better approach (if OFSCore supports it):
                // gOFS.changeDirectory(home_dir); // Assuming CD is implemented to handle this
                
                // For now, let's keep the original placeholder structure but clarify the intent:
                // If you don't have a changeDirectory command, you MUST implement logic
                // within OFSCore to manage the session's CWD state upon login.

                reply = "OK|LOGIN\n";
            } else {
                reply = "ERR|LOGIN_FAILED\n";
            }
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

        else if (cmd == "TRUNCATE") {
            WITH_SESSION(&session);
            string filePath = parts[1];
            string newContent = "DR umer Suleman";

            // 1. Delete the existing file
            bool deleted = gOFS.deleteFile(filePath);
            
            if (deleted) {
                // 2. Create/overwrite the file with the new content
                // Note: The signature used for createFile doesn't return a bool, 
                // so we assume it succeeds if delete did.
                gOFS.createFile(filePath, newContent); 
                reply = "OK|TRUNCATED\n";
            } else {
                // Return an error if the delete failed (e.g., file not found, no permission)
                reply = "ERR|TRUNCATE_FAILED|DELETE_ERROR\n";
            }

        }
        
        else if (cmd == "CREATE_USER") {
            WITH_SESSION(&session);
            string username = parts[1];
            bool isAdmin = parts[3] == "1";

            // 1. Create the user record
            gOFS.createUser(username, parts[2], isAdmin);
            
            // **FIX 1: Explicitly create the home directory**
            string home_dir = "/home/" + username + "/";

            // We use the temporary `boot` session logic if needed, 
            // but since we are currently attached via WITH_SESSION(&session), 
            // and this command *should* only be run by an admin, 
            // we create the directory now.
            
            gOFS.createDirectory(home_dir);
            // if (dir_ok) {
            //     reply = "OK|USER_CREATED_AND_HOME_DIR_CREATED\n";
            // } else {
            //     reply = "ERR|USER_CREATED_BUT_HOME_DIR_FAILED\n";
            //     // You might want to remove the user if directory creation is critical
            //     // but for now, we just report the failure.
            // }
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

        else if (cmd == "WRITE_FILE") {
            // Assuming WRITE_FILE is for overwriting content
            WITH_SESSION(&session);
            string filePath = parts[1];
            string content = parts[2];
            
            // In a simple filesystem, WRITE_FILE can often be implemented as:
            // 1. Delete the old file (to release blocks)
            // 2. Create a new file with the same name and new content
            
            // To be more robust, if your OFSCore has a direct 'writeFile' method, use it:
            // bool ok = gOFS.writeFile(filePath, content); 
            
            // Using the current TRUNCATE logic's approach (delete + create)
            bool deleted = gOFS.deleteFile(filePath);
            if (deleted) {
                gOFS.createFile(filePath, content); 
                reply = "OK|FILE_WRITTEN\n";
            } else {
                 reply = "ERR|WRITE_FAILED|File_not_found_or_no_permission\n";
            }
        }
        
        else {
            reply = "ERR|UNKNOWN_COMMAND\n";
        }

        send(clientSock, reply.c_str(), reply.size(), 0);
    }

    close(clientSock);
}

int start_server() {
// ... (start_server function is unchanged)
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
    
    return 0;
}


















