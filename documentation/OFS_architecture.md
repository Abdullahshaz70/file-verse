# 🧩 Omni File System (OFS) — Architecture Overview

## 1. Overview
The Omni File System (OFS) is a simulated block-based file system implemented in C++.  
It models all major file system components — headers, free space maps, user management, directories, and data blocks —  
while supporting persistent storage, user access control, and version history.

---

## 2. Core Components

| Component | Description |
|------------|-------------|
| **OFSCore** | Central orchestrator managing header layout, files, users, and versions. |
| **FileIOManager** | Handles low-level binary I/O with `.omni` file (header, users, blocks). |
| **UserManager (AVL)** | Stores users in an AVL tree for O(log n) operations. |
| **SessionManager** | Controls login/logout and enforces admin privileges. |
| **DirectoryTree** | Maintains hierarchical metadata for files and folders. |
| **FreeSpace** | Manages block allocation, marking used/free blocks. |

---

## 3. Persistent File Layout

| Section | Description | Typical Offset |
|----------|--------------|----------------|
| Header | File metadata, size, and layout | 0 |
| User Table | 10 user records | 504 |
| Free Space Map | One byte per block | 1944 |
| Directory Metadata | Serialized directory tree | 2200 |
| Data Blocks | Actual file data | 2584+ |
| Version Storage | VersionBlocks for DeltaVault | ~943976 |
| Change Log | DeltaAudit change records | ~1015656 |

---

## 4. Versioning (DeltaVault)

- Each `writeFileContent()` creates a new version block.
- Each `VersionBlock` stores:
  - file path
  - version ID (timestamp)
  - block index
  - creation time
- Versions are persisted and viewable via `listVersions()`.
- `revertToVersion()` restores previous data from version blocks.

---

## 5. Access Control (RBAC)

| Role | Permissions |
|-------|--------------|
| **Admin** | Format FS, create users, view all stats |
| **User** | Read/write files, create versions |
| **Guest** | (Not implemented) |

---

## 6. Directory Tree

- Each node represents either a directory or file.
- Directory metadata is exported to disk as `FileEntry[]`.
- Supports creation, deletion, and traversal of nested paths.

---

## 7. Error Handling

- All operations verify open file state.
- Header magic validation prevents corruption.
- Logged output includes ✅ / ❌ markers for traceability.

---

## 8. Testing Summary
- Verified on Ubuntu 24.04 with g++17.
- `.omni` file validated via `verifyFileStructure()`.
- All operations (format, write, read, version, revert) successful.

---

## 9. Next Phase
- Introduce **socket-based remote access** to OFS.
- Implement client-server synchronization of `.omni` operations.
