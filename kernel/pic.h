#include <stdint.h>

void sendEOIPIC(unsigned char irq);

void setMaskIRQ(unsigned char IRQline);
void clearMaskIRQ(unsigned char IRQline);

void remapPIC(int offset1, int offset2);

void my_outb(uint16_t, uint8_t);
uint8_t my_inb(uint16_t port);
