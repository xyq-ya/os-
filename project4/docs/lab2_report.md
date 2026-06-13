项目2实验报告（MBR + Naive Kernel）

一、实验基本信息
- 实验名称：Bootloader（MBR）+ OS Kernel（Naive Version）
- 实验日期：2026-05-25
- 实验目录：/home/xcy/oslab
- 代码版本标识：no-git-commit（当前目录未检测到可用提交号）

二、实验目标
- 修改并实现可由 QEMU 正常加载的 MBR 引导程序。
- 实现内核基本输出能力（kputc、kputs、kprintf）。
- 封装中断框架，支持系统调用入口（int 0x80）。
- 实现内核态到用户态切换（ring0 -> ring3）。
- 在内核外部用户程序中演示系统调用。

三、实验环境与工具版本
- GCC：gcc (Ubuntu 15.2.0-16ubuntu1) 15.2.0
- NASM：NASM version 3.01
- QEMU：QEMU emulator version 10.2.1 (Debian 1:10.2.1+ds-1ubuntu3)
- GNU Make：GNU Make 4.4.1

四、关键文件与版本（SHA256）
- boot/boot.asm: 870bd08ceccddacfeeb5687d96eb9d23908d05e4506cedc32e9c9c6be29636bf
- linker.ld: a1fbba812709347bb83a993870f27ee5576c662f613e0d433c839429b1e03ac7
- Makefile: 1cec5f08b5fe856609a12f8ad66079db42220d115401ae89a18265f3814d68d5
- README.md: 04242ef63f0d978ac2e285af04aa1d7cae55a428e350c1acf09c8e0eddb78125
- include/common.h: cb4a636d488cb62db6ae307f95c1ef23be9a33988fb6a0eab345f420a7d381a6
- include/cpu.h: ef854b5cfd86e9bb57ef96227ee4af0516253579e4c4a6adf2b1f93e69fce001
- include/gdt.h: 21e36b87fd0e59765b8d3ce989c066c002ec29adb0b8b58f7af9db728ffe86df
- include/idt.h: b95928125b12181b61e56102e29b76577e22065b5152d5021bbe622bc7f11408
- include/print.h: a6b44deefc0b8059290383adc355b8e4a0852c3f5e971f21032e2e86ee51b1e9
- include/syscall.h: f27982cd2a4d4cceabc6ed17b46afbd25c2c1162480937cf13c640a9d16d750e
- kernel/entry.asm: 0e74b62fd92a4559d844704beaa29eb6b9e7e5f3469b66c66eb6ba20047a6e57
- kernel/main.c: eb0c8d2a1e32629497e45053138e84fa0feb12598b5d381d143b549a91a45d20
- kernel/print.c: dcd6ae14723609b82c354e05f989548252a6a1c450aec41b80f43fcedbaba70c
- kernel/gdt.c: 3abd69305f5646f9f60a0e190fab3410994c50dfed9e60cd916baa4c52146d95
- kernel/gdt.asm: 465e75a126597c4ea16c46202662b4767afbac974420e3a5f7e25467f135416f
- kernel/idt.c: 1a476553a5b57c3d3b0bf88619e072f1163f312897b71fa2da4f5d70e3bfc96f
- kernel/idt.asm: b114f03e948025f3b24af2f31a7bc855650835c438a4ae11a39353b6a1dcc1be
- kernel/isr.asm: ed4c4e07dad28b6c8c63ed760f5cdf5a8882bd398fa443ab125018b3c86549ef
- kernel/interrupts.asm: 2dc089e61373d442c33097e4701a8ffc6c6af7387b51b77b67c421230c2a103c
- kernel/syscall.c: 831a1ec265e49e17603ac1b063bf75cee795f1a94f76207f44e432db39a419b8
- kernel/cpu.c: 1b6d0db1a05ea1cc133ca56cc8ff56c655d53fc2d4ca4a3313bf47089390eb20
- kernel/user.asm: fa90b7ce40a8e2134c3ba122a12957a8797ab4b2ca5e7aa8a0a173221491763b
- kernel/faults.c: fc0f466e875e2e63f3363c18d8f2009dde99b18129f968ab929d370196581d09
- docs/overview.md: 679a905e561993b057d86344eaef9bb0601ecf2611df2521969b36be1d1fb5a4

五、实验步骤
1. 清理旧构建产物。
   执行：make clean

2. 编译并链接内核与引导代码，生成镜像。
   执行：make

3. 运行系统镜像。
   执行：make run

4. 调试模式运行（出现异常或重启时使用）。
   执行：make run-debug

六、构建产物说明
- build/boot.bin：MBR 引导扇区二进制
- build/kernel.elf：内核 ELF 文件
- build/kernel.bin：内核平坦二进制
- build/os-image.bin：最终磁盘镜像
- build/obj/*.o：中间目标文件

七、运行现象与结果
- 成功输出：
  - Kernel start
  - Switching to user mode...
  - Hello from user mode via syscall!
- 用户态后通过 syscall 2 进入内核停机路径（cli + hlt），用于稳定结束演示。

八、问题与排查记录摘要
- 曾出现重复启动到 iPXE/Booting from Hard Disk 的现象，本质为虚拟机重启后重新引导。
- 已通过补充异常处理、调整中断初始化、并在演示末尾内核停机来提升稳定性。

九、结论
- 本实验完成了从 MBR 引导到内核执行、从内核态到用户态切换、用户态触发系统调用再返回内核处理的最小闭环。
- 工程具备继续扩展 IRQ、调度、文件系统等功能的基础。