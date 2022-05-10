#include <stdint.h>

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
}__attribute__((packed)) interrupt_frame_t;

typedef void (*isr_t) (uint64_t int_no);

__attribute__((noreturn))
void exception_handler(uint64_t int_no, uint64_t err_code, interrupt_frame_t *stack); 

void register_interrupt_handler(uint8_t irq, isr_t handler);
