#include <stdint.h>
#include <stddef.h>

uint32_t readIOAPIC(size_t ioapicaddr, uint32_t reg);
void writeIOAPIC(size_t ioapicaddr, uint32_t reg, uint32_t value);
void enableKeyboard(size_t ioapicaddr);
