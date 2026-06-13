#include "common.h"
#include "print.h"
#include "disk.h"
#include "fat32.h"

static uint8_t sector_buf[512];

static int sector_equals(const char* a, const char* b, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') {
            ca = (char)(ca - 'a' + 'A');
        }
        if (cb >= 'a' && cb <= 'z') {
            cb = (char)(cb - 'a' + 'A');
        }
        if (ca != cb) {
            return 0;
        }
    }
    return 1;
}//比较两个字符串是否相等,并且不区分大小写

static int read_cluster(const Fat32Fs* fs, uint32_t cluster, uint8_t* buffer) {
    uint32_t lba = fs->data_begin_lba +
                   (cluster - 2u) * fs->bpb.sectors_per_cluster;

    for (uint8_t i = 0; i < fs->bpb.sectors_per_cluster; ++i) {
        if (!disk_read_sectors(fs->drive, lba + i, 1, sector_buf)) {
            return 0;
        }

        for (uint32_t j = 0; j < 512u; ++j) {
            buffer[i * 512u + j] = sector_buf[j];
        }
    }

    return 1;
}//读取簇成功返回1

static uint32_t next_cluster(const Fat32Fs* fs, uint32_t cluster) {
    uint32_t fat_offset = cluster * 4u;//每个FAT表项占四个字节
    uint32_t fat_sector = fs->fat_begin_lba + fat_offset / 512u;//计算FAT表项所在的扇区
    uint32_t ent_offset = fat_offset % 512u;//计算FAT表项在扇区内的偏移

    if (!disk_read_sectors(fs->drive, fat_sector, 1, sector_buf)) {
        return 0x0FFFFFFFu;
    }

    uint32_t next = *(uint32_t*)(sector_buf + ent_offset);
    return next & 0x0FFFFFFFu;
}//读取下一个簇元数据成功返回1

int fat32_mount(uint8_t drive, Fat32Fs* fs) {
    if (!disk_read_sectors(drive, 0, 1, sector_buf)) {
        kputs("FAT32: boot sector read failed.\n");
        return 0;
    }//读取引导扇区失败返回0

    fs->drive = drive;
    for (uint32_t i = 0; i < sizeof(Fat32Bpb); ++i) {//遍历BPB
        ((uint8_t*)&fs->bpb)[i] = sector_buf[i];//将BPB写入文件系统
    }//将BPB写入文件系统

    if (fs->bpb.bytes_per_sector != 512 || fs->bpb.fat_size_32 == 0 ||
        fs->bpb.sectors_per_cluster == 0) {
        kputs("FAT32: invalid BPB.\n");
        return 0;
    }//BPB无效返回0

    if (!sector_equals(fs->bpb.fs_type, "FAT32   ", 8)) {
        kputs("FAT32: filesystem type mismatch.\n");
        return 0;
    }//文件系统类型不匹配返回0

    fs->fat_begin_lba = fs->bpb.reserved_sectors;
    fs->data_begin_lba = fs->fat_begin_lba +
                         (uint32_t)fs->bpb.num_fats * fs->bpb.fat_size_32;
    fs->root_dir_cluster = fs->bpb.root_cluster;
    fs->bytes_per_cluster = (uint32_t)fs->bpb.bytes_per_sector *
                            fs->bpb.sectors_per_cluster;
    return 1;
}//挂载FAT32文件系统成功返回1

static void print_fixed_string(const char* s, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        kputc(s[i]);
    }
}//打印固定长度的字符串

