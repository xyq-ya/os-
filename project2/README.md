Project 2: MBR Bootloader + OS Kernel (Naive, Minimal)

Build:
  make

Run in QEMU:
  make run

Docs:
  docs/overview.md

Notes:
- Bootloader loads 32 sectors from disk into 0x1000 and switches to protected mode.
- No memory management and no process scheduler.
- Keep minimal kernel features: VGA `kprintf`, CPU context switching, and syscall demo.
- `user_program.asm` is an OS kernel 外的程序示例，演示如何调用 syscall。
