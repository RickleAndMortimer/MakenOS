#include <kernel.h>
#include <isr.h>
#include <cpuid.h>
#include <stdint.h>

isr_t interrupt_handlers[256];

size_t strlen(char* s) {
    size_t len = 0;
    while(s[len] != '\0') {len++;};
    return len;
}

void reverse(char* s) {
    int i, j;
    char tmp;
    for (i=0,j=strlen(s)-1; i < j; i++, j--){
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
    
}

void itoa(int value, char* str, int base) {
    int i = 0;
    int sign;
    if ((sign = value) < 0) {
        value = -value;
    }
    do {
        str[i++] = value % base + '0';
        
    } while ((value /= base) > 0);
    if (sign < 0)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str);
}

void uint64toa(uint64_t value, char* str, int base) {
    int i = 0;
    do {
        str[i++] = value % base + '0';
    } while ((value /= base) > 0);
    str[i] = '\0';
    reverse(str);
}

void printNumber(uint64_t num, char* x) {
    uint64toa(num, x, 10);
    term_write(x, strlen(x));
    term_write("\n", 1);
}

void exception_handler(uint64_t int_no, uint64_t err_code) {
    char x[20];
    term_write("\n",1);
    printNumber(int_no, x);
    printNumber(err_code, x);
}

void register_interrupt_handler(uint8_t irq, isr_t handler)
{
	interrupt_handlers[irq] = handler;
}

void irq_handler(interrupt_frame_t* frame)
{
   // Send an EOI (end of interrupt) signal to the PICs->
   // If this interrupt involved the slave->
   term_write("handling interrupt", 20);
   if (frame->int_no >= 40)
   {
       // Send reset signal to slave->
       outb(0xA0, 0x20);
   }
   // Send reset signal to master-> (As well as slave, if necessary)->
   outb(0x20, 0x20);

   if (interrupt_handlers[frame->int_no] != 0)
   {
       isr_t handler = interrupt_handlers[frame->int_no];
   }
}
