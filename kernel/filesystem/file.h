#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stddef.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 

struct dirent {
    char name[128];    
    uint32_t ino;
};

typedef struct fs_node {
    uint32_t flags;
    uint32_t mask;        // The permissions mask.
    uint32_t uid;         // The owning user.
    uint32_t gid;         // The owning group.
    uint32_t inode;       // This is device-specific - provides a way for a filesystem to identify files.
    uint32_t length;      // Size of the file, in bytes.
    uint32_t impl;        // An implementation-defined number.
    size_t (*read)(struct fs_node*, size_t, size_t, uint8_t*);
    size_t (*write)(struct fs_node*, size_t, size_t, uint8_t*);
    void (*open)(struct fs_node*);
    void (*close)(struct fs_node*);
    struct dirent* (*readdir)(struct fs_node*, size_t);
    struct fs_node* (*finddir)(struct fs_node*, char*);
    struct fs_node* ptr;
} fs_node_t;

extern fs_node_t* fs_root; // The root of the filesystem.

size_t read_fs(fs_node_t* node, size_t offset, uint32_t size, uint8_t* buffer);
size_t write_fs(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer);
void open_fs(fs_node_t* node, uint8_t read, uint8_t write);
void close_fs(fs_node_t* node);
struct dirent* readdir_fs(fs_node_t* node, size_t index);
fs_node_t* finddir_fs(fs_node_t* node, char* name);

#endif
