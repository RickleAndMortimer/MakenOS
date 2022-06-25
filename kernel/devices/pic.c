#include <stdint.h>
#include <pic.h>

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define PIC_EOI        0x20

#define PIC1        0x20        /* IO base address for master PIC */
#define PIC2        0xA0        /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA    (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA    (PIC2+1)

#define ICW1_ICW4    0x01        /* ICW4 (not) needed */
#define ICW1_SINGLE    0x02        /* Single (cascade) mode */
#define ICW1_INTERVAL4    0x04        /* Call address interval 4 (8) */
#define ICW1_LEVEL    0x08        /* Level triggered (edge) mode */
#define ICW1_INIT    0x10        /* Initialization - required! */
 
#define ICW4_8086    0x01        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO    0x02        /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE    0x08        /* Buffered mode/slave */
#define ICW4_BUF_MASTER    0x0C        /* Buffered mode/master */
#define ICW4_SFNM    0x10        /* Special fully nested (not) */
#define PIC_READ_IRR                0x0a    /* OCW3 irq ready next CMD read */
#define PIC_READ_ISR                0x0b    /* OCW3 irq service next CMD read */

uint8_t my_inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

uint16_t my_inw(uint16_t port)
{
    uint16_t ret;
    asm volatile ( "inw %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void my_outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    /* There's an outb %al, $imm8  encoding, for compile-time constant port numbers that fit in 8b.  (N constraint).
     * Wider immediate constants would be truncated at assemble-time (e.g. "i" constraint).
     * The  outb  %al, %dx  encoding is the only option for all other cases.
     * %1 expands to %dx because  port  is a uint16_t.  %w1 could be used if we had the port number a wider C type */
}


void my_io_wait(void)
{
    my_outb(0x80, 0);
}

/* Helper func */
static uint16_t __pic_get_irq_reg(int ocw3)
{
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    my_outb(PIC1_COMMAND, ocw3);
    my_outb(PIC2_COMMAND, ocw3);
    return (my_inb(PIC2_COMMAND) << 8) | my_inb(PIC1_COMMAND);
}
 
/* Returns the combined value of the cascaded PICs irq request register */
uint16_t pic_get_irr(void)
{
    return __pic_get_irq_reg(PIC_READ_IRR);
}
 
/* Returns the combined value of the cascaded PICs in-service register */
uint16_t pic_get_isr(void)
{
    return __pic_get_irq_reg(PIC_READ_ISR);
}

void sendEOIPIC(uint8_t irq)
{
    if(irq >= 8)
        my_outb(PIC2_COMMAND,PIC_EOI);

    my_outb(PIC1_COMMAND,PIC_EOI);
}

void setMaskIRQ(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = my_inb(port) | (1 << IRQline);
    my_outb(port, value);        
}
 
void clearMaskIRQ(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }

    value = my_inb(port) & ~(1 << IRQline);
    my_outb(port, value);        
}

void disableAllIRQs() {
    for (uint8_t i = 0; i < 16; i++) {
    setMaskIRQ(i);
    }
}

void enableAllIRQs() {
    for (uint8_t i = 0; i < 16; i++) {
        clearMaskIRQ(i);
    }
}

void remapPIC(int offset1, int offset2)
{
    uint8_t a1, a2;
 
    a1 = my_inb(PIC1_DATA);                        // save masks
    a2 = my_inb(PIC2_DATA);
 
    my_outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
    my_io_wait();
    my_outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    my_io_wait();
    my_outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
    my_io_wait();
    my_outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
    my_io_wait();
    my_outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    my_io_wait();
    my_outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
    my_io_wait();
 
    my_outb(PIC1_DATA, ICW4_8086);
    my_io_wait();
    my_outb(PIC2_DATA, ICW4_8086);
    my_io_wait();
 
    my_outb(PIC1_DATA, a1);   // restore saved masks.
    my_outb(PIC2_DATA, a2);

}
