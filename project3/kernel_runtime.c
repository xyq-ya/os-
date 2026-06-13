#include "common.h"
#include "print.h"
#include "kernel_runtime.h"
#include "switch.h"
#include "elf.h"

extern uint8_t _binary_build_user_program_elf_start[];
extern uint8_t _binary_build_user_program_elf_end[];

#define USER_PARTITION_BASE 0x200000u // 用户分区基地址
#define USER_PARTITION_SIZE  0x100000u // 用户分区大小
#define USER_STACK_SIZE     4096u

static uint8_t* user_partition_cursor = (uint8_t*)USER_PARTITION_BASE;

static uint32_t align_up(uint32_t value, uint32_t align) {
    return (value + align - 1u) & ~(align - 1u);
    // 向上对齐：先把 value 膨胀到下一个刻度门槛，再用 & ~(align-1) 向下对齐
}

static void* partition_alloc(uint32_t size, uint32_t align) {
    uint32_t cursor = align_up((uint32_t)user_partition_cursor, align);
    uint32_t limit = USER_PARTITION_BASE + USER_PARTITION_SIZE;

    if (cursor + size > limit) {
        return 0;
    }

    user_partition_cursor = (uint8_t*)(cursor + size);
    return (void*)cursor;
    // 可变分区分配：移动游标，类似 project3 实验要求的 Variable Partitioning
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
    } // 检查文件是否够大，ELF 头固定 52 字节

    const Elf32_Ehdr* ehdr = (const Elf32_Ehdr*)image;
    if (ehdr->e_ident[0] != 0x7F || ehdr->e_ident[1] != 'E' ||
        ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        return 0;
    } // 检查魔数 0x7F 'E' 'L' 'F'

    if (ehdr->e_ident[4] != 1 || ehdr->e_ident[5] != 1 || ehdr->e_machine != 3) {
        return 0;
    } // 32 位小端 x86 ELF

    if (ehdr->e_phoff + (uint32_t)ehdr->e_phnum * sizeof(Elf32_Phdr) > image_size) {
        return 0;
    } // 程序头表不能越界

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
    } // 至少有一个有效 LOAD 段

    uint32_t image_bytes = align_up(highest_vaddr - lowest_vaddr, 0x1000u);
    uint32_t load_end = lowest_vaddr + image_bytes;

    if (load_end > USER_PARTITION_BASE + USER_PARTITION_SIZE) {
        return 0;
    }

    if ((uint32_t)user_partition_cursor < load_end) {
        user_partition_cursor = (uint8_t*)load_end;
    }
    // 按链接地址加载到用户分区（平坦内存：物理地址 = 链接地址）

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
    const uint8_t* image = _binary_build_user_program_elf_start;
    uint32_t image_size = (uint32_t)(_binary_build_user_program_elf_end -
                                     _binary_build_user_program_elf_start);
    uint32_t entry = 0;
    void* user_stack = 0;

    kputs("Load ELF user program into memory partition...\n");
    if (!load_elf_image(image, image_size, &entry)) {
        kputs("ELF load failed.\n");
        asm volatile("cli");
        for (;;) {
            asm volatile("hlt");
        }
    }

    user_stack = partition_alloc(USER_STACK_SIZE, 16);
    if (!user_stack) {
        kputs("User stack alloc failed.\n");
        asm volatile("cli");
        for (;;) {
            asm volatile("hlt");
        }
    }

    kprintf("ELF entry = 0x%x\n", entry);
    kputs("Switching to loaded user program (ring 3)...\n");

    cpu_switch_to_user(entry, (uint32_t)user_stack + USER_STACK_SIZE);
}
