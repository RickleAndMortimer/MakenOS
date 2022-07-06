#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stivale2.h>
#include <print.h>
#include <kernel.h>

#define MAX_SIZE 4096
extern struct stivale2_struct_tag_memmap* memmap_tag;

void printMemoryMaps() 
{
    char x[20];
    for (size_t i = 0; i < memmap_tag->entries; i++) {
	switch (memmap_tag->memmap[i].type) {
	    case 1:	
	    case 0x1000:
	    case 3:
		term_write("Entry ", 6);
	        printNumber(i, x);
	        term_write("Type ", 5);
	        printNumber(memmap_tag->memmap[i].type, x);
	        term_write("Base ", 5);
	        printNumber(memmap_tag->memmap[i].base, x);
	        term_write("Length ", 7);
	        printNumber(memmap_tag->memmap[i].length, x);
		break;
	}
    }
}

uint64_t* allocatePhysicalMemory(uint64_t* base, size_t length, uint64_t* memory, size_t allocation_size) {
    if (length <= MAX_SIZE && *base != 0) {
        return NULL;
    }
    size_t half = length / 2;
    // Allocate if the current length is enough and unallocated
    if (half <= allocation_size && *base == 0) {
	return memcpy(base, memory, allocation_size);
    }
    // Try to find another block
    else if (half > allocation_size) {
	uint64_t* left = allocatePhysicalMemory(base, half, memory, allocation_size);
	return left ? left : allocatePhysicalMemory(base + half, half, memory, allocation_size);
    }
    // Otherwise, the memory cannot be allocated
    return NULL;
}
