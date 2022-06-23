#include <stdint.h>

typedef struct  {
    char signature[8];
    uint8_t checksum;
    char OEM_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__ ((packed)) RSDPDescriptor;

typedef struct {
    RSDPDescriptor descriptor10;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__ ((packed)) RSDPDescriptor20;

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char OEM_id[6];
    char OEM_table_id[8];
    uint32_t OEM_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__ ((packed)) ACPISDTHeader;

// uint64_t other_SDT[(h.Length - sizeof(h)) / 8];
typedef struct {
    ACPISDTHeader h;
    uint32_t other_SDT[];
} __attribute__ ((packed)) RSDT;

typedef struct {
    ACPISDTHeader h;
    uint64_t other_SDT[];
} __attribute__ ((packed)) XSDT;

void loadRSDPTable(uint64_t rsdp_address);
