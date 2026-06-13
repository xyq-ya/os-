#!/usr/bin/env python3
"""Create a raw FAT32 virtual disk and copy ELF files into the root directory."""
#扇区 0      : 引导扇区 (BPB)
#扇区 1 ~ 31 : 保留区（FSINFO等，共31个扇区）
#扇区 32 ~ 63: FAT表1（32个扇区）
#扇区 64 ~ 95: FAT表2（32个扇区）
#扇区 96 ~   : 数据区
import struct
import sys
from pathlib import Path

SECTOR_SIZE = 512
TOTAL_SECTORS = 32768          # 16 MiB
RESERVED_SECTORS = 32
NUM_FATS = 2
SECTORS_PER_CLUSTER = 8
FAT_SIZE_SECTORS = 32
ROOT_CLUSTER = 2
MEDIA_DESCRIPTOR = 0xF8
END_OF_CHAIN = 0x0FFFFFF8

DATA_BEGIN_LBA = RESERVED_SECTORS + NUM_FATS * FAT_SIZE_SECTORS

#打包BPB，BIOS Parameter Block，保存FAT32文件系统的头部信息
def pack_bpb():
    boot = bytearray(SECTOR_SIZE)
    boot[0:3] = b"\xEB\x58\x90"
    boot[3:11] = b"OSLAB6  "
    struct.pack_into("<H", boot, 11, SECTOR_SIZE)
    boot[13] = SECTORS_PER_CLUSTER
    struct.pack_into("<H", boot, 14, RESERVED_SECTORS)
    boot[16] = NUM_FATS
    struct.pack_into("<H", boot, 17, 0)
    struct.pack_into("<H", boot, 19, 0)
    boot[21] = MEDIA_DESCRIPTOR
    struct.pack_into("<H", boot, 22, 0)
    struct.pack_into("<H", boot, 24, 63)
    struct.pack_into("<H", boot, 26, 255)
    struct.pack_into("<I", boot, 28, 0)
    struct.pack_into("<I", boot, 32, TOTAL_SECTORS)
    struct.pack_into("<I", boot, 36, FAT_SIZE_SECTORS)
    struct.pack_into("<H", boot, 40, 0)
    struct.pack_into("<H", boot, 42, 0)
    struct.pack_into("<I", boot, 44, ROOT_CLUSTER)
    struct.pack_into("<H", boot, 52, 1)
    struct.pack_into("<H", boot, 54, 6)
    boot[64] = 0x80
    boot[66] = 0x29
    struct.pack_into("<I", boot, 67, 0x12345678)
    boot[71:82] = b"OSLABVOL   "
    boot[82:90] = b"FAT32   "
    boot[510] = 0x55
    boot[511] = 0xAA
    return boot


def cluster_to_lba(cluster: int) -> int:
    return DATA_BEGIN_LBA + (cluster - 2) * SECTORS_PER_CLUSTER
#将簇号转换为LBA

def write_cluster(img: bytearray, cluster: int, data: bytes):
    lba = cluster_to_lba(cluster)
    offset = lba * SECTOR_SIZE#计算偏移量
    padded = data + b"\x00" * (SECTORS_PER_CLUSTER * SECTOR_SIZE - len(data))#填充数据
    img[offset:offset + SECTORS_PER_CLUSTER * SECTOR_SIZE] = padded[:SECTORS_PER_CLUSTER * SECTOR_SIZE]#写入数据
#解释一下就是，首先，我们知道Logical Block Address（LBA）是逻辑块地址，是文件系统中的逻辑地址，而Cluster是文件系统中的物理地址，所以需要将Cluster转换为LBA。
#然后，我们得到了逻辑地址在数据块的偏移量，就先填充满4KB的数据满一个簇，然后直接写入文件逻辑地址的对应位置，：前面的是起始点，：后面的是结束点。

def set_fat_entry(fat: bytearray, cluster: int, value: int):
    offset = cluster * 4#计算偏移量，每个FAT32表项占四个字节
    existing = struct.unpack_from("<I", fat, offset)[0]#读取FAT表项
    struct.pack_into("<I", fat, offset, (existing & 0xF0000000) | (value & 0x0FFFFFFF))#写入FAT表项，高4位保留只写低28位
#这个和上面的近似，上面的是写内容下面的是写表项


def make_dir_entry(name_83: bytes, cluster: int, size: int) -> bytes:
    entry = bytearray(32)
    entry[0:11] = name_83.ljust(11, b" ")[:11]#填充文件名8字节文件名3字节扩展名
    entry[11] = 0x20#填充文件名结束符
    struct.pack_into("<H", entry, 20, (cluster >> 16) & 0xFFFF)#写入高16位簇号
    struct.pack_into("<H", entry, 26, cluster & 0xFFFF)#写入低16位簇号
    struct.pack_into("<I", entry, 28, size)#写入文件大小
    return bytes(entry)#返回文件目录项
