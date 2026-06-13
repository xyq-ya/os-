Project 5: Shared Memory + P/V Synchronization

Build:
  make

Run in QEMU:
  make run

Features (based on project4):
- Shared memory bank account at `0x300000`
- Binary semaphore mutex via syscall P/V
- Two user programs:
  - `bank_deposit.asm`: +100 per critical section
  - `bank_withdraw.asm`: -50 per critical section

Syscalls:
- 1: print string (`ebx`=addr, `ecx`=len)
- 2: P (wait mutex)
- 3: V (signal mutex)
- 4: print bank line (`ebx`=balance, `ecx`=0 deposit / 1 withdraw)
- 5: bank operation (`ebx`=0 deposit +100 / 1 withdraw -50), P/V + print in kernel
- 6: yield CPU to the other process
