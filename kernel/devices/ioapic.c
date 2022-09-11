#include "apic.h"
#include "ioapic.h"
#include "../interrupts/isr.h"
#include "../kernel.h"
#include "pic.h"
#include "ps2.h"
#include "../lib/print.h"
#include "stddef.h"
#include "stdint.h"

uint32_t readIOAPIC(size_t ioapicaddr, uint32_t reg)
{
    uint32_t volatile* ioapic = (uint32_t volatile*) (uintptr_t) ioapicaddr;
    ioapic[0] = (reg & 0xFF);
    return ioapic[4];
}
 
void writeIOAPIC(size_t ioapicaddr, uint32_t reg, uint32_t value)
{
    uint32_t volatile* ioapic = (uint32_t volatile*) (uintptr_t) ioapicaddr;
    ioapic[0] = (reg & 0xFF);
    ioapic[4] = value;
}

static void keyboardHandler(InterruptFrame* frame) 
{
    printNumber(readDataPort());
    writeAPICRegister(0xB0, 0);
}

void readIOREDTBLs(size_t ioapicaddr) 
{
    char x[20];
    uint32_t IOAPICID = readIOAPIC(ioapicaddr, 0x0);
    uint32_t IOAPICVER = readIOAPIC(ioapicaddr, 0x1);
    printNumber(IOAPICID);
    printNumber(IOAPICVER);
    for (uint8_t i=0; i < 8; i++) 
    {
        term_write("IRQ ", 4);
        printNumber(i);
        uint32_t redirection_entry_1 = readIOAPIC(ioapicaddr, 0x10 + i);
        uint32_t redirection_entry_2 = readIOAPIC(ioapicaddr, 0x11 + i);
        printNumber(redirection_entry_1);
        printNumber(redirection_entry_2);
    }
}

void enableKeyboard(size_t ioapicaddr) 
{
    writeIOAPIC(ioapicaddr, 0x12, 0x21);
    register_interrupt_handler(0x21, &keyboardHandler);
}
