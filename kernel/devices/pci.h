#ifndef PCI_H
#define PCI_H

#include <stdint.h>

uint16_t pciConfigReadWord(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

#endif
