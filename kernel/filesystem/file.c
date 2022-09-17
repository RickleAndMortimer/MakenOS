#include "file.h"
#include "../lib/string.h"
#include "../memory/pmm.h"

static inode_table table;

inode_table* initRamFS() 
{
    table.v.id = 1;
    table.v.length = sizeof(table);
    table.v.inode_len = sizeof(table.inodes);
    table.v.data_blocks = sizeof(56 * 4096);
    // root directory
    table.inodes[0] = (inode) { 0,0,0,0,0,0,0,0,0,0,0,0 };
    return &table; 
}

inode* fopen(char* filename) 
{
    for (size_t i = 0; i < 16; i++) 
    {
        if (!((table.inode_bitmap >> i) & 1)) 
        {
            table.inode_bitmap |= (1 << i);
            return &table.inodes[i]; 
        }
    }
    return NULL;
}

int fwrite(inode* node, char* data, size_t len) 
{
    memcpy(&node->block[0], data, len);
    return 1;
}

int fread(inode* node, char* buffer, size_t blocks, size_t length) 
{
    for (size_t i = 0; i < blocks; i++) 
    {
        if (node->block[i] == NULL) 
        {
            continue;
        }
        memcpy(buffer, &node->block[i], length);
    }
    return 1; 
}

void* f_malloc(inode* node, size_t block_index) 
{
    for (size_t i = 0; i < 56; i++) 
    {
        if (!((table.dnode_bitmap >> i) & 1)) 
        {
            table.dnode_bitmap |= (1 << i);
            node->block[block_index] = k_malloc();
            return node->block[block_index];
        }
    }
    return NULL;
}
