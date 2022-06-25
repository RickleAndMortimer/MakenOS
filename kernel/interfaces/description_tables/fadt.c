#include <stdint.h>
#include <fadt.h>
#include <pic.h>

// see https://wiki.osdev.org/FADT for more info

FADT* fadt;

void initFADT() 
{
    fadt = (FADT*) findHeader("FACP");
}

void enableACPI() 
{
    my_outb(fadt->SMI_command_port, fadt->ACPI_enable);
    // Poll PM1a control block until it is clear
    while ((my_inw(fadt->PM1a_control_block) & 1) == 0);
}
