#include "apic.h"
#include "ioapic.h"
#include "../interrupts/isr.h" 
#include "../kernel.h"
#include "pic.h"
#include "ps2.h"
#include "serial.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include <stdbool.h>

extern char* set1_scancodes[];
extern char* shift_set1_scancodes[];

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

static bool shift = false;
static bool caps_lock = false;
static bool ctrl = false;

static void keyboardHandler(InterruptFrame* frame) 
{
    size_t input = readDataPort();
    if (input == 0x2A || input == 0x36) 
        shift = true;    

    else if (input == 0x3A) 
        caps_lock ^= true;    

    else if (input == 0x1D) 
        ctrl = true;   

    else if (input < 0x58) 
    {
        if (ctrl)
            term_write("^", 1);

        if (shift ^ caps_lock) 
            term_write(shift_set1_scancodes[input], strlen(shift_set1_scancodes[input])); 
        else 
            term_write(set1_scancodes[input], strlen(set1_scancodes[input]));
    }

    else if (input == 0xAA || input == 0xB6) 
        shift = false;    

    else if (input == 0x9D) 
        ctrl = false;   
   
    writeAPICRegister(0xB0, 0);
}

void readIOREDTBLs(size_t ioapicaddr) 
{
    uint32_t IOAPICID = readIOAPIC(ioapicaddr, 0x0);
    uint32_t IOAPICVER = readIOAPIC(ioapicaddr, 0x1);
    printNumber(IOAPICID);
    printNumber(IOAPICVER);
    for (uint8_t i=0; i < 8; i++) 
    {
        term_write("IRQ ", 4);
        printNumber(i);
        uint32_t redirection_entry_1 = readIOAPIC(ioapicaddr, 0x10 + i * 2);
        uint32_t redirection_entry_2 = readIOAPIC(ioapicaddr, 0x11 + i * 2);
        printNumber(redirection_entry_1);
        printNumber(redirection_entry_2);
    }
}

void enableKeyboard(size_t ioapicaddr) 
{
    writeIOAPIC(ioapicaddr, 0x12, 0x21);
    registerInterruptHandler(0x21, &keyboardHandler);
}

void enableSerialCOM1(size_t ioapicaddr)
{
    writeIOAPIC(ioapicaddr, 0x18, 0x24);
    initSerial();
}
