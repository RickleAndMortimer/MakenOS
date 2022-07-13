#include <stdint.h>
#include <kernel.h>
#include <isr.h>
#include <print.h>
#include <pic.h>
#include <stdint.h>

void (*interrupt_handlers[256]) (InterruptFrame* frame);

static inline uint64_t getCR2(void)
{
	uint64_t val;
	asm volatile ( "mov %%cr2, %0" : "=r"(val) );
    return val;
}

void exception_handler(InterruptFrame* frame) {
    char x[20];
    printNumber(frame->int_no, x);
    printNumber(frame->err_code, x);
	switch (frame->int_no) {
		case 14:
			if (frame->err_code & 1) {
				term_write("Page protection violation\n", 26);
				for (;;) 
				{
					asm volatile ("hlt");
				}
			}
			uint64_t cr2 = getCR2();
			printNumber(cr2, x);
			term_write("dead :(", 7);
			for (;;) 
			{
				asm volatile ("hlt");
			}
			
			break;
	}
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
