#include "../sys/io.h"
#include "serial.h"
#include "../interrupts/isr.h"
#include "../lib/print.h"
#include "../kernel.h"

static void serialInterruptHandler(InterruptFrame* frame) 
{
    printNumber(frame->int_no);
    char x[2];
    x[0] = readSerial();
    term_write(x, 2);
}

bool initSerial() 
{
    registerInterruptHandler(0x24, &serialInterruptHandler);
	outb(COM1 + 1, 0x00);	 // Disable all interrupts
	outb(COM1 + 3, 0x80);	 // Enable DLAB (set baud rate divisor)
	outb(COM1 + 0, 0x03);	 // Set divisor to 3 (lo byte) 38400 baud
	outb(COM1 + 1, 0x00);
	outb(COM1 + 3, 0x03);	 // 8 bits, no parity, one stop bit
	outb(COM1 + 2, 0xC7);	 // Enable FIFO, clear them, with 14-byte threshold
	outb(COM1 + 4, 0x0B);	 // IRQs enabled, RTS/DSR set
	outb(COM1 + 4, 0x1E);	 // Set in loopback mode, test the serial chip
	outb(COM1 + 0, 0xAE);	 // Test serial chip (send byte 0xAE and check if serial returns same byte)
 
	// Check if serial is faulty (i.e: not same byte as sent)
	if(inb(COM1 + 0) != 0xAE) 
	{
        term_write("bokre", 5);
		return true;
	}
 
	// If serial is not faulty set it in normal operation mode
	// (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
	outb(COM1 + 4, 0x0F);
	//outb(COM1 + 1, 0x0F);
	return false;
}

bool isSerialReceived() 
{
	return inb(COM1 + 5) & 1;
}
 
char readSerial() 
{
	while (!isSerialReceived());
	return inb(COM1);
}

char* readSerialString(char* buffer, size_t len) 
{
	 size_t i;
	 for (i = 0; i < len; i++) 
	 {
		  char new_char = readSerial();
		  if (new_char != '\0') 
		  {
              buffer[i++] = new_char;
		  }
	 }
	 buffer[i] = '\0';
	 return buffer;
}

bool isTransmitEmpty() 
{
	return inb(COM1 + 5) & 0x20;
}
 
void writeSerial(char a) 
{
	while (!isTransmitEmpty());
 
	outb(COM1, a);
}

void writeSerialString(const char* str) 
{
	 size_t i = 0;
	 while (str[i] != '\0') 
	 {
		  writeSerial(str[i++]);
	 }
}
