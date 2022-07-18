#include <stdint.h>

typedef struct Register
{
	uint64_t rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp;
	uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
	uint64_t rflags; 
	uint64_t rip; 
	uint64_t cr3;
}__attribute__((packed)) Register;

typedef struct Task 
{
	Register regs;
	uint64_t cs, ds, fs, gs; 
	// General purpose registers
	uint64_t state;
	uint64_t ldtr;
	uint64_t io_map_address, io_map;
	// Stack pointers
	uint64_t sp0, sp1, sp2;
	uint64_t ssp_state;
	struct Task* next;
}__attribute__((packed)) Task;

typedef struct TSSDescriptor {
	uint16_t limit1;
	uint16_t base1;
	uint8_t base2;
	// type[4] 0 DPL[0:1] P[1]
	uint8_t flags1;
	// limit[16:19] AVL[1] 0 0 G[1]
	uint8_t flags2;
	uint8_t base3;
	uint32_t base4;
	uint32_t reserved;
} TSSDescriptor;

typedef struct TSS 
{
	uint32_t reserved;	
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved3;	
	uint16_t reserved4;	
	uint16_t io_map_address;	
}__attribute__((packed)) TSS;
