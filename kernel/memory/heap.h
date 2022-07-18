#include <stdint.h>
#include <stddef.h>

void* kernel_malloc(size_t size);
void kernel_free(void* ptr);
