#include <stdint.h>
#include <stddef.h>

typedef struct VFS {
    int magic_no; // identifies the fs
    int (open*)(char* path, int flags);
    ssize_t (read*)(int fd, char* buf, size_t bytes);
    ssize_t (write*)(int fd, char* buf);
    int (memmap*)(int fd, char* buf);
} VFS;
