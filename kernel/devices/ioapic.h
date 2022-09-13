#ifndef IOAPIC_H
#define IOAPIC_H

#include <stdint.h>
#include <stddef.h>

uint32_t readIOAPIC(size_t ioapicaddr, uint32_t reg);
void writeIOAPIC(size_t ioapicaddr, uint32_t reg, uint32_t value);
void enableKeyboard(size_t ioapicaddr);
void enableSerialCOM1(size_t ioapicaddr);
void readIOREDTBLs(size_t ioapicaddr);

#endif
