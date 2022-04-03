#include <stdint.h>

__attribute__((noreturn))
void exception_handler(uint64_t rip, uint64_t cs, uint64_t rflags, uint64_t rsp); 
