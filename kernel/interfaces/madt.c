#include <stdint.h>
#include <madt.h>

MADT* madt;

void initMADT() {
    madt = (MADT*) findHeader("APIC");
}
