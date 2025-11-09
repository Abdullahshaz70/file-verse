### 1. User Management Structure

Each user is represented by:
- username: string
- password: string (hashed)
- isAdmin: boolean

To store users, a **self-balanced binary search tree (AVL Tree)** is used.
This structure provides O(log n) insertion, deletion, and search,
which keeps authentication and user lookup fast even as the number of users grows.

Although AVL trees require slightly more memory per node than arrays or vectors,
the total overhead is minimal because the user list is small relative to the
overall .omni file size. The tradeoff of a few extra pointers per node is worth
the logarithmic time performance for add/remove/authenticate operations.

The tree can later be serialized into the .omni file for persistence.



## 2. Directory Tree Structure

## 3. Free Space Management

## 4. Path-to-Location Mapping

## 5. .omni File Layout
