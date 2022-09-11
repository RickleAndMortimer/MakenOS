#include <stdint.h>
#include <stddef.h>


typedef struct vfs 
{
    uint16_t id;
    uint64_t length;
    uint64_t inode_len;
    uint64_t data_blocks;
} vfs;

typedef struct inode 
{
    uint16_t mode; 
    uint16_t uid; 
    uint32_t size; 
    uint32_t time; 
    uint32_t ctime; 
    uint32_t mtime; 
    uint32_t dtime; 
    uint16_t gid; 
    uint16_t links_count; 
    uint32_t blocks; 
    uint32_t flags; 
    uint32_t osd1; 
    void* block[15]; 
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
} inode;

typedef struct directory_entry 
{
    char name[256];
    uint16_t inum;
    uint32_t rec_size;
    uint32_t strlen;
} directory_entry;

typedef struct inode_table {
    vfs v;
    uint16_t inode_bitmap;
    uint64_t dnode_bitmap;
    inode inodes[16]; 
} inode_table;

inode* fopen(char* filename);
int fwrite(inode* i, const char* data, size_t len);
int fread(inode* i, char* buffer, size_t blocks, size_t length);
inode_table* initRamFS();
