## 1. User Management Structure

### Overview
The User Management system is implemented using a **self-balancing AVL Tree**.  
Each user is represented as a node in the tree, uniquely identified by their username.

### Node Fields
- **userName (string):** Unique identifier for each user.
- **password (string):** User password (plain for now; can be hashed later).
- **isAdmin (bool):** Flag that marks administrator accounts.
- **left / right (UserNode*):** Pointers to left and right child nodes.
- **height (int):** Node height, used to calculate balance factors.

### Operations
- **addUser(username, password, isAdmin):** Inserts a new node while maintaining AVL balance.
- **authenticate(username, password):** Searches for a user and verifies credentials.
- **removeUser(username):** Deletes a node and rebalances the tree.
- **print():** Performs an in-order traversal to list users alphabetically.

### Design Rationale
An AVL Tree was chosen because it guarantees O(log n) lookup, insertion,  
and deletion. User operations like authentication and registration require  
fast search and insertion. While it uses slightly more memory due to  
pointers and height fields, the trade-off for performance and consistency  
is acceptable given the small number of users in the OFS system.

Memory cleanup is handled through a recursive destructor that deletes  
all nodes when the UserManager instance is destroyed.



## 2. Directory Tree Structure

### Overview
The Directory Tree represents the hierarchical structure of all files and folders
within the Omni File System (OFS). It allows fast traversal, creation, and deletion
of directories and files using path-based access (e.g., `/Documents/Notes.txt`).

### Structure Design
Each node in the tree is represented by the following structure:

```cpp
struct FileNode {
    ing me;                
    bool isFile;                     
    string data;                
    FileNode* parent;           
    vector<FileNode*> children; 
};

## 3. Free Space Management

## 4. Path-to-Location Mapping

## 5. .omni File Layout
