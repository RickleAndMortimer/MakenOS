#include <stdint.h>

void sendEOIPIC(unsigned char irq);

void setMaskIRQ(unsigned char IRQline);
void clearMaskIRQ(unsigned char IRQline);

void remapPIC(int offset1, int offset2);

void outb(uint16_t, uint8_t);
uint8_t inb(uint16_t port);
