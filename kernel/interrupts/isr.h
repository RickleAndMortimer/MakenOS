#ifndef ISR_H
#define ISR_H

#include <stdint.h>

typedef struct InterruptFrame
{
    uint64_t r11, r10, r9, r8;
    uint64_t rsi, rdi, rdx, rcx, rax;
    uint64_t int_no, err_code;
    uint64_t rsp, rflags, cs, rip;
}__attribute__((packed)) InterruptFrame;

void exceptionHandler(InterruptFrame* frame);
void irqHandler(InterruptFrame* frame); 
void registerInterruptHandler(uint8_t irq, void (*handler) (InterruptFrame*));

#endif
