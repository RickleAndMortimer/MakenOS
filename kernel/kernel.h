#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>
extern void (*term_write)(const char *string, size_t length);

#endif
