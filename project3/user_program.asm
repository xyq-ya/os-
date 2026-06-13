BITS 32

global user_program_entry

section .rodata
user_msg db "[user] hello via syscall 1", 10
user_msg_len equ $ - user_msg

section .text
user_program_entry:
    ; syscall 1: 打印字符串 (ebx=地址, ecx=长度)
    mov eax, 1
    mov ebx, user_msg
    mov ecx, user_msg_len
    int 0x80

    ; syscall 2: 请求内核停机，方便观察演示输出
    mov eax, 2
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    int 0x80

.hang:
    jmp .hang
