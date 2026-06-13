Project 2 Overview

Boot Flow
- MBR loads the kernel from disk into 0x1000, enables A20, loads a small GDT, and switches to protected mode.
- Control transfers to the kernel entry point at 0x1000, which sets up a stack and calls kmain.

Kernel Layout
- GDT setup is in kernel/gdt.c with helpers in kernel/gdt.asm to load the GDT and TSS.
- IDT setup is in kernel/idt.c with the lidt helper in kernel/idt.asm.
- Syscall handler is in kernel/syscall.c; interrupt stub is in kernel/interrupts.asm.
- VGA text output and printf-like formatting are in kernel/print.c.

System Call Demo
- kernel/user.asm runs in ring 3 and triggers int 0x80.
- The syscall convention here is eax=syscall_number, ebx=ptr, ecx=len.
- Syscall 1 prints a user string; syscall 2 halts in kernel mode for a stable end state.

Why There Are Same-Name Files
- gdt.c and gdt.asm are different: C builds tables, ASM loads them.
- idt.c and idt.asm are different: C builds tables, ASM loads them.

Build Output
- All build artifacts go under build/ (kernel.elf, kernel.bin, os-image.bin, boot.bin, and obj files).
