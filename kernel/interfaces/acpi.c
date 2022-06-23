#include <stdint.h>
#include <acpi.h>

void loadRSDPTable(uint64_t rsdp_pointer) {
    RSDPDescriptor20 *descriptor = (RSDPDescriptor20 *)rsdp_pointer;

    // validate RSDP table by summing each byte
    uint64_t checksum = descriptor->descriptor10.checksum;
    for (uint8_t i = 0; i < 8; i++) {
	checksum += descriptor->descriptor10.signature[i];
    }

    for (uint8_t i = 0; i < 6; i++) {
	checksum += descriptor->descriptor10.OEM_id[i];
    }

    uint32_t address_byte = descriptor->descriptor10.rsdt_address;
    checksum += descriptor->descriptor10.revision;

    // split address into bytes and add to the checksum
    for (int i = 0 ; i < 3; i++) {
    	checksum += address_byte & 0xFF;
    	address_byte >>= 8;
    }

    // add these bytes if ACPI is version 2
    if (descriptor->descriptor10.revision == 2) {
	address_byte = descriptor->length;
        for (int i = 0 ; i < 3; i++) {
    	    checksum += address_byte & 0xFF;
    	    address_byte >>= 8;
        }

	address_byte = descriptor->xsdt_address;
        for (int i = 0 ; i < 7; i++) {
    	    checksum += address_byte & 0xFF;
    	    address_byte >>= 8;
        }

	checksum += descriptor->extended_checksum;

	for (uint8_t i = 0; i < 3; i++) {
	    checksum += descriptor->reserved[i];
    	}
    }
    if (checksum & 1) {
	// ACPI cannot be used
    }
    else {
	// TODO: Parse RSDT and XSDT
    }
}
