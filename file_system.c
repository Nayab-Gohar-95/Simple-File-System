#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DIRECTORY_SIZE 128
#define MAX_FILE_NAME_SIZE 64
#define MAX_FILE_SIZE 128   // Blocks
#define BLOCK_SIZE 1024     // Bytes
#define TOTAL_DISK_SIZE (64 * 1024 * 1024) // 64 MB
#define TOTAL_BLOCKS (TOTAL_DISK_SIZE / BLOCK_SIZE)


int FAT[TOTAL_BLOCKS];

typedef struct {
    char fileName[MAX_FILE_NAME_SIZE];
    int startBlock;
    int size; // In blocks
    int isDirectory;
} DirectoryEntry;


DirectoryEntry directory[DIRECTORY_SIZE];


FILE *diskFile;

void writeBlock(int blockNum, const void *buffer) {
    if (blockNum < 0 || blockNum >= TOTAL_BLOCKS) {
        printf("Error: Block number %d is out of range.\n", blockNum);
        return;
    }
    fseek(diskFile, blockNum * BLOCK_SIZE, SEEK_SET);
    if (fwrite(buffer, BLOCK_SIZE, 1, diskFile) != 1) {
        perror("Error writing to block");
    }
}

void readBlock(int blockNum, void *buffer) {
    if (blockNum < 0 || blockNum >= TOTAL_BLOCKS) {
        printf("Error: Block number %d is out of range.\n", blockNum);
        return;
    }
    fseek(diskFile, blockNum * BLOCK_SIZE, SEEK_SET);
    if (fread(buffer, BLOCK_SIZE, 1, diskFile) != 1) {
        perror("Error reading from block");
    }
}

void createAndFormatPartition() {

    diskFile = fopen("virtualDisk.bin", "wb+");
    if (!diskFile) {
        perror("Failed to create or open the virtual disk file");
        exit(1);
    }

    memset(FAT, -1, sizeof(FAT));

    memset(directory, 0, sizeof(directory));

    writeBlock(0, FAT); 
    writeBlock(1, directory); 

    printf("Partition created and formatted successfully.\n");
}

void loadPartition() {

    readBlock(0, FAT);

    readBlock(1, directory);

    printf("Partition loaded successfully.\n");
}

void savePartition() {

    writeBlock(0, FAT);

    writeBlock(1, directory);

    printf("Partition saved successfully.\n");
}

void createFile(const char *name, int size) {
    if (size > MAX_FILE_SIZE) {
        printf("Error: File size exceeds the maximum allowed size.\n");
        return;
    }

    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (directory[i].fileName[0] == '\0') { // Empty entry
            strncpy(directory[i].fileName, name, MAX_FILE_NAME_SIZE);
            directory[i].size = size;
            directory[i].isDirectory = 0;

            int prevBlock = -1;
            for (int j = 0; j < size; j++) {
                for (int k = 0; k < TOTAL_BLOCKS; k++) {
                    if (FAT[k] == -1) { 
                        FAT[k] = -2; 
                        if (prevBlock != -1) {
                            FAT[prevBlock] = k;
                        } else {
                            directory[i].startBlock = k;
                        }
                        prevBlock = k;
                        break;
                    }
                }
            }

            printf("File '%s' created successfully.\n", name);
            return;
        }
    }
    printf("Error: Directory is full.\n");
}

void deleteFile(const char *name) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (strcmp(directory[i].fileName, name) == 0 && !directory[i].isDirectory) {
            int block = directory[i].startBlock;
            while (block != -1) {
                int nextBlock = FAT[block];
                FAT[block] = -1; 
                block = nextBlock;
            }
            memset(&directory[i], 0, sizeof(DirectoryEntry));
            printf("File '%s' deleted successfully.\n", name);
            return;
        }
    }
    printf("Error: File not found.\n");
}

void writeFile(const char *name, const char *data, int dataSize) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (strcmp(directory[i].fileName, name) == 0 && !directory[i].isDirectory) {
            if (dataSize > directory[i].size * BLOCK_SIZE) {
                printf("Error: Data size exceeds file capacity.\n");
                return;
            }

            int block = directory[i].startBlock;
            int offset = 0;
            while (block != -1 && offset < dataSize) {
                char buffer[BLOCK_SIZE] = {0};
                int chunkSize = (dataSize - offset > BLOCK_SIZE) ? BLOCK_SIZE : (dataSize - offset);
                memcpy(buffer, data + offset, chunkSize);
                writeBlock(block, buffer);
                offset += chunkSize;
                block = FAT[block];
            }

            printf("Data written to file '%s'.\n", name);
            return;
        }
    }
    printf("Error: File not found.\n");
}

void readFile(const char *name) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (strcmp(directory[i].fileName, name) == 0 && !directory[i].isDirectory) {
            int block = directory[i].startBlock;
            printf("Reading data from file '%s':\n", name);

            while (block != -1) {
                char buffer[BLOCK_SIZE] = {0};
                readBlock(block, buffer);
                printf("%s", buffer);
                block = FAT[block];
            }

            printf("\nEnd of file.\n");
            return;
        }
    }
    printf("Error: File not found.\n");
}

