#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE 4096

void printMemoryMaps();
void setMemoryMap(uint8_t selection);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();

void* k_malloc();
void k_free(void* ptr);

#endif
