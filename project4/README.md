Project 4: Timer IRQ + Round-Robin Scheduler

Build:
  make

Run in QEMU:
  make run

Notes:
- Based on project3: ELF loader, ring3 user programs, int 0x80.
- Loads two standalone ELF programs: `user_program.asm` and `user_program2.asm`.
- PIC + PIT provide IRQ0 timer interrupts (vector 0x20).
- Scheduler preempts between the two user programs in round-robin order.
