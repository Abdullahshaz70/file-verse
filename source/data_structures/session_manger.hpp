#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include "user_manager.hpp"

using namespace std;

struct Session {
    string username;
    bool isAdmin;
    time_t loginTime;
    int operations;
    bool active;

    Session() : username(""), isAdmin(false), loginTime(0), operations(0), active(false) {}
};

class SessionManager {
private:
    Session current;
    UserManager* userManager;   // Pointer to UserManager tree

public:
    SessionManager(UserManager* um) : userManager(um) {}

    // -------------------------------
    // LOGIN + LOGOUT
    // -------------------------------
    bool login(const string& username, const string& password) {
        if (userManager->authenticate(username, password)) {
            UserNode* node = userManager->getRoot();
            while (node) {
                if (username == node->userName) {
                    current.username = username;
                    current.isAdmin = node->isAdmin;
                    current.loginTime = time(nullptr);
                    current.operations = 0;
                    current.active = true;
                    cout << "🔓 Login successful for " << username
                         << (current.isAdmin ? " (Admin)" : " (User)") << endl;
                    return true;
                }
                node = (username < node->userName) ? node->left : node->right;
            }
        }
        cout << "❌ Invalid credentials for " << username << endl;
        return false;
    }

    void logout() {
        if (current.active) {
            cout << "👋 User " << current.username << " logged out.\n";
            current = Session();  // reset
        } else {
            cout << "⚠️ No active session to logout.\n";
        }
    }

    // -------------------------------
    // OPERATIONS + STATUS
    // -------------------------------
    void recordOperation() {
        if (current.active) current.operations++;
    }

    // --- NEW Helper Accessors (used by OFSCore) ---
    bool isLoggedIn() const { return current.active; }
    bool isAdmin() const { return current.active && current.isAdmin; }
    string getActiveUsername() const { return current.active ? current.username : "None"; }
    bool isActive() const { return current.active; }

    bool isAdminUser() const {return current.active && current.isAdmin; }

string getCurrentUser() const { return current.active ? current.username : "None"; }



    // Optional: used to show current user info
    void printSession() const {
        if (!current.active) {
            cout << "No active session.\n";
            return;
        }
        cout << "🧑‍💻 Current User: " << current.username
             << (current.isAdmin ? " (Admin)" : " (User)") << endl;
        cout << "🕒 Login Time: " << ctime(&current.loginTime);
        cout << "⚙️ Operations performed: " << current.operations << endl;
    }

    void forceSetUser(const std::string& username) {
    current.username = username;
    }


};
