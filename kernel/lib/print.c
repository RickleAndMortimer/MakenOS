#include <print.h>
#include <stdlib.h>
#include <string.h>
#include <kernel.h>

void printNumber(uint64_t num, char* x) 
{
    uint64toa(num, x, 10);
    term_write(x, strlen(x));
    term_write("\n", 1);
}
