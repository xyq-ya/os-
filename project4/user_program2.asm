BITS 32

global user_program_entry

section .rodata
user_msg db "[user2] running via syscall", 10
user_msg_len equ $ - user_msg

section .text
user_program_entry:
.loop:
    mov eax, 1
    mov ebx, user_msg
    mov ecx, user_msg_len
    int 0x80

    mov ecx, 400000
.delay:
    loop .delay
    jmp .loop
