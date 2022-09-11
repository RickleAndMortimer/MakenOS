#include "fadt.h"
#include "../../sys/io.h"

// see https://wiki.osdev.org/FADT for more info

FADT* fadt;

void initFADT() 
{
    fadt = (FADT*) findHeader("FACP");
}

void enableACPI() 
{
    outb(fadt->SMI_command_port, fadt->ACPI_enable);
    // Poll PM1a control block until it is clear
    while ((inw(fadt->PM1a_control_block) & 1) == 0);
}
