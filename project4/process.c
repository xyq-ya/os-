#include "common.h"
#include "process.h"
#include "pic.h"
#include "switch.h"

static process_t processes[MAX_PROCESSES];
static int process_count = 0;
static int current_index = 0;

int process_create(uint32_t entry, uint32_t stack_top) {
    if (process_count >= MAX_PROCESSES) {
        return -1;
    }

    process_t* proc = &processes[process_count];
    proc->used = 1;
    proc->pid = process_count;
    proc->entry = entry;
    proc->stack_top = stack_top;
    proc->has_run = 0;
    proc->cs = 0x1B;
    proc->user_ss = 0x23;
    proc->eflags = 0x202;
    proc->eip = entry;
    proc->user_esp = stack_top;

    return process_count++;
}

static void save_frame(process_t* proc, uint32_t* frame) {
    proc->edi = frame[0];
    proc->esi = frame[1];
    proc->ebp = frame[2];
    proc->ebx = frame[4];
    proc->edx = frame[5];
    proc->ecx = frame[6];
    proc->eax = frame[7];
    proc->eip = frame[12];
    proc->cs = frame[13];
    proc->eflags = frame[14];
    proc->user_esp = frame[15];
    proc->user_ss = frame[16];
    proc->has_run = 1;
}

static void restore_frame(process_t* proc, uint32_t* frame) {
    frame[0] = proc->edi;
    frame[1] = proc->esi;
    frame[2] = proc->ebp;
    frame[4] = proc->ebx;
    frame[5] = proc->edx;
    frame[6] = proc->ecx;
    frame[7] = proc->eax;
    frame[12] = proc->eip;
    frame[13] = proc->cs;
    frame[14] = proc->eflags;
    frame[15] = proc->user_esp;
    frame[16] = proc->user_ss;
}

void timer_interrupt_handler(uint32_t* frame) {
    process_t* current = &processes[current_index];

    pic_send_eoi(PIC_IRQ_TIMER);
    save_frame(current, frame);

    current_index = (current_index + 1) % process_count;
    restore_frame(&processes[current_index], frame);
}

void scheduler_start(void) {
    if (process_count == 0) {
        return;
    }

    current_index = 0;
    cpu_switch_to_user(processes[0].entry, processes[0].stack_top);
}
