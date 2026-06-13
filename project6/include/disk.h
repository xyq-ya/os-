#ifndef DISK_H
#define DISK_H

#include "common.h"

#define DISK_BOOT_DRIVE   0
#define DISK_FAT32_DRIVE  1

int disk_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, void* buffer);

#endif
