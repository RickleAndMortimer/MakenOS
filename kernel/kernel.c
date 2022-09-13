#include "devices/apic.h"
#include "interrupts/idt.h"
#include "devices/ioapic.h"
#include "interfaces/description_tables/madt.h"
#include "process/task.h"
#include "memory/paging.h"
#include "devices/pic.h"
#include "devices/pci.h"
#include "devices/pit.h"
#include "memory/pmm.h"
#include "lib/print.h"
#include "devices/ps2.h"
#include "stivale2.h"
#include "devices/serial.h"
#include "filesystem/file.h"

extern RSDPDescriptor20 *rsdp_descriptor;
extern XSDT* xsdt;
extern RSDT* rsdt;
extern MADT* madt;

extern ProcessorAPIC* processor_apics[];
extern IOAPIC* ioapics[];
extern IOAPICSourceOverride* ioapic_source_overrides[];
extern IOAPICNonMaskableInterruptSource* ioapic_interrupt_sources[];
extern IOAPICNonMaskableInterrupt* ioapic_interrupts[];
extern LAPICAddressOverride* lapic_address_overrides[];
extern x2LAPIC* x2_lapics[];


void (*term_write)(const char *string, size_t length);
struct stivale2_struct_tag_memmap* memmap_tag;

static uint8_t stack[8192];

static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        .next = 0
    },
    .flags = 0
};

static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uint64_t)&terminal_hdr_tag
    },
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};
static struct stivale2_header_tag_smp smp_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_SMP_ID,
        .next = (uint64_t)&framebuffer_hdr_tag
    },
    .flags = 0
};


// The stivale2 specification says we need to define a "header structure".
// This structure needs to reside in the .stivale2hdr ELF section in order
// for the bootloader to find it. We use this __attribute__ directive to
// tell the compiler to put the following structure in said section.
__attribute__((section(".stivale2hdr"), used))
static struct stivale2_header stivale_hdr = {
    // The entry_point member is used to specify an alternative entry
    // point that the bootloader should jump to instead of the executable's
    // ELF entry point. We do not care about that so we leave it zeroed.
    .entry_point = 0,
    // Let's tell the bootloader where our stack is.
    // We need to add the sizeof(stack) since in x86(_64) the stack grows
    // downwards.
    .stack = (uintptr_t)stack + sizeof(stack),
    // Bit 1, if set, causes the bootloader to return to us pointers in the
    // higher half, which we likely want since this is a higher half kernel.
    // Bit 2, if set, tells the bootloader to enable protected memory ranges,
    // that is, to respect the ELF PHDR mandated permissions for the executable's
    // segments.
    // Bit 3, if set, enables fully virtual kernel mappings, which we want as
    // they allow the bootloader to pick whichever *physical* memory address is
    // available to load the kernel, rather than relying on us telling it where
    // to load it.
    // Bit 4 disables a deprecated feature and should always be set.
    .flags = (1 << 1) | (1 << 2) | (1 << 3) | (1 << 4),
    .tags = (uintptr_t)&smp_hdr_tag
};

// Scan for tags in linked list
void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id) 
{
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;) {
        if (current_tag == NULL) {
            return NULL;
        }

        if (current_tag->identifier == id) {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (void *)current_tag->next;
    }
}

// Kernel entrypoint
void _start(struct stivale2_struct *stivale2_struct) {
    struct stivale2_struct_tag_terminal* term_str_tag;
    term_str_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_TERMINAL_ID);

    // Access RSDP for ACPI
    struct stivale2_struct_tag_rsdp* rsdp_tag;
    rsdp_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_RSDP_ID);

    // Get LAPIC info
    struct stivale2_struct_tag_smp* smp_tag;
    smp_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_SMP_ID);

    // Check if the tags were actually found.
    if (term_str_tag == NULL) 
    {
        for (;;) 
            __asm__ ("hlt");
    }

    if (rsdp_tag == NULL) 
    {
        for (;;) 
            __asm__ ("hlt");
    }

    void *term_write_ptr = (void *)term_str_tag->term_write;
    
    term_write = term_write_ptr;

    term_write("Hello World\n", 13);
    printNumber(rsdp_tag->rsdp);

    rsdp_descriptor = (RSDPDescriptor20*) rsdp_tag->rsdp;
    if (rsdp_descriptor->descriptor10.revision == 2) 
    {
        xsdt = (XSDT*)rsdp_descriptor->xsdt_address;
    }
    rsdt = (RSDT*)(uintptr_t)rsdp_descriptor->descriptor10.rsdt_address;

    if ((validateRSDPChecksum() & 0xFF) == 0) 
    {
        term_write("ACPI ready to go\n", 18);
        printNumber(rsdp_descriptor->descriptor10.revision);
    }
    term_write(rsdt->h.signature, 4);

    // Find FADT and enable ACPI mode there
    ACPISDTHeader* fadt = findHeader("FACP");
    if (fadt) 
    {
    	term_write(fadt->signature, 4);
    }

    // Initialize MADT
    initMADT();
    term_write("\nfinding APICS\n", 15);
    parseMADT();
    term_write("found APICS\n", 13);

    term_write("testing results\n", 16);

    term_write("my results\n", 12);
    printNumber(madt->header.length);
    printNumber(madt->APIC_address);
    printNumber(ioapics[0]->global_system_interrupt_base);
    for (uint8_t i = 0; i < 5; i++) 
    {
        term_write("IOAPIC\n", 7);
        printNumber(ioapic_source_overrides[i]->bus_source);
        printNumber(ioapic_source_overrides[i]->IRQ_source);
        printNumber(ioapic_source_overrides[i]->global_system_interrupt);
    }

    if (smp_tag) 
    {
        term_write("limine's results\n", 18);
        printNumber(smp_tag->cpu_count);
        printNumber(smp_tag->flags);
        printNumber(smp_tag->unused);
        printNumber(smp_tag->smp_info[0].lapic_id);
        printNumber(smp_tag->smp_info[0].processor_id);
    }

    term_write("results done\n", 14);

    memmap_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    PageTable* pml4 = initPML4();
    printMemoryMaps();
    setMemoryMap(4);

    // Initialize devices
    remapPIC(0x20, 0x28);
    initIdt();

    enableAPIC();
    enableAPICTimer(10000);
	enableKeyboard(ioapics[0]->address);

	initTasking();

    /*
    uint64_t* p = getPhysicalAddress((void*) 0x9000);
    uint64_t* f = getPhysicalAddress((void*) 0xA000);

    mapPage(0x9000, (void*) 0x1000, 3);
    mapPage(0xA000, (void*) 0x1000, 3);

    term_write("goodbye\n", 8);
    p = getPhysicalAddress((void*) 0x9001);
    f = getPhysicalAddress((void*) 0xA000);

    uint64_t* y = (uint64_t*) 0x9000;
    *y = 100;
    printNumber(*y);
    uint64_t* z = (uint64_t*) 0xA000;
    printNumber(*z);


    printNumber(p);
    printNumber(f);

    */
    inode_table* f = initRamFS();
    inode* i = fopen("neighbor");
    fwrite(i, "hello", 6);
    char jk[10];
    fread(i, jk, 1, 6);
    term_write(jk, 10);
    
    uint16_t rose = pciConfigReadWord(0, 31, 3, 0x20);
    printNumber(rose);

    rose = pciConfigReadWord(0, 31, 3, 0x22);
    printNumber(rose);

	enableSerialCOM1(ioapics[0]->address);

    for (;;) 
    {
		__asm__ volatile ("hlt");
    }
}
