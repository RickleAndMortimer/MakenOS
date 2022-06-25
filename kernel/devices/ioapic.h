#include <stdint.h>

uint32_t readIOAPIC(void *ioapicaddr, uint32_t reg);
uint32_t writeIOAPIC(void *ioapicaddr);
uint32_t enableKeyboard(void *ioapicaddr);


