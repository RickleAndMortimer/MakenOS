#include <stdint.h>
#include <stddef.h>
#include <string.h>

size_t strlen(char* s) 
{
    size_t len = 0;
    while(s[len] != '\0') { len++; };
    return len;
}

void* memset(void* ptr, int value, size_t num) 
{
    uint8_t* k = ptr;
    uint8_t l = value;
    for (size_t i = 0; i < num; i++) {
	k[i] = l;
    }
    return ptr;
}

void* memcpy(void* destination, void* source, size_t num) 
{
    uint8_t* k = destination;
    uint8_t* l = source;
    for (size_t i = 0; i < num; i++) {
	k[i] = l[i];
    }
    return destination;
}

void* reverse(char* s) 
{
    int i, j;
    char tmp;
    for (i=0,j=strlen(s)-1; i < j; i++, j--)
    {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
    return s;
}

