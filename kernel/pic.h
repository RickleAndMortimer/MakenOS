#include <stdint.h>

void sendEOIPIC(unsigned char irq);

void setMaskIRQ(unsigned char IRQline);
void clearMaskIRQ(unsigned char IRQline);

void remapPIC(int offset1, int offset2);
