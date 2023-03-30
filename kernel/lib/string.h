#ifndef STRING_H
#define STRING_H

#include <stddef.h>

size_t strlen(const char* s);
char* strcpy(char* destination, char* source);
int strcmp(char* destination, char* source);
void* reverse(char* s);
void* memcpy(void* destination, void* source, size_t num);
void* memset(void* ptr, int value, size_t num);

#endif 
