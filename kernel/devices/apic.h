#ifndef APIC_H

#define APIC_H
#include <stdint.h>

// https://wiki.osdev.org/APIC#IO_APIC_Registers

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi);
void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi);
uintptr_t cpuGetAPICBase(); 
void cpuSetAPICBase(uintptr_t apic);
uint32_t readAPICRegister(uint32_t reg);
void writeAPICRegister(uint32_t reg, uint32_t value);
void enableAPIC();
void enableAPICTimer(uint32_t frequency);

#endif
