BITS 32

global cpu_switch_to_user

extern gdt_set_kernel_stack

section .text
cpu_switch_to_user:
    mov edi, [esp + 4]
    mov ecx, [esp + 8]

    mov edx, esp
    push edx
    call gdt_set_kernel_stack
    add esp, 4

    push 0x23
    push ecx
    pushfd
    or dword [esp], 0x200
    push 0x1B
    push edi
    iret
