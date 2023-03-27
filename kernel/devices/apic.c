#include <cpuid.h>
#include <stdint.h>

#include "apic.h"
#include "../kernel.h"
#include "pic.h"
#include "pit.h"
#include "../interrupts/isr.h"
#include "../lib/print.h"

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define APIC_EOI_REGISTER 0xB0

extern void term_write(const char*, size_t);

enum 
{
    CPUID_FEAT_ECX_SSE3         = 1 << 0, 
    CPUID_FEAT_ECX_PCLMUL       = 1 << 1,
    CPUID_FEAT_ECX_DTES64       = 1 << 2,
    CPUID_FEAT_ECX_MONITOR      = 1 << 3,  
    CPUID_FEAT_ECX_DS_CPL       = 1 << 4,  
    CPUID_FEAT_ECX_VMX          = 1 << 5,  
    CPUID_FEAT_ECX_SMX          = 1 << 6,  
    CPUID_FEAT_ECX_EST          = 1 << 7,  
    CPUID_FEAT_ECX_TM2          = 1 << 8,  
    CPUID_FEAT_ECX_SSSE3        = 1 << 9,  
    CPUID_FEAT_ECX_CID          = 1 << 10,
    CPUID_FEAT_ECX_SDBG         = 1 << 11,
    CPUID_FEAT_ECX_FMA          = 1 << 12,
    CPUID_FEAT_ECX_CX16         = 1 << 13, 
    CPUID_FEAT_ECX_XTPR         = 1 << 14, 
    CPUID_FEAT_ECX_PDCM         = 1 << 15, 
    CPUID_FEAT_ECX_PCID         = 1 << 17, 
    CPUID_FEAT_ECX_DCA          = 1 << 18, 
    CPUID_FEAT_ECX_SSE4_1       = 1 << 19, 
    CPUID_FEAT_ECX_SSE4_2       = 1 << 20, 
    CPUID_FEAT_ECX_X2APIC       = 1 << 21, 
    CPUID_FEAT_ECX_MOVBE        = 1 << 22, 
    CPUID_FEAT_ECX_POPCNT       = 1 << 23, 
    CPUID_FEAT_ECX_TSC          = 1 << 24, 
    CPUID_FEAT_ECX_AES          = 1 << 25, 
    CPUID_FEAT_ECX_XSAVE        = 1 << 26, 
    CPUID_FEAT_ECX_OSXSAVE      = 1 << 27, 
    CPUID_FEAT_ECX_AVX          = 1 << 28,
    CPUID_FEAT_ECX_F16C         = 1 << 29,
    CPUID_FEAT_ECX_RDRAND       = 1 << 30,
    CPUID_FEAT_ECX_HYPERVISOR   = 1 << 31,
 
    CPUID_FEAT_EDX_FPU          = 1 << 0,  
    CPUID_FEAT_EDX_VME          = 1 << 1,  
    CPUID_FEAT_EDX_DE           = 1 << 2,  
    CPUID_FEAT_EDX_PSE          = 1 << 3,  
    CPUID_FEAT_EDX_TSC          = 1 << 4,  
    CPUID_FEAT_EDX_MSR          = 1 << 5,  
    CPUID_FEAT_EDX_PAE          = 1 << 6,  
    CPUID_FEAT_EDX_MCE          = 1 << 7,  
    CPUID_FEAT_EDX_CX8          = 1 << 8,  
    CPUID_FEAT_EDX_APIC         = 1 << 9,  
    CPUID_FEAT_EDX_SEP          = 1 << 11, 
    CPUID_FEAT_EDX_MTRR         = 1 << 12, 
    CPUID_FEAT_EDX_PGE          = 1 << 13, 
    CPUID_FEAT_EDX_MCA          = 1 << 14, 
    CPUID_FEAT_EDX_CMOV         = 1 << 15, 
    CPUID_FEAT_EDX_PAT          = 1 << 16, 
    CPUID_FEAT_EDX_PSE36        = 1 << 17, 
    CPUID_FEAT_EDX_PSN          = 1 << 18, 
    CPUID_FEAT_EDX_CLFLUSH      = 1 << 19, 
    CPUID_FEAT_EDX_DS           = 1 << 21, 
    CPUID_FEAT_EDX_ACPI         = 1 << 22, 
    CPUID_FEAT_EDX_MMX          = 1 << 23, 
    CPUID_FEAT_EDX_FXSR         = 1 << 24, 
    CPUID_FEAT_EDX_SSE          = 1 << 25, 
    CPUID_FEAT_EDX_SSE2         = 1 << 26, 
    CPUID_FEAT_EDX_SS           = 1 << 27, 
    CPUID_FEAT_EDX_HTT          = 1 << 28, 
    CPUID_FEAT_EDX_TM           = 1 << 29, 
    CPUID_FEAT_EDX_IA64         = 1 << 30,
    CPUID_FEAT_EDX_PBE          = 1 << 31
};

