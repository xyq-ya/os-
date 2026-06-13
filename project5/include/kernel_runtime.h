#ifndef KERNEL_RUNTIME_H
#define KERNEL_RUNTIME_H

#include "common.h"

#define SYSCALL_RESCHEDULED ((uint32_t)-2)

void kernel_run_demo(void);
uint32_t kernel_syscall_dispatch(uint32_t* frame, uint32_t num, uint32_t arg1,
                                 uint32_t arg2, uint32_t arg3);

#endif
