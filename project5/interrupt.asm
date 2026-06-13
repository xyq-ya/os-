BITS 32

global isr_default
global isr_80
global isr_irq0

extern kernel_syscall_dispatch
extern timer_interrupt_handler

section .text
isr_default:
    cli
.hang:
    hlt
    jmp .hang

isr_irq0:
    push ds
    push es
    push fs
    push gs
    pusha

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp
    call timer_interrupt_handler
    add esp, 4

    popa
    pop gs
    pop fs
    pop es
    pop ds
    iretd

isr_80:
    push ds
    push es
    push fs
    push gs
    pusha

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    cli

    mov eax, [esp + 28]
    mov ebx, [esp + 16]
    mov ecx, [esp + 24]
    mov edx, [esp + 20]

    push edx
    push ecx
    push ebx
    push eax
    push esp
    call kernel_syscall_dispatch
    add esp, 20
    cmp eax, -2
    je .skip_eax_write
    mov [esp + 28], eax
.skip_eax_write:

    popa
    pop gs
    pop fs
    pop es
    pop ds
    iretd
