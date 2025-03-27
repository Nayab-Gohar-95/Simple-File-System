# Simple File System

## Overview
This project is a **Simple File System** implemented in C. It simulates a basic file system with features like file creation, deletion, reading, writing, truncation, and directory management. The system uses a **File Allocation Table (FAT)** approach to manage file storage within a virtual disk.

## Features
- Create and delete files
- Read and write data to files
- Truncate files to a specified size
- Create and delete directories
- List directory contents
- Uses a **64MB virtual disk** with **1KB block size**

## Installation & Usage
### 1. Compile the program
Ensure you have `gcc` installed, then compile:
```sh
gcc file_system.c -o simple_fs
```

### 2. Run the program
```sh
./simple_fs
```

### 3. File System Operations
Once running, you can use commands like:
```
createFile <name> <size>  # Creates a file with given size in blocks
deleteFile <name>         # Deletes a specified file
writeFile <name>          # Writes data to the file
readFile <name>           # Reads file contents
truncateFile <name> <size> # Truncates file to a new size
createDir <name>         # Creates a directory
deleteDir <name>         # Deletes a directory
list                     # Lists all files and directories
exit                     # Exits the file system
```

## Implementation Details
- **Virtual Disk:** Implemented as a `virtualDisk.bin` file.
- **FAT Table:** Manages block allocation using linked-list structure.
- **File Operations:** Read/write operations use block-based storage.
- **Directory Structure:** Stored within an array-based directory table.

## Future Improvements
- Implement a **journaled file system** for better recovery.
- Add **user permissions** and file metadata.
- Enhance **file search and indexing** for performance.

