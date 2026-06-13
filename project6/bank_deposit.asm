BITS 32

global user_program_entry

section .text
user_program_entry:
    mov eax, 5
    xor ebx, ebx
    int 0x80

    mov ecx, 20000000
.delay:
    loop .delay

    mov eax, 7
    int 0x80

.done:
    hlt
    jmp .done
