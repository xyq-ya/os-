BITS 32

global user_program_entry

section .rodata
user_msg db "[user] hello via syscall 1", 10
user_msg_len equ $ - user_msg

section .text
user_program_entry:
    mov eax, 1
    mov ebx, user_msg
    mov ecx, user_msg_len
    int 0x80

    mov eax, 2
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    int 0x80

.hang:
    jmp .hang
