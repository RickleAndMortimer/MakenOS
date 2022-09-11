#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../interrupts/isr.h"
#include "pic.h"
#include "pci.h"
#include "ioapic.h"

uint16_t pciConfigReadWord(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) 
{
    uint32_t lbus  = (uint32_t)bus;
    uint32_t ldevice = (uint32_t)device;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;
 
    // Create configuration address
    uint32_t address = (uint32_t)((lbus << 16) | (ldevice << 11) |
                       (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
    my_out(0xCF8, address);
    // Read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32-bit register
    tmp = (uint16_t)((my_in(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}

uint16_t getVendorID(uint16_t bus, uint16_t device, uint16_t function) 
{
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    return pciConfigReadWord(bus, device, function, 0);
}

uint16_t getHeaderType(uint16_t bus, uint16_t device, uint16_t function) 
{
    /* Try and read the first configuration register. Since there are no
     * vendors that == 0xFFFF, it must be a non-existent device. */
    return pciConfigReadWord(bus, device, function, (0xC + 2) & 0xFF);
}

bool checkFunction(uint8_t bus, uint8_t device, uint8_t function) 
{
    return true;
}

void checkDevice(uint8_t bus, uint8_t device) 
{
    uint8_t function = 0;

    if (!checkFunction(bus, device, 0)) return;

    uint16_t headerType = getHeaderType(bus, device, function);
    if ((headerType & 0x80) != 0) {
        // It's a multi-function device, so check remaining functions
        for (function = 1; function < 8; function++) {
            if (getVendorID(bus, device, function) != 0xFFFF){

            }
        }
    }
}

void checkAllBuses(void) 
{
    for (size_t bus = 0; bus < 256; bus++) {
        for (size_t device = 0; device < 32; device++) {
            checkDevice(bus, device);
        }
    }
}

void enablePCIInterrupts(uint8_t bus, uint8_t device, uint8_t function, size_t ioapicaddr) {
    uint16_t interrupt = pciConfigReadWord(bus, device, function, 0x4C);
    uint16_t interrupt_line = interrupt & 0xFF;
    uint16_t interrupt_pin = interrupt >> 8;
    writeIOAPIC(ioapicaddr, interrupt_line * 2 + 0x10, 0x20 + interrupt_line);
    register_interrupt_handler(0x20 + interrupt_line, &keyboardHandler);
}
