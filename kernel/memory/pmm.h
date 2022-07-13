#include <stdint.h>
#include <stddef.h>

void printMemoryMaps();
uint64_t* allocatePhysicalMemory(uint64_t* base, size_t length, size_t allocation_size);
