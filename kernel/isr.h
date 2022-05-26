#include <stdint.h>

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rsp, rflags, cs, rip;
}__attribute__((packed)) interrupt_frame_t;

typedef void (*isr_t) (interrupt_frame_t stack);

__attribute__((noreturn))
void exception_handler(interrupt_frame_t* stack); 
__attribute__((noreturn))
void irq_handler(interrupt_frame_t* stack); 

void register_interrupt_handler(uint8_t irq, isr_t handler);
