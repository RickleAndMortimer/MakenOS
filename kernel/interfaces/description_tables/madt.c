#include <stdint.h>
#include <madt.h>
#include <isr.h>

MADT* madt;

ProcessorAPIC* processor_apics[32];
IOAPIC* ioapics[32];
IOAPICSourceOverride* ioapic_source_overrides[32];
IOAPICNonMaskableInterruptSource* ioapic_interrupt_sources[32];
IOAPICNonMaskableInterrupt* ioapic_interrupts[32];
LAPICAddressOverride* lapic_address_overrides[32];
x2LAPIC* x2_lapics[32];

void initMADT() 
{
    madt = (MADT*) findHeader("APIC");
}

void parseMADT() 
{
    APICRecordHeader* head;
    uint32_t length = sizeof(MADT);
    uintptr_t madt_address = (uintptr_t) madt;

    char x[20];
    // array pointers
    int i, j, k, l, m, n, o;
    i = j = k = l = m = n = o = 0;
    while (length < madt->header.length) 
    {
	head = (APICRecordHeader*)(madt_address + length);
	
	switch (head->entry_type) 
	{
	    case 0:
		processor_apics[i++] = (ProcessorAPIC*) head;
		length += head->record_length;
		break;
	    case 1:
		ioapics[j++] = (IOAPIC*) head;
		length += head->record_length;
		break;
	    case 2:
		ioapic_source_overrides[k++] = (IOAPICSourceOverride*) head;
		length += head->record_length;
		break;
	    case 3:
		ioapic_interrupt_sources[l++] = (IOAPICNonMaskableInterruptSource*) head;
		length += head->record_length;
		break;
	    case 4:
		ioapic_interrupts[m++] = (IOAPICNonMaskableInterrupt*) head;
		length += head->record_length;
		break;
	    case 5:
		lapic_address_overrides[n++] = (LAPICAddressOverride*) head;
		length += head->record_length;
		break;
	    case 9:
		x2_lapics[o++] = (x2LAPIC*) head;
		length += head->record_length;
		break;
	}
    }
}
