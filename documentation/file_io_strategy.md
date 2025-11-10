# Omni File System (OFS)
## Phase-2 — File I/O Strategy and Persistent Storage Layer

### 📘 Overview
This document explains the design and working of the **File I/O (Persistence) Layer** of the Omni File System (OFS).  
The layer is responsible for reading, writing, and maintaining all file system data inside a single binary container file called `.omni`.

The I/O system ensures that:
- Data stored in memory by `OFSCore` (users, directories, free blocks) is written to disk safely.
- The file system can be re-opened later, restoring all structures from the `.omni` file.
- All writes are **block-based** and aligned to the structure sizes defined in `odf_types.hpp`.

---

## 🧱 File Layout Structure

The `.omni` file is divided into fixed regions as defined in the project specification:

+---------------------------+ 0x0000
| OMNI Header (512 bytes) | ← General info and offsets
+---------------------------+
| User Table (N * 128 B) | ← User accounts (UserInfo)
+---------------------------+
| Metadata Area | ← FileEntry & Directory info
+---------------------------+
| Free Space Bitmap | ← 1 bit per block
+---------------------------+
| Data Blocks | ← Actual file data
+---------------------------+


Each section is written and read using **binary streams (`fstream`)** for fast, offset-based I/O.  
The layout and struct sizes are standardized through the provided header file `odf_types.hpp`.

---

## ⚙️ Implementation — `FileIOManager`

The `FileIOManager` class handles all interactions with the `.omni` file.  
It is implemented as a header-only C++ class in `/source/include/core/file_io_manager.hpp`.

### **Responsibilities**
| Function | Description |
|-----------|-------------|
| `createOmniFile()` | Creates a new `.omni` binary file and fills it with zero bytes. |
| `openFile()` | Opens an existing `.omni` file for read/write operations. |
| `writeHeader()` | Writes the `OMNIHeader` structure to the start of the file. |
| `readHeader()` | Reads the `OMNIHeader` from disk back into memory. |
| `writeBlock()` | Writes binary data into a specific data block by index. |
| `readBlock()` | Reads binary data from a specific data block by index. |
| `closeFile()` | Closes and flushes the active file stream. |

---

## 🧩 Binary I/O Method

All I/O operations are performed in **binary mode** to preserve structure integrity.

Example: Writing the header to disk
```cpp
file.seekp(0, ios::beg);
file.write(reinterpret_cast<const char*>(&header), sizeof(OMNIHeader));
