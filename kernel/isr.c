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

void exception_handler(interrupt_frame_t* stack) {
    char* x;
    term_write("\n",1);
    printNumber(stack->int_no, x);
    printNumber(stack->err_code, x);
    printNumber(stack->r15, x);
    printNumber(stack->r14, x);
    printNumber(stack->r13, x);
    printNumber(stack->r12, x);
    printNumber(stack->r11, x);
    printNumber(stack->r10, x);
    printNumber(stack->r9, x);
    printNumber(stack->r8, x);
    printNumber(stack->rsp, x);
    printNumber(stack->rsi, x);
    printNumber(stack->rdi, x);
    printNumber(stack->rbp, x);
    printNumber(stack->rax, x);
    printNumber(stack->rbx, x);
    printNumber(stack->rcx, x);
    printNumber(stack->rdx, x);
    printNumber(stack->rflags, x);
    printNumber(stack->cs, x);
    printNumber(stack->rip, x);
    asm volatile ("cli; hlt");
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
