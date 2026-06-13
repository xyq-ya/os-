BITS 32

global gdt_init
global gdt_set_kernel_stack
global tss_entry

struc TSS
    .back_link resd 1
    .esp0      resd 1
    .ss0       resw 1
    .ss0_pad   resw 1
    .esp1      resd 1
    .ss1       resw 1
    .ss1_pad   resw 1
    .esp2      resd 1
    .ss2       resw 1
    .ss2_pad   resw 1
    .cr3       resd 1
    .eip       resd 1
    .eflags    resd 1
    .eax       resd 1
    .ecx       resd 1
    .edx       resd 1
    .ebx       resd 1
    .esp       resd 1
    .ebp       resd 1
    .esi       resd 1
    .edi       resd 1
    .es        resw 1
    .es_pad    resw 1
    .cs        resw 1
    .cs_pad    resw 1
    .ss        resw 1
    .ss_pad    resw 1
    .ds        resw 1
    .ds_pad    resw 1
    .fs        resw 1
    .fs_pad    resw 1
    .gs        resw 1
    .gs_pad    resw 1
    .ldt       resw 1
    .ldt_pad   resw 1
    .trap      resw 1
    .iomap     resw 1
endstruc

section .bss
align 16
tss_entry:
    resb TSS_size

section .data
align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
    dq 0x00CFFA000000FFFF
    dq 0x00CFF2000000FFFF
tss_desc:
    dw 0
    dw 0
    db 0
    db 0x89
    db 0
    db 0
gdt_end:

align 4
gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

section .text
gdt_set_kernel_stack:
    mov eax, [esp + 4]
    mov [tss_entry + TSS.esp0], eax
    mov word [tss_entry + TSS.ss0], 0x10
    ret

gdt_init:
    cli
    mov [tss_entry + TSS.esp0], esp
    mov word [tss_entry + TSS.ss0], 0x10

    mov eax, tss_entry
    mov word [tss_desc + 0], TSS_size - 1
    mov word [tss_desc + 2], ax
    shr eax, 16
    mov byte [tss_desc + 4], al
    mov byte [tss_desc + 7], ah

    lgdt [gdt_ptr]
    jmp 0x08:.flush_cs

.flush_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ax, 0x28
    ltr ax
    ret
