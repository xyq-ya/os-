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
;执行到这里的时候我都理解应该是esp是返回位置，+4为基地址，+8为界限
section .data
align 4
idt_ptr:
    dw 0
    dd 0
