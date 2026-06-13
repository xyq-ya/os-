BITS 32

global idt_load

section .text
idt_load:
    mov eax, [esp + 4]
    mov [idt_ptr + 2], eax
    mov ax, [esp + 8]
    mov [idt_ptr], ax
    lidt [idt_ptr]
    ret

section .data
align 4
idt_ptr:
    dw 0
    dd 0
