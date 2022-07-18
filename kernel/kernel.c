#include <apic.h>
#include <idt.h>
#include <ioapic.h>
#include <madt.h>
#include <paging.h>
#include <pic.h>
#include <pit.h>
#include <pmm.h>
#include <print.h>
#include <ps2.h>
#include <stddef.h>
#include <stdint.h>
#include <stivale2.h>

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

// Write to console function shared amongst the codebase
void (*term_write)(const char *string, size_t length);
struct stivale2_struct_tag_memmap* memmap_tag;

// We need to tell the stivale bootloader where we want our stack to be.
// We are going to allocate our stack as an array in .bss.
static uint8_t stack[8192];

// stivale2 uses a linked list of tags for both communicating TO the
// bootloader, or receiving info FROM it. More information about these tags
// is found in the stivale2 specification.

// stivale2 offers a runtime terminal service which can be ditched at any
// time, but it provides an easy way to print out to graphical terminal,
// especially during early boot.
static struct stivale2_header_tag_terminal terminal_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        // Identification constant defined in stivale2.h and the specification.
        .identifier = STIVALE2_HEADER_TAG_TERMINAL_ID,
        // If next is 0, it marks the end of the linked list of header tags.
        .next = 0
    },
    // The terminal header tag possesses a flags field, leave it as 0 for now
    // as it is unused.
    .flags = 0
};

// We are now going to define a framebuffer header tag.
// This tag tells the bootloader that we want a graphical framebuffer instead
// of a CGA-compatible text mode. Omitting this tag will make the bootloader
// default to text mode, if available.
static struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&terminal_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
    .framebuffer_width  = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp    = 0
};
static struct stivale2_header_tag_smp smp_hdr_tag = {
    // Same as above.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_SMP_ID,
        // Instead of 0, we now point to the previous header tag. The order in
        // which header tags are linked does not matter.
        .next = (uint64_t)&framebuffer_hdr_tag
    },
    // We set all the framebuffer specifics to 0 as we want the bootloader
    // to pick the best it can.
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
    // This header structure is the root of the linked list of header tags and
    // points to the first one in the linked list.
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
    // Let's get the terminal structure tag from the bootloader.
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
        for (;;) {
            asm ("hlt");
        }
    }

    if (rsdp_tag == NULL) 
    {
        for (;;) {
            asm ("hlt");
        }
    }

    // Let's get the address of the terminal write function.
    void *term_write_ptr = (void *)term_str_tag->term_write;
    
    // Now, let's assign this pointer to a function pointer which
    // matches the prototype described in the stivale2 specification for
    // the stivale2_term_write function.
    term_write = term_write_ptr;

    // We should now be able to call the above function pointer to print out
    // a simple "Hello World" to screen.
    term_write("Hello World\n", 13);
    char x[20];
    // start accessing XSDT/RSDT entries
    printNumber(rsdp_tag->rsdp, x);

    rsdp_descriptor = (RSDPDescriptor20*) rsdp_tag->rsdp;
    if (rsdp_descriptor->descriptor10.revision == 2) 
    {
	xsdt = (XSDT*)rsdp_descriptor->xsdt_address;
    }
    rsdt = (RSDT*)(uintptr_t)rsdp_descriptor->descriptor10.rsdt_address;

    if ((validateRSDPChecksum() & 0xFF) == 0) 
    {
	term_write("ACPI ready to go\n", 18);
	printNumber(rsdp_descriptor->descriptor10.revision, x);
    }
    term_write(rsdt->h.signature, 4);
    // Find FADT and enable ACPI mode there
    ACPISDTHeader* fadt = findHeader("FACP");
    if (fadt) {
    	term_write(fadt->signature, 4);
    }
    // Initialize MADT
    initMADT();
    term_write("\nfinding APICS\n", 15);
    parseMADT();
    term_write("found APICS\n", 13);

    term_write("testing results\n", 16);

    term_write("my results\n", 12);
    printNumber(madt->header.length, x);
    printNumber(madt->APIC_address, x);
    printNumber(ioapics[0]->global_system_interrupt_base, x);
    for (uint8_t i = 0; i < 5; i++) 
    {
	term_write("IOAPIC\n", 7);
        printNumber(ioapic_source_overrides[i]->bus_source, x);
        printNumber(ioapic_source_overrides[i]->IRQ_source, x);
        printNumber(ioapic_source_overrides[i]->global_system_interrupt, x);
    }

    if (smp_tag) {
        term_write("limine's results\n", 18);
        printNumber(smp_tag->cpu_count, x);
        printNumber(smp_tag->flags, x);
        printNumber(smp_tag->unused, x);
        printNumber(smp_tag->smp_info[0].lapic_id, x);
        printNumber(smp_tag->smp_info[0].processor_id, x);
    }

    term_write("results done\n", 14);

    memmap_tag = stivale2_get_tag(stivale2_struct, STIVALE2_STRUCT_TAG_MEMMAP_ID);

    PageTable* pml4 = initPML4();
    printMemoryMaps();
    setMemoryMap(4);

    uint64_t* p = getPhysicalAddress((void*) 0x9000);
    uint64_t* f = getPhysicalAddress((void*) 0xA000);

    mapPage(p, 0x1000, 3);
    mapPage(f, 0x1000, 3);

    p = getPhysicalAddress((void*) 0x9000);
    f = getPhysicalAddress((void*) 0xA000);

    uint64_t* y = (uint64_t*)0x9000;
    *y = 10;
    printNumber(*y, x);
    uint64_t* z = (uint64_t*) 0xA000;
    printNumber(*z, x);

    // Initialize devices
    remapPIC(0x20, 0x28);
    initIdt();
    enableAPIC();
    enableAPICTimer(10000);
    enableKeyboard(ioapics[0]->address);

    for (;;) 
    {
		asm volatile ("hlt");
    }
}
