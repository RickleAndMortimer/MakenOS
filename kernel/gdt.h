#include <stdint.h>

typedef struct SegmentDescriptor
{
    uint8_t base;
    uint8_t flags;
    uint8_t access_byte;
    uint8_t base_high;
    uint16_t base_low;
    uint16_t limit_low;
}__attribute__((packed)) SegmentDescriptor;

typedef struct GDTPointer
{
    uint64_t gdt_address;
    uint16_t limit;
}__attribute__((packed)) GDTPointer;

void setEntry(SegmentDescriptor* entry, uint32_t base, uint8_t flags, uint8_t access_byte, uint16_t limit);
void setSystemEntry(SegmentDescriptor* entry_1, SegmentDescriptor* entry_2, uint64_t base, uint8_t flags, uint8_t access_byte, uint16_t limit);
