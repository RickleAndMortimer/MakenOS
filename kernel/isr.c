#include <kernel.h>
#include <isr.h>
#include <stdint.h>

size_t strlen(char* s) {
    size_t len = 0;
    while(s[len] != '\0') {len++;};
    return len;
}

void reverse(char* s) {
    int i, j;
    char tmp;
    for (i=0,j=strlen(s)-1; i < j; i++, j--){
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
    
}

void itoa(int value, char* str, int base) {
    int i = 0;
    int sign;
    if ((sign = value) < 0) {
        value = -value;
    }
    do {
        str[i++] = value % base + '0';
        
    } while ((value /= base) > 0);
    if (sign < 0)
        str[i++] = '-';
    str[i] = '\0';
    reverse(str);
}
void uint64toa(uint64_t value, char* str, int base) {
    int i = 0;
    do {
        str[i++] = value % base + '0';
    } while ((value /= base) > 0);
    str[i] = '\0';
    reverse(str);
}

void printNumber(uint64_t num, char* x) {
	uint64toa(num, x, 10);
	term_write(x, strlen(x));
	term_write("\n", 1);
}

void exception_handler(uint64_t int_no, uint64_t err_code){ 
	char* x;
	term_write("\n",1);
	printNumber(int_no, x);
	printNumber(err_code, x);
	asm volatile("cli; hlt");
}
