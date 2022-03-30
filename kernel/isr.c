__attribute__((noreturn))
void exception_handler(void);
void exception_handler() {
    asm volatile ("cli"); // Completely hangs the computer
    for (;;) {
    	asm volatile ("hlt");
    }
}
