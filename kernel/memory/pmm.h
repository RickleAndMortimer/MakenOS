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

typedef struct List {
    size_t size;
    Node* head;
} List;

void printMemoryMaps();
void setMemoryMap(uint8_t selection);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();

void* k_malloc();
void k_free(void* ptr);

#endif
