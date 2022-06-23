#include <stdint.h>

typedef struct {
    uint64_t r11, r10, r9, r8;
    uint64_t rsi, rdi, rdx, rcx, rax;
    uint64_t int_no, err_code;
    uint64_t rsp, rflags, cs, rip;
}__attribute__((packed)) interrupt_frame_t;

typedef void (*isr_t) (interrupt_frame_t* frame);

void exception_handler(interrupt_frame_t* frame);
void irq_handler(interrupt_frame_t* frame); 
void register_interrupt_handler(uint8_t irq, isr_t handler);
void printNumber(uint64_t num, char* x);