#解释一下就是，首先，我们知道文件名是8.3格式，所以需要填充文件名，然后填充文件名结束符，然后写入高16位簇号，然后写入低16位簇号，然后写入文件大小。


def to_83(stem: str, ext: str) -> bytes:
    return stem.upper().ljust(8)[:8].encode("ascii") + ext.upper().ljust(3)[:3].encode("ascii")
#把普通文件名转换成 FAT32 文件系统要求的 8.3 格式

def write_file_data(img: bytearray, fat: bytearray, start_cluster: int, data: bytes) -> int:
    cluster_size = SECTORS_PER_CLUSTER * SECTOR_SIZE
    cluster = start_cluster
    first_cluster = start_cluster
    offset = 0

    while offset < len(data):
        chunk = data[offset:offset + cluster_size]
        write_cluster(img, cluster, chunk)
        offset += len(chunk)

        if offset < len(data):
            next_cluster = cluster + 1
            set_fat_entry(fat, cluster, next_cluster)
            cluster = next_cluster
        else:
            set_fat_entry(fat, cluster, END_OF_CHAIN)

    return first_cluster


def main():
    build_dir = Path(__file__).resolve().parent.parent / "build"
    build_dir.mkdir(parents=True, exist_ok=True)

    files = [
        (to_83("DEPOSIT", "ELF"), build_dir / "bank_deposit.elf"),
        (to_83("WITHDRAW", "ELF"), build_dir / "bank_withdraw.elf"),
    ]
#遍历文件列表，如果文件不存在，则打印错误信息并返回1
    for _, path in files:
        if not path.exists():
            print(f"error: missing {path}, build ELF files first", file=sys.stderr)
            return 1
#创建一个字节数组，大小为总扇区数乘以扇区大小
    img = bytearray(TOTAL_SECTORS * SECTOR_SIZE)
    img[0:SECTOR_SIZE] = pack_bpb()
#打包BPB，保存FAT32文件系统的头部信息
    fat = bytearray(FAT_SIZE_SECTORS * SECTOR_SIZE)
    set_fat_entry(fat, 0, 0x0FFFFFF8)
    set_fat_entry(fat, 1, END_OF_CHAIN)
    set_fat_entry(fat, ROOT_CLUSTER, END_OF_CHAIN)
#创建一个字节数组，大小为每个簇的大小
    root_entries = bytearray(SECTORS_PER_CLUSTER * SECTOR_SIZE)
    next_cluster = ROOT_CLUSTER + 1
    dir_offset = 0

    for name_83, path in files:
        data = path.read_bytes()
        clusters_needed = (len(data) + SECTORS_PER_CLUSTER * SECTOR_SIZE - 1) // (
            SECTORS_PER_CLUSTER * SECTOR_SIZE
        )
        cluster = next_cluster
        next_cluster += clusters_needed
        first_cluster = write_file_data(img, fat, cluster, data)
        entry = make_dir_entry(name_83, first_cluster, len(data))
        root_entries[dir_offset:dir_offset + 32] = entry
        dir_offset += 32
#写入根目录项,这里超级有说法，第一我们先创建了一个root_entries字节数组，大小为一个簇当作目录项，然后我们遍历文件列表
#把每个文件写入以后返回文件首个簇号，并且暂时记录在这个数组里，到文件全部写入这个数组就能当做目录写入硬盘了，ROOT_CLUSTER + 1的伏笔
    write_cluster(img, ROOT_CLUSTER, bytes(root_entries))
#写入FAT表
    for fat_idx in range(NUM_FATS):
        start = (RESERVED_SECTORS + fat_idx * FAT_SIZE_SECTORS) * SECTOR_SIZE
        img[start:start + len(fat)] = fat
#收尾做备份工作，把FAT表写入硬盘，然后输出文件名和文件大小
    out = build_dir / "fat32.img"
    out.write_bytes(img)
    print(f"Created {out} ({len(img)} bytes)")
    print("FAT32 layout:")
    print(f"  reserved sectors: {RESERVED_SECTORS}")
    print(f"  FAT size sectors: {FAT_SIZE_SECTORS}")
    print(f"  data begin LBA:   {DATA_BEGIN_LBA}")
    print(f"  root cluster:     {ROOT_CLUSTER}")
    for name_83, path in files:
        print(f"  {name_83.decode('ascii')}: {path.name} ({path.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
