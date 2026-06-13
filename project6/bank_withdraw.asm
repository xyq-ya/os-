BITS 32

global user_program_entry

section .text
user_program_entry:
    mov eax, 5
    mov ebx, 1
    int 0x80

    mov ecx, 20000000
.delay:
    loop .delay

.done:
    hlt
    jmp .done
