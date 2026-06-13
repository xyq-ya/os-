#include "common.h"
#include "disk.h"

#define ATA_PRIMARY_DATA   0x1F0u//数据端口
#define ATA_PRIMARY_ERROR  0x1F1u//错误端口
#define ATA_PRIMARY_SECCNT 0x1F2u//扇区计数端口
#define ATA_PRIMARY_LBA0   0x1F3u//LBA0端口
#define ATA_PRIMARY_LBA1   0x1F4u//LBA1端口
#define ATA_PRIMARY_LBA2   0x1F5u//LBA2端口
#define ATA_PRIMARY_DRIVE  0x1F6u//驱动器端口
#define ATA_PRIMARY_STATUS 0x1F7u//状态端口

#define ATA_SR_BSY 0x80u//忙标志
#define ATA_SR_DRQ 0x08u//数据请求标志
#define ATA_SR_ERR 0x01u//错误标志

static int ata_wait_not_busy(void) {
    for (int i = 0; i < 200000; ++i) {
        if ((inb(ATA_PRIMARY_STATUS) & ATA_SR_BSY) == 0) {
            return 1;
        }
    }
    return 0;
}//等待硬盘不忙

static int ata_wait_drq(void) {
    for (int i = 0; i < 200000; ++i) {
        uint8_t status = inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_ERR) {
            return 0;
        }
        if (status & ATA_SR_DRQ) {
            return 1;
        }
        if ((status & ATA_SR_BSY) == 0) {
            return 0;
        }
    }
    return 0;
}//等待数据请求

int disk_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, void* buffer) {
    uint8_t* out = (uint8_t*)buffer;//输出缓冲区
    uint8_t drive_sel = (drive == 0) ? 0xE0u : 0xF0u;//选择驱动器

    if (count == 0) {//如果读取扇区数为0，则返回0
        return 0;
    }
    for (uint8_t sector = 0; sector < count; ++sector) {
        uint32_t cur_lba = lba + sector;

        if (!ata_wait_not_busy()) {
            return 0;
        }

        outb(ATA_PRIMARY_SECCNT, 1);
        outb(ATA_PRIMARY_LBA0, (uint8_t)(cur_lba & 0xFFu));
        outb(ATA_PRIMARY_LBA1, (uint8_t)((cur_lba >> 8) & 0xFFu));
        outb(ATA_PRIMARY_LBA2, (uint8_t)((cur_lba >> 16) & 0xFFu));
        outb(ATA_PRIMARY_DRIVE, (uint8_t)(drive_sel | ((cur_lba >> 24) & 0x0Fu)));
        outb(ATA_PRIMARY_STATUS, 0x20u);

        if (!ata_wait_drq()) {
            return 0;
        }

        for (int i = 0; i < 256; ++i) {
            uint16_t word = inw(ATA_PRIMARY_DATA);
            out[sector * 512u + (uint32_t)i * 2u] = (uint8_t)(word & 0xFFu);
            out[sector * 512u + (uint32_t)i * 2u + 1u] = (uint8_t)(word >> 8);
        }
    }

    return 1;
}//读取扇区成功返回1
