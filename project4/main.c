#include "common.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "kernel_runtime.h"

void kmain(void) {
    gdt_init();
    idt_init();

    kclear();
    kputs("Kernel start\n");
    kputs("Project 4: timer IRQ + round-robin scheduler.\n");

    kernel_run_demo();

    for (;;) {
        asm volatile("hlt");
    }
}
