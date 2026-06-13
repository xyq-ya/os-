BITS 32

global user_program_entry

section .text
user_program_entry:
.loop:
    mov eax, 5
    mov ebx, 1
    int 0x80
    cmp eax, 1
    je .retry

    mov ecx, 15000000
.delay:
    loop .delay
    jmp .loop

.retry:
    pause
    jmp .retry
