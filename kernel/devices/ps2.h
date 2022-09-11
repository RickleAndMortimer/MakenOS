#ifndef PS2_H

#include <stdint.h>

#define PS2_H

#define PS2_DATA_PORT 0x60 // Read/Write
#define PS2_STATUS_REG 0x64 // Read
#define PS2_COMMAND_REG 0x64 // Write

void initKeyboard();
uint8_t readDataPort();

// TODO: map scan codes to ascii

// Scancodes: Set 1

// Scancodes: Set 2

// Scancodes: Set 3

#endif
