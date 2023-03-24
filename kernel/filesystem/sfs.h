#include <stdbool.h>
#include <stdint.h>

struct super {
    uint16_t inodes;  
    uint16_t blocks;  
    uint16_t id;
}__attribute__((aligned(4096)));

struct inode {                                                                  
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
}__attribute__((aligned(256)));      

struct table {
    struct super s;
    bool inode_bitmap[4096];
    bool dnode_bitmap[4096];
    inode inodes[80];
    uint8_t data[4096 * 56];
};

struct directory_entry {
    uint8_t inum;   
    uint8_t reclen;   
    uint8_t strlen;   
    char name[128];
};
