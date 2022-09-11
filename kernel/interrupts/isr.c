#include "../kernel.h"
#include "isr.h"
#include "../memory/paging.h"
#include "../lib/print.h"
#include "../devices/pic.h"

void (*interrupt_handlers[256]) (InterruptFrame* frame);

static inline uint64_t getCR2(void)
{
	uint64_t val;
	__asm__ volatile ( "mov %%cr2, %0" : "=r"(val) );
    return val;
}

void exceptionHandler(InterruptFrame* frame) {
    printNumber(frame->int_no);
    printNumber(frame->err_code);
	switch (frame->int_no) {
		case 14:
			if (frame->err_code & 1) {
				term_write("Page protection violation\n", 26);
				for (;;) 
				{
					__asm__ volatile ("hlt");
				}
			}
			uint64_t cr2 = getCR2();
			printNumber(cr2);
			term_write("dead :(", 7);
			break;
	}
}

void registerInterruptHandler(uint8_t interrupt, void (*handler) (InterruptFrame* frame))
{
    interrupt_handlers[interrupt] = handler;
}

void irqHandler(InterruptFrame* frame)
{
    if (&interrupt_handlers[frame->int_no] != NULL)
    {
        interrupt_handlers[frame->int_no](frame);
    }
}
