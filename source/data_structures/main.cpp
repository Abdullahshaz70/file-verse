#include<iostream>
#include"user_manager.hpp"

using namespace std;

int main() {
    UserManager um;
    um.addUser("Ali", "123", false);
    um.addUser("Zara", "456", true);
    um.addUser("Ahmed", "789", false);

    cout << "\nAll Users:\n";
    um.print();

    cout << "\nAuthenticate Zara: "
         << (um.authenticate("Zara", "456") ? "Success" : "Fail") << endl;

    um.removeUser("Ali");
    cout << "\nAfter removing Ali:\n";
    um.print();

    return 0;
}
