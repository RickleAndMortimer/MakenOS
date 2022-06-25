#include <stdint.h>
#include <kernel.h>
#include <isr.h>
#include <print.h>
#include <pic.h>
#include <stdint.h>

void (*interrupt_handlers[256]) (InterruptFrame* frame);

void exception_handler(InterruptFrame* frame) {
    char x[20];
    printNumber(frame->int_no, x);
    printNumber(frame->err_code, x);
}

void register_interrupt_handler(uint8_t interrupt, void (*handler) (InterruptFrame* frame))
{
    interrupt_handlers[interrupt] = handler;
}

void irq_handler(InterruptFrame* frame)
{
    // Send an EOI (end of interrupt) signal to the PICs->
    // If this interrupt involved the slave->
    if (&interrupt_handlers[frame->int_no] != NULL)
    {
        interrupt_handlers[frame->int_no](frame);
    }
}
