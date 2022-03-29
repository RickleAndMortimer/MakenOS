#include <stdint.h>

struct idt64_entry {
	uint16_t offset_1;        // offset bits 0..15
	uint16_t selector;        // a code segment selector in GDT or LDT
	uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	uint8_t  type_attributes; // gate type, dpl, and p fields
	uint16_t offset_2;        // offset bits 16..31
	uint32_t offset_3;        // offset bits 32..63
	uint32_t zero;            // reserved
};
typedef struct idt64_entry idt64_entry_t;

struct idt64_ptr {
	uint64_t offset;
	uint16_t size;
}__attribute__((packed));
typedef struct idt64_ptr idt64_ptr_t;

uint16_t idt_entries[256];
idt64_ptr_t idt_ptr;

void encodeIdt(uint16_t *target, idt64_entry_t entry) {
	target[0] = entry.offset_1;
	target[1] = entry.selector;
	target[2] = entry.type_attributes << 8 | entry.ist;
	target[3] = entry.offset_2;
	target[4] = entry.offset_3 & 0xFFFF;
	target[5] = entry.offset_3 >> 16; 
	target[6] = entry.zero & 0xFFFF;
	target[7] = entry.zero >> 16;
}

void initIdt() {
	idt_ptr.offset = (uint64_t)&idt_entries;
	idt_ptr.size = sizeof(idt_entries) - 1;
	asm volatile("lidt %0" : : "m"(idt_ptr));
}
