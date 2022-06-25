#include <stdint.h>
#include <acpi.h>
#include <stddef.h>

typedef struct MADT 
{
    ACPISDTHeader header;
    // 0: Local APIC Address
    // 1: Flags ()
    uint32_t APIC_address;
    uint32_t APIC_flags;
    // nth entry: Entry type
    // n+1th entry: Record length
} __attribute__((packed)) MADT;

typedef struct APICRecordHeader
{
    uint8_t entry_type;
    uint8_t record_length;
} APICRecordHeader;

// Entry Type 0
typedef struct ProcessorAPIC
{
    APICRecordHeader head;
    uint8_t ACPI_processor_id; 
    uint8_t id; 
    uint32_t flags;
} ProcessorAPIC;

// Entry Type 1
typedef struct IOAPIC
{
    APICRecordHeader head;
    uint8_t id; 
    uint8_t reserved; 
    uint32_t address;
    uint32_t global_system_interrupt_base;
} __attribute__((packed)) IOAPIC;

// Entry Type 2
typedef struct IOAPICSourceOverride
{
    APICRecordHeader head;
    uint8_t bus_source; 
    uint8_t IRQ_source; 
    uint32_t global_system_interrupt;
    uint16_t flags;
} __attribute__((packed)) IOAPICSourceOverride;

// Entry Type 3
typedef struct {
    APICRecordHeader head;
    uint8_t NMI_source; 
    uint8_t reserved; 
    uint16_t flags;
    uint32_t global_system_interrupt;
} __attribute__((packed)) IOAPICNonMaskableInterruptSource;

// Entry Type 4
typedef struct {
    APICRecordHeader head;
    uint8_t ACPI_processor_id; 
    uint16_t flags;
    uint8_t LINT;
} __attribute__((packed)) IOAPICNonMaskableInterrupt;

// Entry Type 5
typedef struct {
    APICRecordHeader head;
    uint16_t reserved;
    uint64_t LAPIC_address;
} __attribute__((packed)) LAPICAddressOverride;

// Entry Type 9
typedef struct x2LAPIC
{
    APICRecordHeader head;
    uint16_t reserved; 
    uint32_t id;
    uint32_t flags;
    uint32_t ACPI_id;
} __attribute__((packed)) x2LAPIC;

void initMADT();
void parseMADT();