void truncateFile(const char *name, int newSize) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (strcmp(directory[i].fileName, name) == 0 && !directory[i].isDirectory) {
            if (newSize > directory[i].size) {
                printf("Error: New size exceeds current file size.\n");
                return;
            }

            int block = directory[i].startBlock;
            int prevBlock = -1;
            int blockCount = 0;

            while (block != -1) {
                if (blockCount >= newSize) {
                    int nextBlock = FAT[block];
                    FAT[block] = -1;
                    block = nextBlock;
                } else {
                    prevBlock = block;
                    block = FAT[block];
                    blockCount++;
                }
            }

            if (prevBlock != -1) {
                FAT[prevBlock] = -1;
            }

            directory[i].size = newSize;
            printf("File '%s' truncated to %d blocks.\n", name, newSize);
            return;
        }
    }
    printf("Error: File not found.\n");
}

void createDirectory(const char *name) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (directory[i].fileName[0] == '\0') { // Empty entry
            strncpy(directory[i].fileName, name, MAX_FILE_NAME_SIZE);
            directory[i].isDirectory = 1;
            printf("Directory '%s' created successfully.\n", name);
            return;
        }
    }
    printf("Error: Directory is full.\n");
}

void deleteDirectory(const char *name) {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (strcmp(directory[i].fileName, name) == 0 && directory[i].isDirectory) {
            for (int j = 0; j < DIRECTORY_SIZE; j++) {
                if (directory[j].fileName[0] != '\0' && 
                    strcmp(directory[j].fileName, name) != 0 &&
                    directory[j].isDirectory == 0) {
                    printf("Error: Directory '%s' is not empty.\n", name);
                    return;
                }
            }

            memset(&directory[i], 0, sizeof(DirectoryEntry));
            printf("Directory '%s' deleted successfully.\n", name);
            return;
        }
    }
    printf("Error: Directory not found.\n");
}

void listDirectory() {
    printf("Directory contents:\n");
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
        if (directory[i].fileName[0] != '\0') {
            printf("%s %s\n", directory[i].fileName, directory[i].isDirectory ? "(Directory)" : "(File)");
        }
    }
}

// Console Interface
void consoleInterface() {
    char command[256];
    char name[MAX_FILE_NAME_SIZE];
    char data[BLOCK_SIZE * 2]; 
    int size;

    printf("\n--- File System Operations Menu ---\n");
    printf("1. createFile <name> <size>\n");
    printf("2. deleteFile <name>\n");
    printf("3. writeFile <name>\n");
    printf("4. readFile <name>\n");
    printf("5. truncateFile <name> <size>\n");
    printf("6. createDir <name>\n");
    printf("7. deleteDir <name>\n");
    printf("8. list\n");
    printf("9. exit\n");
    printf("-----------------------------------\n");

    while (1) {        
        printf("Enter your command: ");
        scanf("%s", command);

        if (strcmp(command, "createFile") == 0) {
            scanf("%s %d", name, &size);
            createFile(name, size);
        } else if (strcmp(command, "deleteFile") == 0) {
            scanf("%s", name);
            deleteFile(name);
        } else if (strcmp(command, "writeFile") == 0) {
            scanf("%s", name);
            printf("Enter data to write: ");
            scanf(" %[^\n]", data); 
            writeFile(name, data, strlen(data));
        } else if (strcmp(command, "readFile") == 0) {
            scanf("%s", name);
            readFile(name);
        } else if (strcmp(command, "truncateFile") == 0) {
            scanf("%s %d", name, &size);
            truncateFile(name, size);
        } else if (strcmp(command, "createDir") == 0) {
            scanf("%s", name);
            createDirectory(name);
        } else if (strcmp(command, "deleteDir") == 0) {
            scanf("%s", name);
            deleteDirectory(name);
        } else if (strcmp(command, "list") == 0) {
            listDirectory();
        } else if (strcmp(command, "exit") == 0) {
            printf("Exiting file system. Goodbye!\n");
            break;
        } else {
            printf("Unknown command. Please select from the menu.\n");
        }
    }
}

int main() {
    int choice;

    printf("\n=========================\n");
    printf("  File System Simulator  \n");
    printf("=========================\n");
    printf("This program simulates a file system with the following features:\n");
    printf("- Create and manage files and directories\n");
    printf("- Simulate a File Allocation Table (FAT)\n");
    printf("- Perform read, write, truncate operations\n");
    printf("- List directory contents\n\n");

    while (1) {
        printf("\nMain Menu:\n");
        printf("1. Create and Format Partition\n");
        printf("2. Load Existing Partition\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            createAndFormatPartition();
            break;
        } else if (choice == 2) {
            diskFile = fopen("virtualDisk.bin", "rb+");
            if (!diskFile) {
                perror("Failed to open existing virtual disk file");
                continue; // Go back to the menu
            }
            loadPartition();
            break;
        } else if (choice == 3) {
            printf("Exiting file system. Goodbye!\n");
            return 0;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    consoleInterface();
    savePartition();
    fclose(diskFile);
    return 0;
}
