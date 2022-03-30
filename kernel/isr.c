__attribute__((noreturn))
void exception_handler(void);
void exception_handler() {
    asm volatile ("cli; hlt"); // Completely hangs the computer
}
