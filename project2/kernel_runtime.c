#include "common.h"
#include "print.h"
#include "kernel_runtime.h"

extern void user_program_entry(void);
extern void cpu_switch_to_user(uint32_t entry, uint32_t user_stack_top);

static uint8_t user_stack[4096];

uint32_t kernel_syscall_dispatch(uint32_t num, uint32_t arg1,
                                 uint32_t arg2, uint32_t arg3) {
    (void)arg3;

    if (num == 1) {
        const char* s = (const char*)arg1;
        uint32_t len = arg2;
        for (uint32_t i = 0; i < len; ++i) {
            kputc(s[i]);
        }
        return 0;
    }

    if (num == 2) {
        kputs("[kernel] syscall 2 -> halt\n");
        asm volatile("cli");
        for (;;) {
            asm volatile("hlt");
        }
    }

    kprintf("[kernel] unknown syscall: %d\n", (int)num);
    return (uint32_t)-1;
}

void kernel_run_demo(void) {
    kputs("Switching to user mode (ring 3)...\n");

    cpu_switch_to_user((uint32_t)user_program_entry,
                       (uint32_t)(user_stack + sizeof(user_stack)));
}
