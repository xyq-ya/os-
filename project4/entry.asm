BITS 32

global _start

global stack_top

extern kmain

section .text
_start:
    mov esp, stack_top
    call kmain
.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 8192
stack_top:
