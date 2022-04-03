#include <kernel.h>
#include <isr.h>
#include <string.h>
#include <stdint.h>


void itoa(uint64_t value, char* str, uint64_t base) {
	uint64_t i = 0;
	uint64_t sign;
	if ((sign = value) < 0) {
		value = -value;
	}

	do {
		str[i++] = value % base;
		value /= base;
	}
	while (value > 9);

	if (sign < 0)
		str[i++] = '_';
	str[i] = '\0';
	reverse(str);
}

void reverse(char* s) {
	int i, j; 
	char tmp;
	for (i=0, j=strlen(s); i < j; i++, j--) {
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;	
	}	
}
size_t strlen(const char* s) {
	size_t len = 0;	
	while(s[len++] != '\0') {}
	return len;
}

void exception_handler(uint64_t rip, uint64_t cs, uint64_t rflags, uint64_t rsp) { 
	char* x;
	itoa(rip, x, 10);
	term_write(x, strlen(x));
	term_write("HA", 2);
	asm volatile("cli; hlt");
}
