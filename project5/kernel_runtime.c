#include "common.h"
#include "print.h"
#include "kernel_runtime.h"
#include "process.h"
#include "pic.h"
#include "pit.h"
#include "elf.h"
#include "shm.h"
#include "sync.h"

extern uint8_t _binary_build_bank_deposit_elf_start[];
extern uint8_t _binary_build_bank_deposit_elf_end[];
extern uint8_t _binary_build_bank_withdraw_elf_start[];
extern uint8_t _binary_build_bank_withdraw_elf_end[];

#define USER_PARTITION_BASE 0x200000u
#define USER_PARTITION_SIZE 0x100000u
#define USER_STACK_SIZE     4096u

static uint8_t* user_partition_cursor = (uint8_t*)USER_PARTITION_BASE;

static uint32_t align_up(uint32_t value, uint32_t align) {
    return (value + align - 1u) & ~(align - 1u);
}

static void* partition_alloc(uint32_t size, uint32_t align) {
    uint32_t cursor = align_up((uint32_t)user_partition_cursor, align);
    uint32_t limit = USER_PARTITION_BASE + USER_PARTITION_SIZE;

    if (cursor + size > limit) {
        return 0;
    }

    user_partition_cursor = (uint8_t*)(cursor + size);
    return (void*)cursor;
}

static void mem_copy(void* dst, const void* src, uint32_t size) {
    uint8_t* out = (uint8_t*)dst;
    const uint8_t* in = (const uint8_t*)src;

    for (uint32_t i = 0; i < size; ++i) {
        out[i] = in[i];
    }
}

static void mem_zero(void* dst, uint32_t size) {
    uint8_t* out = (uint8_t*)dst;

    for (uint32_t i = 0; i < size; ++i) {
        out[i] = 0;
    }
}

static int load_elf_image(const uint8_t* image, uint32_t image_size, uint32_t* entry_out) {
    if (image_size < sizeof(Elf32_Ehdr)) {
        return 0;
    }

    const Elf32_Ehdr* ehdr = (const Elf32_Ehdr*)image;
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        return 0;
    }

    if (ehdr->e_ident[4] != 1 || ehdr->e_ident[5] != 1 || ehdr->e_machine != 3) {
        return 0;
    }

    if (ehdr->e_phoff + (uint32_t)ehdr->e_phnum * sizeof(Elf32_Phdr) > image_size) {
        return 0;
    }

    const Elf32_Phdr* phdr = (const Elf32_Phdr*)(image + ehdr->e_phoff);
    uint32_t lowest_vaddr = 0xFFFFFFFFu;
    uint32_t highest_vaddr = 0;

    for (uint16_t i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD || phdr[i].p_memsz == 0) {
            continue;
        }

        if (phdr[i].p_vaddr < lowest_vaddr) {
            lowest_vaddr = phdr[i].p_vaddr;
        }

        if (phdr[i].p_vaddr + phdr[i].p_memsz > highest_vaddr) {
            highest_vaddr = phdr[i].p_vaddr + phdr[i].p_memsz;
        }
    }

    if (lowest_vaddr == 0xFFFFFFFFu || highest_vaddr <= lowest_vaddr) {
        return 0;
    }

    uint32_t image_bytes = align_up(highest_vaddr - lowest_vaddr, 0x1000u);
    uint32_t load_end = lowest_vaddr + image_bytes;

    if (load_end > USER_PARTITION_BASE + USER_PARTITION_SIZE) {
        return 0;
    }

    if ((uint32_t)user_partition_cursor < load_end) {
        user_partition_cursor = (uint8_t*)load_end;
    }

    for (uint16_t i = 0; i < ehdr->e_phnum; ++i) {
        if (phdr[i].p_type != PT_LOAD || phdr[i].p_memsz == 0) {
            continue;
        }

        if (phdr[i].p_offset + phdr[i].p_filesz > image_size) {
            return 0;
        }

        uint8_t* dst = (uint8_t*)phdr[i].p_vaddr;
        mem_copy(dst, image + phdr[i].p_offset, phdr[i].p_filesz);
        if (phdr[i].p_memsz > phdr[i].p_filesz) {
            mem_zero(dst + phdr[i].p_filesz, phdr[i].p_memsz - phdr[i].p_filesz);
        }
    }

    *entry_out = ehdr->e_entry;
    return 1;
}

static int load_user_elf_image(const uint8_t* image, uint32_t image_size,
                               const char* name, uint32_t* entry_out) {
    if (!load_elf_image(image, image_size, entry_out)) {
        kprintf("ELF load failed: %s\n", name);
        return 0;
    }

    kprintf("Loaded %s, entry = 0x%x\n", name, *entry_out);
    return 1;
}

static int create_user_process(uint32_t entry) {
    void* user_stack = partition_alloc(USER_STACK_SIZE, 16);

    if (!user_stack) {
        kputs("User stack alloc failed.\n");
        return 0;
    }

    if (process_create(entry, (uint32_t)user_stack + USER_STACK_SIZE) < 0) {
        kputs("Process table full.\n");
        return 0;
    }

    return 1;
}

uint32_t kernel_syscall_dispatch(uint32_t* frame, uint32_t num, uint32_t arg1,
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
        return sync_try_P() ? 0u : 1u;
    }

    if (num == 3) {
        sync_V();
        return 0;
    }

    if (num == 4) {
        if (arg2 == 0) {
            kprintf("[deposit] balance=%d\n", (int)arg1);
        } else {
            kprintf("[withdraw] balance=%d\n", (int)arg1);
        }
        return 0;
    }

    if (num == 5) {
        int32_t* balance = shm_bank_balance();

        if (!sync_try_P()) {
            return 1;
        }

        if (arg1 == 0) {
            *balance += 100;
            kprintf("[deposit] balance=%d\n", (int)*balance);
        } else {
            *balance -= 50;
            kprintf("[withdraw] balance=%d\n", (int)*balance);
        }
        sync_V();
        return 0;
    }

    if (num == 6) {
        frame[7] = 0;
        int before = process_current_index();
        syscall_yield(frame);
        if (process_current_index() != before) {
            return SYSCALL_RESCHEDULED;
        }
        return 0;
    }

    kprintf("[kernel] unknown syscall: %d\n", (int)num);
    return (uint32_t)-1;
}

void kernel_run_demo(void) {
    const uint8_t* image1 = _binary_build_bank_deposit_elf_start;
    uint32_t size1 = (uint32_t)(_binary_build_bank_deposit_elf_end -
                                _binary_build_bank_deposit_elf_start);
    const uint8_t* image2 = _binary_build_bank_withdraw_elf_start;
    uint32_t size2 = (uint32_t)(_binary_build_bank_withdraw_elf_end -
                                _binary_build_bank_withdraw_elf_start);

    uint32_t entry1 = 0;
    uint32_t entry2 = 0;

    shm_init();
    sync_init();

    kputs("Load bank deposit/withdraw ELF programs...\n");

    if (!load_user_elf_image(image1, size1, "bank_deposit", &entry1) ||
        !load_user_elf_image(image2, size2, "bank_withdraw", &entry2) ||
        !create_user_process(entry1) ||
        !create_user_process(entry2)) {
        asm volatile("cli");
        for (;;) {
            asm volatile("hlt");
        }
    }

    pic_init();
    pit_init();
    kputs("Timer IRQ + P/V sync enabled. Bank demo starts...\n");

    scheduler_start();
}
