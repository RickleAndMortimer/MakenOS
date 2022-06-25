#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void itoa(int value, char* str, int base) 
{
    int i = 0;
    uint8_t negative = value < 0;
        
    value = negative ? -value : value;

    do 
    {
        str[i++] = value % base + '0';
    } 
    while ((value /= base) > 0);

    if (negative)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str);
}

void uint64toa(uint64_t value, char* str, int base) 
{
    int i = 0;
    do 
    {
        str[i++] = value % base + '0';
    } 
    while ((value /= base) > 0);
    str[i] = '\0';
    reverse(str);
}
