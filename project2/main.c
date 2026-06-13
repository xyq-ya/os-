#include "common.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "kernel_runtime.h"

void kmain(void) {
    gdt_init();//初始化GDT，我的理解是在boot.asm只加载了临时GDT，这个函数是加载完整GDT
    idt_init();//初始化IDT

    kclear();//清屏
    kputs("Kernel start\n");
    kputs("Naive kernel: ring0 -> ring3 switch + int 0x80 syscall.\n");

    kernel_run_demo();

    for (;;) {
        asm volatile("hlt");
    }
}
