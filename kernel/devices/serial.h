#ifndef SERIAL_H
#define SERIAL_H

#include <stdbool.h>
#include <stddef.h>

#define COM1 0x3F8

// Reference: https://wiki.osdev.org/Serial_Ports 

bool initSerial();

bool isSerialReceived();
char readSerial();
char* readSerialString(char* buffer, size_t len);

bool isTransmitEmpty();
void writeSerial(char a);
void writeSerialString(const char* str);

#endif
