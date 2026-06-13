#ifndef PROCESS_H
#define PROCESS_H

#include "common.h"

#define MAX_PROCESSES 2

typedef struct process {
    int used;
    int pid;
    uint32_t entry;
    uint32_t stack_top;
    int has_run;

    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t user_esp;
    uint32_t user_ss;
} process_t;

int process_create(uint32_t entry, uint32_t stack_top);
void scheduler_start(void);
void timer_interrupt_handler(uint32_t* frame);

#endif
