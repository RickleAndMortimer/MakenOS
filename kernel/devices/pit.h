#ifndef PIT_H
#define PIT_H

#include <stdint.h>

void initPIT(uint32_t frequency);
uint32_t PIT_sleep(uint64_t milliseconds);

#endif