const uint32_t CPUID_FLAG_MSR = 1 << 5;
 
static int cpuHasMSR()
{
    uint32_t eax, edx;
    cpuid(1, &eax, &edx);
    return edx & CPUID_FLAG_MSR;
}
 
void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
    __asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
 
void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
    __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

uintptr_t cpuGetAPICBase() 
{
    uint32_t eax, edx;
    cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);
 
#ifdef __PHYSICAL_MEMORY_EXTENSION__
    return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
#else
    return (eax & 0xfffff000);
#endif
}
  
/* Set the physical address for local APIC registers */
void cpuSetAPICBase(uintptr_t apic) 
{
    uint32_t edx = 0;
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
 
#ifdef __PHYSICAL_MEMORY_EXTENSION__
    edx = (apic >> 32) & 0x0f;
#endif
 
    cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

static int getModel(void)
{
    int ebx, unused;
    __cpuid(0, unused, ebx, unused, unused);
    return ebx;
}
 
static int checkAPIC(void)
{
    unsigned int eax, unused, edx;
    __get_cpuid(1, &eax, &unused, &unused, &edx);
    return edx & CPUID_FEAT_EDX_APIC;
}
 
uint32_t readAPICRegister(uint32_t reg)
{
    return *((volatile uint32_t *)(cpuGetAPICBase() + reg));
} 

void writeAPICRegister(uint32_t reg, uint32_t value)
{
    *((volatile uint32_t *)(cpuGetAPICBase() + reg)) = value;
}

void enableAPIC(void) 
{
    term_write("Enabling APIC\n", 14);
    // disable PIC
    __asm__ volatile ("mov $0xff, %al;"
		  "out %al, $0xa1;"
		  "out %al, $0x21;");
    disableAllIRQs();
    /* Hardware enable the Local APIC if it wasn't enabled */
    cpuSetAPICBase(cpuGetAPICBase());
    printNumber(cpuGetAPICBase());
 
    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    writeAPICRegister(0xF0, readAPICRegister(0xF0) | 0x1FF);
    term_write("APIC enabled!\n", 14);
}


// APIC TIMER
static void APIC_timer_callback(InterruptFrame* frame)
{
    // do timer stuff
    writeAPICRegister(0xB0, 0);
}

void enableAPICTimer(uint32_t frequency) 
{
    initPIT(frequency);
    // Enable APIC Timer
    writeAPICRegister(0x3E0, 0x3);
    writeAPICRegister(0x380, 0xFFFFFFFF);
    // Sleep for 10 ms
    term_write("Calibrating APIC Timer\n", 24);
    PIT_sleep(10);
    term_write("Finished Calibration\n", 22);
    // Mask APIC Timer interrupt
    writeAPICRegister(0x320, 0x10000);
    // Reinitialize APIC timer with calculated APIC ticks
    uint32_t apic_ticks = 0xFFFFFFFF - readAPICRegister(0x390);

    writeAPICRegister(0x320, 0x20 | 0x60000);
    writeAPICRegister(0x3E0, 0x3);
    writeAPICRegister(0x380, apic_ticks);

    registerInterruptHandler(32, &APIC_timer_callback);
}