void fat32_print_info(const Fat32Fs* fs) {
    const Fat32Bpb* b = &fs->bpb;

    kputs("--- FAT32 Boot Sector / BPB ---\n");
    kputs("OEM name: ");
    print_fixed_string(b->oem_name, 8);
    kputc('\n');
    kprintf("Bytes per sector: %d\n", (int)b->bytes_per_sector);
    kprintf("Sectors per cluster: %d\n", (int)b->sectors_per_cluster);
    kprintf("Reserved sectors: %d\n", (int)b->reserved_sectors);
    kprintf("Number of FATs: %d\n", (int)b->num_fats);
    kprintf("FAT size (sectors): %d\n", (int)b->fat_size_32);
    kprintf("Root dir cluster: %d\n", (int)b->root_cluster);
    kprintf("Total sectors: %d\n", (int)b->total_sectors_32);
    kputs("Volume label: ");
    print_fixed_string(b->volume_label, 11);
    kputc('\n');
    kputs("FS type: ");
    print_fixed_string(b->fs_type, 8);
    kputc('\n');
    kputs("--- FAT32 Layout ---\n");
    kprintf("FAT begin LBA: %d\n", (int)fs->fat_begin_lba);
    kprintf("Data begin LBA: %d\n", (int)fs->data_begin_lba);
    kprintf("Bytes per cluster: %d\n", (int)fs->bytes_per_cluster);
    kputs("Root directory entries (cluster chain):\n");

    uint8_t cluster_buf[4096];
    if (fs->bytes_per_cluster > sizeof(cluster_buf)) {
        kputs("FAT32: cluster too large to dump.\n");
        return;
    }

    if (!read_cluster(fs, fs->root_dir_cluster, cluster_buf)) {
        kputs("FAT32: root cluster read failed.\n");
        return;
    }

    for (uint32_t off = 0; off < fs->bytes_per_cluster; off += 32u) {
        uint8_t* e = cluster_buf + off;
        if (e[0] == 0x00) {
            break;
        }
        if (e[0] == 0xE5) {
            continue;
        }
        if (e[11] == 0x0F) {
            continue;
        }

        kputs("  ");
        print_fixed_string((const char*)e, 8);
        kputc(' ');
        print_fixed_string((const char*)(e + 8), 3);
        kprintf(" attr=0x%x size=%d\n",
                (unsigned)e[11],
                (int)*(uint32_t*)(e + 28));
    }
}//打印根目录项

static int find_root_entry(const Fat32Fs* fs, const char* name_83,
                           uint32_t* first_cluster, uint32_t* file_size) {
    uint8_t cluster_buf[4096];

    if (fs->bytes_per_cluster > sizeof(cluster_buf)) {
        return 0;
    }

    if (!read_cluster(fs, fs->root_dir_cluster, cluster_buf)) {
        return 0;
    }

    for (uint32_t off = 0; off < fs->bytes_per_cluster; off += 32u) {
        uint8_t* e = cluster_buf + off;//当前目录项
        if (e[0] == 0x00) {
            break;
        }//文件名结束符返回0
        if (e[0] == 0xE5 || e[11] == 0x0F) {
            continue;
        }//文件名无效返回0

        if (!sector_equals((const char*)e, name_83, 11)) {
            continue;
        }//文件名不匹配返回0

        uint32_t hi = (uint32_t)(*(uint16_t*)(e + 20));//计算文件高16位簇号
        uint32_t lo = (uint32_t)(*(uint16_t*)(e + 26));//计算文件低16位簇号
        *first_cluster = (hi << 16) | lo;//计算文件首个簇号
        *file_size = *(uint32_t*)(e + 28);//计算文件大小
        return 1;//找到根目录项成功返回1
    }

    return 0;
}//找到根目录项成功返回1

int fat32_read_file(const Fat32Fs* fs, const char* name_83, uint8_t* buffer,
                    uint32_t buffer_size, uint32_t* bytes_read) {
    uint32_t cluster = 0;//文件首个簇号
    uint32_t file_size = 0;//文件大小
    uint32_t copied = 0;
    uint8_t cluster_buf[4096];//簇缓冲区

    if (fs->bytes_per_cluster > sizeof(cluster_buf)) {
        return 0;
    }//簇缓冲区大小不足返回0

    if (!find_root_entry(fs, name_83, &cluster, &file_size)) {
        kprintf("FAT32: file not found: ");//文件未找到
        print_fixed_string(name_83, 11);//打印文件名
        kputc('\n');//打印换行符
        return 0;//文件未找到返回0
    }

    if (file_size > buffer_size) {
        kputs("FAT32: file too large for buffer.\n");//文件太大返回0
        return 0;
    }//文件太大返回0

    while (cluster >= 2u && cluster < 0x0FFFFFF8u) {
        if (!read_cluster(fs, cluster, cluster_buf)) {
            return 0;
        }

        uint32_t remain = file_size - copied;
        uint32_t chunk = remain < fs->bytes_per_cluster ? remain : fs->bytes_per_cluster;
        for (uint32_t i = 0; i < chunk; ++i) {
            buffer[copied + i] = cluster_buf[i];
        }
        copied += chunk;
        if (copied >= file_size) {
            break;
        }
        cluster = next_cluster(fs, cluster);
    }//读取文件

    *bytes_read = copied;
    return 1;
}//读取文件成功返回1
