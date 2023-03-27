#include "devices/ahci.h"
#include "devices/apic.h"
#include "devices/ioapic.h" 
#include "devices/pci.h" 
#include "devices/pic.h"
#include "devices/pit.h"
#include "devices/ps2.h" 
#include "devices/serial.h"
#include "filesystem/file.h"
#include "filesystem/initrd.h"
#include "interfaces/description_tables/madt.h"
#include "interrupts/idt.h" 
#include "lib/print.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "process/task.h"
#include "limine.h"
#include "lib/string.h"

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
    .flags = 1
};

static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 8192,
};

extern struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

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

struct limine_terminal* main_terminal;
struct limine_memmap_response* memmap_info;

extern fs_node_t* fs_root;
extern initrd_header_t* initrd_header;

extern void term_write(const char *string, size_t length) {
    terminal_request.response->write(main_terminal, string, length);
}

// Kernel entrypoint
void _start(void) {
    const char* hello_msg = "Hello World";
    main_terminal = terminal_request.response->terminals[0];

    // Access RSDP for ACPI
    rsdp_descriptor = (RSDPDescriptor20*) rsdp_request.response->address; 
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

    // Get LAPIC info
    for (uint8_t i = 0; i < 5; i++) 
    {
        term_write("IOAPIC\n", 7);
        printNumber(ioapic_source_overrides[i]->bus_source);
        printNumber(ioapic_source_overrides[i]->IRQ_source);
        printNumber(ioapic_source_overrides[i]->global_system_interrupt);
    }


    struct limine_smp_response* smp_response = smp_request.response;
    if (smp_response) 
    {
        term_write("limine's results\n", 18);
        printNumber(smp_response->cpu_count);
        printNumber(smp_response->flags);
        for (uint64_t i = 0; i < smp_response->cpu_count; i++) {
            printNumber(smp_response->cpus[i]->lapic_id);
            printNumber(smp_response->cpus[i]->processor_id);
        }
    }

    term_write("results done\n", 14);


    // Initialize paging and memory management
    memmap_info = memmap_request.response;
    initPML4(); 
    printMemoryMaps();
    setMemoryMap(4);

    // Initialize devices
    remapPIC(0x20, 0x28);
    initIdt();

    enableAPIC();
    enableAPICTimer(10);

    /* test that paging works
   
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

    /* Test basic filesystem code

    inode_table* f = initRamFS();
    inode* i = fopen("neighbor");
    
    char jk[18];
    f_malloc(i, 0);
    fwrite(i, "jerry seinfelding\n", 19);

    fread(i, jk, 1, 19);
    term_write(jk, 19);

    fwrite(i, "jerry seinfelded\n", 18);
    fread(i, jk, 1, 18);
    term_write(jk, 18);
    
    uint16_t rose = pciConfigReadWord(0, 31, 3, 0x20);
    printNumber(rose);

    rose = pciConfigReadWord(0, 31, 3, 0x22);
    printNumber(rose);

    */

	enableSerialCOM1(ioapics[0]->address);
    checkMSI(0, 31, 2);
    
    // Test AHCI drivers for a successful read
    HBA_MEM* host = (HBA_MEM*) 0xFEBD5000;
    probePort(host);

	initTasking();
    uint16_t* s = k_malloc(4096);
    if (read(&host->ports[0], 0, 0, 1, s))
    {
        term_write((char*) s, 13);
        term_write("\nFile successfully read!\n", 25);
    }

    uint8_t* c = k_malloc(4096);
    c = "jello";

    if (write(&host->ports[0], 0, 0, 1, (uint16_t*) c)) 
    {
        term_write((char*) c, 6);
        term_write("\nFile successfully written!\n", 28);
    }

    uint8_t* g = k_malloc(400);
    g[0] = 4;
    g[1] = 8;
    g[2] = 12;
    printNumber(g[0]);
    printNumber(g[1]);
    printNumber(g[2]);
    k_free(g);

    int i = 0;
    struct dirent *node = 0;

    struct limine_file* module = module_request.response->modules[0];
    void* cl = module->address;

    size_t initrd_location = (size_t) module->address;
    size_t initrd_end = (size_t)(module->address + 1);
    fs_root = initialise_initrd(initrd_location);

    while ((node = readdir_fs(fs_root, i)) != 0)
    {
        term_write("Found file ", strlen("Found file "));
        term_write(node->name, strlen(node->name));
        fs_node_t* fsnode = finddir_fs(fs_root, node->name);

        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
            term_write("\n\t(directory)\n", strlen("\n\t(directory)\n"));
        else
        {
            term_write("\n\t contents: \"", strlen("\n\t contents: \""));
            char buf[256];
            size_t sz = read_fs(fsnode, 0, 256, buf);
            int j;
            term_write(buf, strlen(buf));
            term_write("\"\n", strlen("\"\n"));
        }
        i++;
    }

	enableKeyboard(ioapics[0]->address);

    for (;;) 
    {
		__asm__ volatile ("hlt");
    }
}
