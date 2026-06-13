BITS 32

global isr_default
global isr_80

extern kernel_syscall_dispatch

section .text
isr_default:
    cli
.hang:
    hlt
    jmp .hang

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

    mov eax, [esp + 28]
    mov ebx, [esp + 16]
    mov ecx, [esp + 24]
    mov edx, [esp + 20]

    push edx
    push ecx
    push ebx
    push eax
    call kernel_syscall_dispatch
    add esp, 16
    mov [esp + 28], eax

    popa
    pop gs
    pop fs
    pop es
    pop ds
    iretd
