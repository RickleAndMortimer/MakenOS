#include <stdint.h>
#include <stddef.h>

void printMemoryMaps();
uint64_t* allocatePhysicalMemory(uint64_t* base, size_t length, size_t allocation_size);
void setMemoryMap(uint8_t selection);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();
