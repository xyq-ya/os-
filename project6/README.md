Project 6: FAT32 Virtual Disk + ELF Loader

Build:
  make

Run in QEMU (two drives: OS image + FAT32 virtual disk):
  make run

Features:
- Host tool `tools/mkfat32.py` creates `build/fat32.img` (16 MiB, FAT32)
- Kernel mounts drive 1, prints BPB / layout / root directory entries
- Loads `DEPOSIT.ELF` and `WITHDRAW.ELF` from FAT32 root, runs bank demo (project5)

Disk layout (FAT32 image):
- Boot sector + BPB at LBA 0
- Reserved sectors, 2 FAT copies, data region
- Root directory at cluster 2; ELF files in root

Syscalls: same as project5 (print, P/V, bank op, yield).
