%macro pushad 0
    push rax      
    push rcx
    push rdx
    push rdi
    push rsi
    push r8
    push r9
    push r10
    push r11
%endmacro

%macro popad 0
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rdx      
    pop rcx
    pop rax
%endmacro

global switch
switch:
	pushad
	mov [rax], rax ; Save state of the task
	mov [rax + 8], rbx
	mov [rax + 16], rcx
	mov [rax + 24], rdx
	mov [rax + 32], rsi
	mov [rax + 40], rdi
	mov [rax + 48], rsp
	mov [rax + 56], rbp
	mov [rax + 64], r8
	mov [rax + 72], r9
	mov [rax + 80], r10
	mov [rax + 88], r11
	mov [rax + 96], r12
	mov [rax + 104], r13
	mov [rax + 112], r14
	mov [rax + 120], r15

	push rbx ; Save RFLAGS
	pushfq
	pop rbx
	mov [rax + 128], rbx
	pop rbx

	mov [rax + 136], rip
	mov [rax + 144], cr3

	mov rax, [rdx] ; Load the state of the next task
	mov rbx, [rdx + 8]
	mov rcx, [rdx + 16]
	mov rdx, [rdx + 24]
	mov rsi, [rdx + 32]
	mov rdi, [rdx + 40]
	mov r8, [rdx + 64]
	mov r9, [rdx + 72]
	mov r10, [rdx + 80]
	mov r11, [rdx + 88]
	mov r12, [rdx + 96]
	mov r13, [rdx + 104]
	mov r14, [rdx + 112]
	mov r15, [rdx + 120]

	push [rdx + 128] ; Load RFLAGS
	popf

	mov rip, [rdx + 136]
	mov cr3, [rdx + 144]
	mov rbp, [rdx + 56]
	mov rdx, [rdx + 48]
	ret
