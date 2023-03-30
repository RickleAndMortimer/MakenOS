#include <stdlib.h>

#include "print.h"
#include "string.h"
#include "stdlib.h"
#include "../kernel.h"

void printNumber(uint64_t num) 
{
    char x[20];
    uint64toa(num, x, 10);
    term_write(x, strlen(x));
    term_write("\n", 1);
}
