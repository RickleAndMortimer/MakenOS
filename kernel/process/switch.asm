global switchTask
switchTask:
	mov [rdi], rax ; Save state of the task
	mov [rdi + 8], rbx
	mov [rdi + 16], rcx
	mov [rdi + 24], rdx
	mov [rdi + 32], rsi
	mov [rdi + 40], rdi
	mov [rdi + 48], rsp
	mov [rdi + 56], rbp
	mov [rdi + 64], r8
	mov [rdi + 72], r9
	mov [rdi + 80], r10
	mov [rdi + 88], r11
	mov [rdi + 96], r12
	mov [rdi + 104], r13
	mov [rdi + 112], r14
	mov [rdi + 120], r15

	pushfq ; Save RFLAGS
	pop rcx
	mov [rdi + 128], rcx

	mov rcx, [rsp] ; Save RIP
	mov [rdi + 136], rcx
	push rcx

	mov rax, [rsi] ; Load the state of the next task
	mov rbx, [rsi + 8]
	mov rdx, [rsi + 24]
	mov rdi, [rsi + 40]
	mov r8, [rsi + 64]
	mov r9, [rsi + 72]
	mov r10, [rsi + 80]
	mov r11, [rsi + 88]
	mov r12, [rsi + 96]
	mov r13, [rsi + 104]
	mov r14, [rsi + 112]
	mov r15, [rsi + 120]

	mov rcx, [rsi + 128]

	push rcx ; Load RFLAGS
	popfq

	mov rbp, [rsi + 56]	
	mov rsp, [rsi + 48]

	mov rcx, [rsi + 136] ; Load new RIP
	push rcx
	mov rcx, [rsi + 16]

	mov rsi, [rsi + 32]

	ret
