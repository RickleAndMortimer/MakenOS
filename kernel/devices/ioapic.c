#include <stdint.h>
#include <apic.h>
#include <pic.h>
#include <kernel.h>
#include <isr.h>

extern void* irq_stub_table[];
// IOAPIC
uint32_t readIOAPIC(void *ioapicaddr, uint32_t reg)
{
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xFF);
    return ioapic[4];
}
 
void writeIOAPIC(void *ioapicaddr, uint32_t reg, uint32_t value)
{
    uint32_t volatile *ioapic = (uint32_t volatile *)ioapicaddr;
    ioapic[0] = (reg & 0xFF);
    ioapic[4] = value;
}

static void keyboardHandler(interrupt_frame_t* frame) {
    writeAPICRegister(0xB0, 0);
    term_write(".", 1);
}

void readIOREDTBLs(void *ioapicaddr) {
    char x[20];
    uint32_t IOAPICID = readIOAPIC(ioapicaddr, 0x0);
    uint32_t IOAPICVER = readIOAPIC(ioapicaddr, 0x1);
    printNumber(IOAPICID, x);
    printNumber(IOAPICVER, x);
    for (uint8_t i=0; i < 8; i++) {
	term_write("IRQ ", 4);
	printNumber(i, x);
        uint32_t redirection_entry_1 = readIOAPIC(ioapicaddr, 0x10 + i);
        uint32_t redirection_entry_2 = readIOAPIC(ioapicaddr, 0x11 + i);
        printNumber(redirection_entry_1, x);
        printNumber(redirection_entry_2, x);
    }
}

void enableKeyboard(void *ioapicaddr) {
    char x[20];
    //readIOREDTBLs(ioapicaddr);
    uint32_t redirection_entry_1 = readIOAPIC(ioapicaddr, 0x14);
    uint32_t redirection_entry_2 = readIOAPIC(ioapicaddr, 0x15);
    writeIOAPIC(ioapicaddr, 0x10, 0x21);
    printNumber((uint64_t)&irq_stub_table[1], x);
    printNumber(readIOAPIC(ioapicaddr, (uint8_t)&irq_stub_table[1]), x);
    register_interrupt_handler(33, &keyboardHandler);
}
