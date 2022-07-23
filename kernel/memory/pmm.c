#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stivale2.h>
#include <print.h>
#include <kernel.h>

#define BLOCK_SIZE 512

extern struct stivale2_struct_tag_memmap* memmap_tag;
static struct stivale2_mmap_entry* memmap;

static const char* getMemoryMapType(uint32_t type) {
    switch (type) {
        case 0x1:
            return "Usable RAM";
        case 0x2:
            return "Reserved";
        case 0x3:
            return "ACPI reclaimable";
        case 0x4:
            return "ACPI NVS";
        case 0x5:
            return "Bad memory";
        case 0x1000:
            return "Bootloader reclaimable";
        case 0x1001:
            return "Kernel/Modules";
        case 0x1002:
            return "Framebuffer";
        default:
            return "???";
    }
}

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

				const char* type = getMemoryMapType(memmap_tag->memmap[i].type);
				term_write(type, strlen(type));

				term_write("\nBase ", 6);
				printNumber(memmap_tag->memmap[i].base, x);

				term_write("Length ", 7);
				printNumber(memmap_tag->memmap[i].length, x);
			break;
		}
    }
}

void setMemoryMap(uint8_t selection) 
{
	memmap = &(memmap_tag->memmap[selection]);
}

void* getMemoryMapBase() 
{
	return (void*) memmap->base;
}

uint64_t getMemoryMapLength()
{
	return memmap->length;
}

void* k_malloc(uint64_t* base, size_t length, size_t size) {
    if (length <= BLOCK_SIZE && *base != 0) {
        return NULL;
    }
    size_t half = length / 2;
    // Allocate if the current length is enough and unallocated
    if (half <= size && *base == 0) {
		return base;
    }
    // Try to find another block
    else if (half > size) {
		uint64_t* left = k_malloc(base, half, size);
		return left ? left : k_malloc(base + half, half, size);
    }
    // Otherwise, the memory cannot be allocated
    return NULL;
}

void k_free(void* base) 
{
	uint64_t* ptr = base;
	for (size_t i = 0; i < BLOCK_SIZE; i++) {
		ptr[i] = 0;
	}
}
