#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MAX_SIZE 512

uint64_t* allocatePhysicalMemory(uint64_t* base, size_t length, uint64_t memory[], size_t allocation_size) {
    if (length <= MAX_SIZE && *base != 0) {
        return NULL;
    }
    size_t half = length / 2;
    // Allocate if the current length is enough and unallocated
    if (half <= allocation_size && *base == 0) {
	return memcpy(base, memory, allocation_size*8);
    }
    // Try to find another block
    else if (half > allocation_size) {
	uint64_t* left = allocatePhysicalMemory(base, half, memory, allocation_size);
	return left ? left : allocatePhysicalMemory(base + half, half, memory, allocation_size);
    }
    // Otherwise, the memory cannot be allocated
    return NULL;
}
