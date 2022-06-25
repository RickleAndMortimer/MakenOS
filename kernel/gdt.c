#include <stdint.h>
#include <gdt.h>

SegmentDescriptor entries[6];

void setGdtEntry(SegmentDescriptor *entry, uint32_t base, uint8_t flags, uint8_t access_byte, uint16_t limit) 
{
    entry->base = base >> 24;
    entry->base_high = base >> 16 & 0xFF;
    entry->base_low = base & 0xFFFF;
    entry->flags = flags;
    entry->access_byte = access_byte;
    entry->limit_low = limit;
}

void setSystemEntry(SegmentDescriptor *entry_1, SegmentDescriptor *entry_2, uint64_t base, uint8_t flags, uint8_t access_byte, uint16_t limit) 
{
    entry_1->base = 0; 
    entry_1->flags = 0; 
    entry_1->access_byte = 0; 
    entry_1->base_high = 0; 
    entry_1->base_low = base >> 48; 
    entry_1->limit_low = base >> 32 & 0xFFFF; 

    entry_2->base = base >> 24 & 0xFF; 
    entry_2->flags = flags; 
    entry_2->access_byte = access_byte; 
    entry_2->base_high = base >> 8 & 0xFF; 
    entry_2->base_low = base & 0xFFFF; 
    entry_2->limit_low = limit; 
} 

void initGDT() 
{
    setGdtEntry(&entries[0], 0, 0, 0, 0);
    setGdtEntry(&entries[1], 0, 0xCF, 0x9A, 0xFFFF);
    setGdtEntry(&entries[2], 0, 0xCF, 0x92, 0xFFFF);
    setGdtEntry(&entries[3], 0, 0xCF, 0xFA, 0xFFFF);
    setGdtEntry(&entries[4], 0, 0xCF, 0xF2, 0xFFFF);
    asm volatile ("lgdt %0" :: "m"(entries));
}
