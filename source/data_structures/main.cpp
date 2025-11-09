#include<iostream>
#include"user_manager.hpp"
#include"directory_tree.hpp"

using namespace std;

int main1() {
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






int main() {
    DirectoryTree dt;

    
    dt.createDirectory("/", "Documents");
    dt.createDirectory("/Documents", "Assignments");
    dt.createDirectory("/Documents", "Notes");

    dt.createFile("/Documents", "todo.txt", "Finish OFS project!");
    dt.createFile("/Documents/Assignments", "lab1.txt", "Lab 1: AVL Trees");
    dt.createFile("/Documents/Assignments", "lab2.txt", "Lab 2: File System");

    cout << "\nListing root directory:\n";
    dt.list("/");

    cout << "\nListing Documents directory:\n";
    dt.list("/Documents");

    cout << "\nListing Assignments directory:\n";
    dt.list("/Documents/Assignments");


    cout << "\nDeleting lab1.txt...\n";
    dt.deleteNode("/Documents/Assignments/lab1.txt");

    cout << "\nAfter deletion:\n";
    dt.list("/Documents/Assignments");

    dt.createDirectory("/Documents", "Images");
    dt.createFile("/Documents/Images", "logo.png", "PNG DATA");

    cout << "\nListing Documents after adding Images:\n";
    dt.list("/Documents");

    return 0;
}








