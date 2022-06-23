#include <stdint.h>
#include <acpi.h>
#include <stddef.h>

RSDPDescriptor20 *rsdp_descriptor;
RSDT* rsdt;
XSDT* xsdt;

uint8_t validateRSDPChecksum() {
    uint64_t checksum = rsdp_descriptor->descriptor10.checksum;
    for (uint8_t i = 0; i < 8; i++) {
	checksum += rsdp_descriptor->descriptor10.signature[i];
    }

    for (uint8_t i = 0; i < 6; i++) {
	checksum += rsdp_descriptor->descriptor10.OEM_id[i];
    }

    uint32_t address_byte = rsdp_descriptor->descriptor10.rsdt_address;
    checksum += rsdp_descriptor->descriptor10.revision;

    // split address into bytes and add to the checksum
    for (int i = 0 ; i < 4; i++) {
    	checksum += address_byte & 0xFF;
    	address_byte >>= 8;
    }

    // add these bytes if ACPI is version 2
    if (rsdp_descriptor->descriptor10.revision == 2) {
	address_byte = rsdp_descriptor->length;
        for (int i = 0 ; i < 4; i++) {
    	    checksum += address_byte & 0xFF;
    	    address_byte >>= 8;
        }

	address_byte = rsdp_descriptor->xsdt_address;
        for (int i = 0 ; i < 8; i++) {
    	    checksum += address_byte & 0xFF;
    	    address_byte >>= 8;
        }

	checksum += rsdp_descriptor->extended_checksum;

	for (uint8_t i = 0; i < 3; i++) {
	    checksum += rsdp_descriptor->reserved[i];
    	}
    }
    return checksum;
}

uint8_t validateSDTChecksum(ACPISDTHeader* table_header) {
    uint8_t sum = 0;
 
    for (int i = 0; i < table_header->length; i++)
    {
        sum += ((uint8_t *) table_header)[i];
    }
 
    return sum == 0;
}

ACPISDTHeader* findHeader(char* signature) {
    int entries = (rsdt->h.length - sizeof(rsdt->h)) / 4;

    for (int i = 0; i < entries; i++)
    {
        ACPISDTHeader *header = (ACPISDTHeader *) rsdt->other_SDT[i];
        if (header->signature[0] == signature[0] && header->signature[1] == signature[1] && header->signature[2] == signature[2] && header->signature[3] == signature[3])
            return header;
    }

    // No header found
    return NULL;
}


