#ifndef FAT32_H
#define FAT32_H

#include "common.h"

#define FAT32_MAX_FILE_SIZE 65536u

typedef struct fat32_bpb {
    uint8_t jmp_boot[3];
    char oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed)) Fat32Bpb;

typedef struct fat32_fs {
    uint8_t drive;
    Fat32Bpb bpb;
    uint32_t fat_begin_lba;
    uint32_t data_begin_lba;
    uint32_t root_dir_cluster;
    uint32_t bytes_per_cluster;
} Fat32Fs;

int fat32_mount(uint8_t drive, Fat32Fs* fs);
void fat32_print_info(const Fat32Fs* fs);
int fat32_read_file(const Fat32Fs* fs, const char* name_83, uint8_t* buffer,
                    uint32_t buffer_size, uint32_t* bytes_read);

#endif
