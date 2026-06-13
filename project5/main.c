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
    kputs("Project 5: shared memory + P/V synchronization.\n");

    kernel_run_demo();

    for (;;) {
        asm volatile("hlt");
    }
}
