#include <pit.h>
#include <pic.h>
#include <isr.h>
#include <kernel.h>

uint32_t tick = 0;

static isr_t timer_callback(interrupt_frame_t* frame)
{
    term_write(".", 1);
}

void initTimer(uint32_t frequency)
{
    // Firstly, register our timer callback.
    term_write("Initializing timer\n", 19);
    register_interrupt_handler(32, timer_callback);

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte.
    my_outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    // Send the frequency divisor.
    my_outb(0x40, l);
    my_outb(0x40, h);
    term_write("Timer initialized\n", 18);
    clearMaskIRQ(0);
}

void sleep() {


}