#include <stdint.h>

typedef struct {
    uint8_t base;
    uint8_t flags;
    uint8_t access_byte;
    uint8_t base_high;
    uint16_t base_low;
    uint16_t limit_low;
}__attribute__((packed)) segment_descriptor_t;

typedef struct {
    uint64_t gdt_address;
    uint16_t limit;
}__attribute__((packed)) gdtr_t;

void setEntry(segment_descriptor_t *entry, uint32_t base, uint8_t flags, uint8_t access_byte, uint16_t limit);

void setSystemEntry(segment_descriptor_t *entry_1, segment_descriptor_t *entry_2, uint64_t base, uint8_t flags, uint8_t access_byte, uint16_t limit);
