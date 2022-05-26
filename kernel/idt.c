#include <stdint.h>
#include <idt.h>
#include <pic.h>
#include <isr.h>

extern void* isr_stub_table[];
extern void* irq_stub_table[];

idt64_entry_t idt_entries[256];

idt64_ptr_t idt_ptr = (idt64_ptr_t) {
    (uint16_t)sizeof(idt_entries) - 1,
    (uintptr_t)&idt_entries[0]
};

void setIdtEntry(idt64_entry_t *target, uint64_t offset, uint16_t selector, uint8_t ist, uint8_t type_attributes) {
	target->offset_1 = offset & 0xFFFF;
	target->selector = selector;
	target->ist = ist;
	target->type_attributes = type_attributes;
	target->offset_2 = (offset >> 16) & 0xFFFF;
	target->offset_3 = (offset >> 32) & 0xFFFFFFFF;
	target->zero = 0;
}
void initIdt() {
    	//remapPIC(0x20, 0x28);
	for (uint8_t i = 0; i < 32; i++) {
		setIdtEntry(&idt_entries[i], (uint64_t)isr_stub_table[i], 0x28, 0, 0x8F);
	}
	//for (uint8_t i = 0; i < 16; i++) {
	//	setIdtEntry(&idt_entries[i+32], (uint64_t)irq_stub_table[i], 0x28, 0, 0x8F);
	//}
	asm volatile("lidt %0" : : "m"(idt_ptr));
	asm volatile("sti");
}
