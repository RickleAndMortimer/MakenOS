#include <stdint.h>
#include <stddef.h>
#include <string.h>

size_t strlen(char* s) 
{
    size_t len = 0;
    while(s[len] != '\0') { len++; };
    return len;
}

void reverse(char* s) 
{
    int i, j;
    char tmp;
    for (i=0,j=strlen(s)-1; i < j; i++, j--)
    {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

