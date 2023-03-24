#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE 4096

typedef struct Node {
    void* base;
    size_t length;
    struct Node* next;
} Node;

typedef struct alloc_entry {
    size_t size;
    void* base;
} alloc_entry;

void printMemoryMaps();
void setMemoryMap(uint8_t selection);
void* printHeader(void* start);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();

void* k_malloc();
void k_free(void* ptr);

#endif
