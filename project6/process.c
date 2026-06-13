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
    proc->state = PROC_READY;
    proc->entry = entry;
    proc->stack_top = stack_top;
    proc->has_run = 0;
    proc->cs = 0x1B;
    proc->user_ss = 0x23;
    proc->eflags = 0x202;
    proc->eip = entry;
    proc->user_esp = stack_top;
    proc->eax = 0;
    proc->ebx = 0;
    proc->ecx = 0;
    proc->edx = 0;

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
    proc->eflags = 0x202;
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
    frame[14] = 0x202;
    frame[15] = proc->user_esp;
    frame[16] = proc->user_ss;
}

static int pick_next_from(int from_index, int want_state) {
    int next = from_index;//从哪个进程开始找
    int tries = 0;

    do {
        next = (next + 1) % process_count;
        tries++;//尝试次数加1
        if (processes[next].state == want_state) {
            return next;//如果找到的进程状态为want_state，则返回这个进程的索引
        }
    } while (tries < process_count);

    return -1;
}

static int next_runnable(int from_index) {
    return pick_next_from(from_index, PROC_READY);
}//找到下一个非阻塞进程并返回这个进程的索引

void process_block(uint32_t* frame) {
    process_t* current = &processes[current_index];
    save_frame(current, frame);
    current->state = PROC_BLOCKED;
}//将当前进程状态设置为阻塞

void process_wake_one(void) {
    int idx = pick_next_from(current_index, PROC_BLOCKED);
    if (idx >= 0) {
        processes[idx].state = PROC_READY;
    }
}//找到下一个阻塞进程并将其状态设置为就绪

int process_switch_to(int index, uint32_t* frame) {
    if (index < 0 || index >= process_count) {
        return 0;
    }

    process_t* current = &processes[current_index];
    if (current->state == PROC_READY) {
        save_frame(current, frame);
    }

    current_index = index;
    restore_frame(&processes[current_index], frame);
    return 1;
}

void process_restart_current_at_entry(uint32_t* frame) {
    process_t* current = &processes[current_index];
    current->eip = current->entry;
    current->eax = 0;
    frame[12] = current->entry;
    frame[7] = 0;
}

void syscall_yield(uint32_t* frame) {
    process_t* current = &processes[current_index];

    if (current->state == PROC_READY) {
        save_frame(current, frame);
    }

    int next = next_runnable(current_index);
    if (next < 0) {
        return;
    }

    current_index = next;
    restore_frame(&processes[current_index], frame);
}

void timer_interrupt_handler(uint32_t* frame) {
    process_t* current = &processes[current_index];

    pic_send_eoi(PIC_IRQ_TIMER);
    save_frame(current, frame);

    current_index = (current_index + 1) % process_count;
    restore_frame(&processes[current_index], frame);
}

int process_current_index(void) {
    return current_index;
}

void scheduler_start(void) {
    if (process_count == 0) {
        return;
    }

    current_index = 0;
    cpu_switch_to_user(processes[0].entry, processes[0].stack_top);
}// 启动调度器
