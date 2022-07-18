#include <stdint.h>
#include <stddef.h>

void printMemoryMaps();
void* malloc(size_t size);
void free(void* ptr);
void setMemoryMap(uint8_t selection);
void* getMemoryMapBase();
uint64_t getMemoryMapLength();
