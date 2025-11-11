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

    Session() : isAdmin(false), loginTime(0), operations(0), active(false) {}
};

class SessionManager {
private:
    Session current;
    UserManager* userManager;   

public:
    SessionManager(UserManager* um) : userManager(um) {}

    bool login(const string& username, const string& password) {
        if (userManager->authenticate(username, password)) {
            current.username = username;
            UserNode* node = userManager->getRoot();
            while (node) {
                if (username == node->userName) {
                    current.isAdmin = node->isAdmin;
                    break;
                }
                node = (username < node->userName) ? node->left : node->right;
            }
            current.loginTime = time(nullptr);
            current.active = true;
            current.operations = 0;
            cout << "🔓 Login successful for " << username
                 << (current.isAdmin ? " (Admin)" : " (User)") << endl;
            return true;
        }
        cout << "❌ Invalid credentials for " << username << endl;
        return false;
    }

    void logout() {
        if (current.active) {
            cout << "👋 User " << current.username << " logged out.\n";
            current = Session();
        } else {
            cout << "⚠️ No active session to logout.\n";
        }
    }

    void recordOperation() {
        if (current.active) current.operations++;
    }

    bool isActive() const { return current.active; }
    bool isAdminUser() const { return current.active && current.isAdmin; }
    string getCurrentUser() const { return current.active ? current.username : "None"; }

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
};
