#include "common.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "kernel_runtime.h"

void kmain(void) {
    gdt_init(); // 初始化 GDT/TSS（boot.asm 里只有临时引导用 GDT）
    idt_init(); // 初始化 IDT，挂接 int 0x80

    kclear();
    kputs("Kernel start\n");
    kputs("Project 3: ELF loader + ring3 user program (int 0x80).\n");

    kernel_run_demo();

    for (;;) {
        asm volatile("hlt");
    }
}
