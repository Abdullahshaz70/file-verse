# Omni File System (OFS)
## Phase-1 — Testing & Verification Report

### 📘 Overview
This document records the testing, verification, and output logs for the **Core Design & Function Implementation** phase of the Omni File System (OFS) project.

Phase-1 validates the correct behavior of all core subsystems:
- User Management (AVL Tree)
- Directory Tree (Hierarchical File System)
- Free Space Manager (Bitmap Allocation)
- OFSCore Integration (Unified Controller)

---

## 🧩 Test Environment

| Component | Details |
|------------|----------|
| **Compiler** | g++ 13.2.0 |
| **Operating System** | Ubuntu 24.04 (WSL2) |
| **Language Standard** | C++17 |
| **Executable Name** | `main.exe` |
| **Working Directory** | `~/Abdullah Shahzad/Data Structures/Project/file-verse/source/` |

---

## 🧱 Test Structure

All modules were integrated inside `OFSCore.hpp` as a single header-only class.  
Testing was performed via `main.cpp` located in `/source/data_structures/`.

### **Code Snippet: main.cpp**

```cpp
#include "../include/core/OFSCore.hpp"

int main() {
    cout << "Starting program..." << endl;
    OFSCore ofs(20);                // Initialize core with 20 blocks

    cout << "Calling format..." << endl;
    ofs.format();                   

    cout << "Creating user..." << endl;
    ofs.createUser("admin", "123", true);

    cout << "Logging in..." << endl;
    ofs.login("admin", "123");

    cout << "Creating file..." << endl;
    ofs.createFile("/", "readme.txt", "Welcome to OFS!");

    cout << "Listing directory..." << endl;
    ofs.listDirectory("/");

    cout << "All done!" << endl;
}
