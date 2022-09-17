#ifndef PCI_H
#define PCI_H

#include <stdint.h>
#include <stddef.h>

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void enablePCIInterrupts(uint8_t bus, uint8_t device, uint8_t function, size_t ioapicaddr);
void checkMSI(uint8_t bus, uint8_t device, uint8_t func);

#endif
