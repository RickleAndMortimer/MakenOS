#include <stdint.h>
#include <stddef.h>

void printMemoryMaps();
void setMemoryMap(uint8_t selection);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();

void* k_malloc();
void k_free(void* ptr);
