## 1. User Management Structure

### Overview
The User Management system is implemented using a self-balancing AVL Tree.  
Each user is represented as a node in the tree, uniquely identified by their username.

### Node Fields
- userName: Unique identifier for each user  
- password: User password (plain for now; can be hashed later)  
- isAdmin: Flag that marks administrator accounts  
- left / right: Pointers to left and right child nodes  
- height: Node height, used to calculate balance factors

### Operations
- addUser(username, password, isAdmin) — Inserts a new node while maintaining AVL balance  
- authenticate(username, password) — Searches for a user and verifies credentials  
- removeUser(username) — Deletes a node and rebalances the tree  
- print() — Performs an in-order traversal to list users alphabetically

### Design Rationale
An AVL Tree was chosen because it guarantees O(log n) lookup, insertion, and deletion.  
User operations like authentication and registration require fast search and insertion.  
While it uses slightly more memory due to pointers and height fields, the trade-off  
for performance and consistency is acceptable given the small number of users  
in the OFS system.

Memory cleanup is handled through a recursive destructor that deletes all nodes  
when the UserManager instance is destroyed.

---

## 2. Directory Tree Structure

### Overview
The Directory Tree represents the hierarchical structure of all files and folders  
within the Omni File System (OFS). It allows fast traversal, creation, and deletion  
of directories and files using path-based access (e.g., `/Documents/Notes.txt`).

### Structure Design
Each node in the tree is represented by the following structure:

struct FileNode {
string name;
bool isFile;
string data;
FileNode* parent;
vector<FileNode*> children;
};



The Directory Tree is managed by a DirectoryTree class that provides  
operations for directory and file management.

### Operations
- createDirectory(path, name) — Creates a new directory under the given path  
- createFile(path, name, data) — Creates a file and stores its content  
- deleteNode(path) — Deletes a file or directory recursively  
- list(path) — Lists all files and directories inside a given path  
- printTree() — Displays the entire tree structure for debugging

### Design Rationale
A tree-based structure naturally models a hierarchical file system.  
Using a single FileNode type for both files and directories simplifies management.  
Each directory maintains a vector of its children, allowing flexible traversal.  
The structure exists in memory for fast access and will later be serialized  
into the `.omni` container using fixed-width integer types for storage consistency.

---

## 3. Free Space Management

### Overview
The Free Space Management module tracks which data blocks inside the `.omni`  
container are free or occupied. It ensures efficient allocation and reuse of  
disk space when files are created or deleted.

### Structure Design
A bitmap-based approach is used, where each bit (or boolean value) represents a block:

- 0 → Free block  
- 1 → Used block


class FreeSpace {
private:
vector<bool> freeMap;
int totalBlocks;
public:
FreeSpace(int totalBlocks);
int allocateBlock();
void freeBlock(int index);
int getFreeCount() const;
void printMap() const;
void format();
};



### Design Rationale
The bitmap is compact, lightweight, and easy to serialize.  
Each bit represents one block, minimizing memory use.  
Fragmentation is handled at the metadata level through linked block references.  
This makes allocation and deallocation operations efficient and ensures  
reliable management of space inside the `.omni` file.

---

## 4. Path-to-Location Mapping

### Overview
This component maps a file or directory path (e.g., `/Documents/Notes.txt`)  
to its corresponding metadata entry in memory or inside the `.omni` file.

### Design Plan
A hash map or dictionary will be used to associate each full path string  
with its corresponding metadata index. This allows O(1) lookups and direct  
access during read, write, or delete operations.

When serialized, this mapping will help the system locate file data quickly  
without traversing the entire Directory Tree.

---

## 5. .omni File Layout

### Overview
The `.omni` file is the single binary container for the entire OFS.  
It is divided into fixed regions for efficient storage and access.

### Layout Design
