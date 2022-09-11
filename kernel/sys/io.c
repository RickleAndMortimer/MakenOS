#include "io.h"

uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

uint32_t in(uint16_t port)
{
    uint32_t ret;
    __asm__ volatile ( "in %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}

void out(uint16_t port, uint32_t val)
{
    __asm__ volatile ( "out %0, %1" : : "a"(val), "Nd"(port) );
}

void io_wait(void)
{
    outb(0x80, 0);
}
