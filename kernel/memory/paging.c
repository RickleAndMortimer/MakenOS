#include <paging.h>
#include <stdint.h>

PageTable pml4;

void setPageTableEntry(PageEntry* entry, uint8_t flags, uintptr_t physical_address, uint16_t available) 
{
    entry->present = (flags >> 1) & 1;
    entry->writable = (flags >> 2) & 1;
    entry->user_accessible = (flags >> 3) & 1;
    entry->write_through_caching = (flags & 0x8) & 1;
    entry->disable_cache = (flags & 0x10) & 1;
    entry->null = (flags & 0x20) & 1;
    entry->global = (flags & 0x40) & 1;
    entry->avl1 = available & 0x3;
    entry->physical_address = physical_address; 
    entry->avl2 = available >> 3;
    entry->no_execute = flags >> 7;
}

void* getPhysicalAddress(void* virtual_address) 
{
    uintptr_t address = (uintptr_t) virtual_address;

    uint64_t offset = address & 0xFFF;
    uint64_t page_table_index = (address >> 12) & 0x1FF;
    uint64_t page_directory_index = (address >> 21) & 0x1FF;
    uint64_t page_directory_pointer_index = (address >> 30) & 0x1FF;
    uint64_t pml4_index = (address >> 39) & 0x1FF;

    // Check if entry is present in memory
    if (pml4.entries[pml4_index].present) {
	// TODO: handle not present entries
    }
    PageTable* page_directory_pointer = (PageTable*) (uint64_t) (pml4.entries[pml4_index].physical_address);

    if (page_directory_pointer->entries[page_directory_pointer_index].present) {

    }
    PageTable* page_directory = (PageTable*) ((uint64_t) (page_directory_pointer->entries[page_directory_pointer_index].physical_address) << 3);

    if (page_directory->entries[page_directory_index].present) {
	
    } 
    PageTable* page_table = (PageTable*) ((uint64_t) (page_directory->entries[page_directory_index].physical_address) << 3);

    if (page_table->entries[page_table_index].present) {

    }
    return (void*) (page_table->entries[page_table_index].physical_address << 3 + offset);
}

void mapPage(void* physical_address, void* virtual_address, uint8_t flags, uint16_t available) 
{
    // Make sure that both addresses are page-aligned.
    uintptr_t virtual_address_int = (uintptr_t) virtual_address;
    uintptr_t physical_address_int = (uintptr_t) physical_address;

    uint64_t page_table_index = (virtual_address_int >> 12) & 0x1FF;
    uint64_t page_directory_index = (virtual_address_int >> 21) & 0x1FF;
    uint64_t page_directory_pointer_index = (virtual_address_int >> 30) & 0x1FF;
    uint64_t pml4_index = (virtual_address_int >> 39) & 0x1FF;
 
    PageTable* page_directory_pointer = (PageTable*) (uint64_t) (pml4.entries[pml4_index].physical_address);
    PageTable* page_directory = (PageTable*) (uint64_t) (page_directory_pointer->entries[page_directory_pointer_index].physical_address);
    PageTable* page_table = (PageTable*) (uint64_t) (page_directory->entries[page_directory_index].physical_address);

    setPageTableEntry(&(page_table->entries[page_table_index]), flags, physical_address_int, available);

    // Now you need to flush the entry in the TLB

}

