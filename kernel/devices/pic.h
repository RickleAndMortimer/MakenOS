#include <stdint.h>

void sendEOIPIC(uint8_t irq);

void setMaskIRQ(uint8_t IRQline);
void clearMaskIRQ(uint8_t IRQline);
void disableAllIRQs();

void remapPIC(int offset1, int offset2);

void my_outb(uint16_t, uint8_t);
uint8_t my_inb(uint16_t port);
