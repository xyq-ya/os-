Project 3: Memory Partition + ELF Loader (based on Project 2)

Build:
  make

Run in QEMU:
  make run

Notes:
- Kernel infrastructure comes from project2: GDT/TSS, IDT, int 0x80, ring3 switch.
- `user_program.asm` is built as a standalone ELF and embedded into the kernel image.
- Kernel loads the ELF into a fixed user partition (0x200000) and starts it in ring 3.
- No file system and no paging.
