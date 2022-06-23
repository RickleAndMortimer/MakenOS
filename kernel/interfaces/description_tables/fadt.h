#include <stdint.h>
#include <acpi.h>

// see https://wiki.osdev.org/FADT for more info

typedef struct GenericAddressStructure
{
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
} GenericAddressStructure;

typedef struct FADT
{
    ACPISDTHeader h;
    uint32_t firmware_control;
    uint32_t DSDT_address;
 
    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  reserved;
 
    uint8_t  preferred_power_management_profile;
    uint16_t SCI_interrupt;
    uint32_t SMI_command_port;
    uint8_t  ACPI_enable;
    uint8_t  ACPI_disable;
    uint8_t  S4BIOS_req;
    uint8_t  PSTATE_control;
    uint32_t PM1a_event_block;
    uint32_t PM1b_event_block;
    uint32_t PM1a_control_block;
    uint32_t PM1b_control_block;
    uint32_t PM2_control_block;
    uint32_t PM_timer_block;
    uint32_t GPE0_block;
    uint32_t GPE1_block;
    uint8_t  PM1_event_length;
    uint8_t  PM1_control_length;
    uint8_t  PM2_control_length;
    uint8_t  PM_timer_length;
    uint8_t  GPE0_length;
    uint8_t  GPE1_length;
    uint8_t  GPE1_base;
    uint8_t  C_state_control;
    uint16_t worst_C2_latency;
    uint16_t worst_C3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t  duty_offset;
    uint8_t  duty_width;
    uint8_t  day_alarm;
    uint8_t  month_alarm;
    uint8_t  century;
 
    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t boot_architecture_flags;
 
    uint8_t  reserved_2;
    uint32_t flags;
 
    GenericAddressStructure reset_reg;
 
    uint8_t  reset_value;
    uint8_t  reserved_3[3];
 
    // 64bit pointers - Available on ACPI 2.0+
    uint64_t x_firmware_control;
    uint64_t x_DSDT;
 
    GenericAddressStructure X_PM1a_event_block;
    GenericAddressStructure X_PM1b_event_block;
    GenericAddressStructure X_PM1a_control_block;
    GenericAddressStructure X_PM1b_control_block;
    GenericAddressStructure X_PM2_control_block;
    GenericAddressStructure X_PM_timer_block;
    GenericAddressStructure X_GPE0_block;
    GenericAddressStructure X_GPE1_block;
}
