#ifndef ELF_H
#define ELF_H

#include "common.h"

#define EI_NIDENT 16
#define PT_LOAD 1

typedef struct elf32_ehdr {
    unsigned char e_ident[EI_NIDENT];//ELF文件的标识符
    uint16_t e_type;//类型
    uint16_t e_machine;//机器码
    uint32_t e_version;//版本
    uint32_t e_entry;//入口地址
    uint32_t e_phoff;//程序头表偏移量
    uint32_t e_shoff;//节头表偏移量
    uint32_t e_flags;//标志
    uint16_t e_ehsize;//ELF头大小
    uint16_t e_phentsize;//程序头表项大小
    uint16_t e_phnum;//程序头表项数
    uint16_t e_shentsize;//节头表项大小
    uint16_t e_shnum;//节头表项数       
    uint16_t e_shstrndx;//节头表字符串索引
} __attribute__((packed)) Elf32_Ehdr;

typedef struct elf32_phdr {
    uint32_t p_type;//类型
    uint32_t p_offset;//偏移量
    uint32_t p_vaddr;//虚拟地址
    uint32_t p_paddr;//物理地址
    uint32_t p_filesz;//文件大小
    uint32_t p_memsz;//内存大小
    uint32_t p_flags;//标志
    uint32_t p_align;//对齐
} __attribute__((packed)) Elf32_Phdr;

#endif