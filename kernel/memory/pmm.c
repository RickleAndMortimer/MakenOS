#include "pmm.h"
#include "../lib/string.h"
#include "../lib/print.h"
#include "../kernel.h"
#include "../limine.h"

extern struct limine_memmap_response* memmap_info;
static struct limine_memmap_entry* memmap;

static const char* getMemoryMapType(uint32_t type) 
{
    switch (type) 
    {
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
    for (size_t i = 0; i < memmap_info->entry_count; i++) 
    {
        struct limine_memmap_entry* entry = memmap_info->entries[i];
		switch (entry->type) 
        {
			case 1:	
			case 0x1000:
			case 3:
				term_write("Entry ", 6);
				printNumber(i);

				const char* type = getMemoryMapType(entry->type);
				term_write(type, strlen(type));

				term_write("\nBase ", 6);
				printNumber(entry->base);

				term_write("Length ", 7);
				printNumber(entry->length);
                break;
		}
    }
}

void setMemoryMap(uint8_t selection) 
{
	memmap = memmap_info->entries[selection];
}

void* getMemoryMapBase() 
{
	return (void*) memmap->base;
}

uint64_t getMemoryMapLength()
{
	return memmap->length;
}

static void* b_malloc(uint64_t* base, size_t length, size_t size) 
{
    if (length <= BLOCK_SIZE) {
        if (size + 1 <= length && *((uint64_t*) base) == 0) {
            *base = size;
            memset(base + 1, 0, sizeof(void*) * size);
            return (void*) (base + 1);
        }
        else {
            return NULL;
        }
    }

    size_t half = length / 2;
    // Allocate if the current length is enough and unallocated
    if (half <= size + 1 && *((uint64_t*) base) == 0) {
        *base = size;
        memset(base + 1, 0, sizeof(void*) * size);
		return (void*) (base + 1);
    }
    // Try to find another block
    else if (half > size) 
    {
		void* b = b_malloc(base, half, size);
        // If alloc is null, search blocks on the right side
        if (b == NULL) {
            b = b_malloc(base + half, half, size);
        }

        return b;
    }
    // Otherwise, the memory cannot be allocated
    return NULL;
}

void* k_malloc(size_t size) 
{
    return b_malloc((uint64_t*)memmap->base, memmap->length, size);
}

void k_free(void* base) 
{
    uint64_t size = *(((uint64_t*) base) - 1);
    memset(base, 0, size);
}
