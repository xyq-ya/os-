系统调用全过程说明（中文详解）

一、总体调用链
1. MBR 启动并加载内核：boot/boot.asm
2. 内核入口建立栈并进入 C：kernel/entry.asm -> kernel/main.c
3. 初始化保护模式核心数据结构：kernel/gdt.c + kernel/gdt.asm，kernel/idt.c + kernel/idt.asm
4. 切换到用户态执行用户代码：kernel/cpu.c -> kernel/user.asm
5. 用户态通过 int 0x80 发起系统调用：kernel/user.asm
6. 中断门进入内核桩代码并保存上下文：kernel/interrupts.asm
7. C 层系统调用分发与处理：kernel/syscall.c
8. 返回路径：interrupts.asm 的 iretd 返回到调用方，或由 syscall 2 在内核停机

二、从开机到可系统调用的准备过程
1. 引导阶段
- 文件：boot/boot.asm
- 作用：
  - BIOS 将 MBR 装载到 0x7C00 并开始执行。
  - MBR 读取后续扇区到内存 0x1000。
  - 开启 A20，加载最小 GDT，置位 CR0.PE 进入保护模式。
  - 跳转到 0x1000 执行内核。

2. 内核入口与初始化
- 文件：kernel/entry.asm
- 作用：设置栈顶，调用 kmain。

- 文件：kernel/main.c
- 作用：
  - 打印启动信息。
  - 调用 gdt_init 和 idt_init。
  - 调用 switch_to_user_mode 切换到 ring3 的 user_entry。

3. GDT/TSS 初始化（支持特权级切换）
- 文件：kernel/gdt.c
- 关键点：
  - 建立内核代码段/数据段与用户代码段/数据段。
  - 创建 TSS，并设置 esp0/ss0，供 ring3 -> ring0 时切换内核栈。

- 文件：kernel/gdt.asm
- 关键点：
  - gdt_flush 加载 GDTR 并刷新段寄存器。
  - tss_flush 加载 TR（任务寄存器）。

4. IDT 初始化（挂接 int 0x80）
- 文件：kernel/idt.c
- 关键点：
  - 建立 IDT 项。
  - 设置 0x80 号门为 DPL=3，允许用户态调用。
  - 同时安装 0~31 号异常处理入口，便于故障定位。

- 文件：kernel/idt.asm
- 关键点：
  - idt_load 执行 lidt 装载 IDTR。

三、用户态发起系统调用的全过程
1. 用户程序发起调用
- 文件：kernel/user.asm
- 过程：
  - eax=1，ebx=字符串地址，ecx=字符串长度，执行 int 0x80。
  - 这会触发 CPU 根据 IDT[0x80] 从 ring3 切换到 ring0，并切换到 TSS 指定的内核栈。

2. 内核中断入口保存上下文
- 文件：kernel/interrupts.asm
- 过程：
  - 保存通用寄存器和段寄存器。
  - 将内核数据段选择子装入 ds/es/fs/gs。
  - 把当前栈指针作为参数传给 syscall_handler。

3. C 层分发系统调用
- 文件：kernel/syscall.c
- 过程：
  - 读取 frame->eax 作为系统调用号。
  - eax=1：按 ebx 指针和 ecx 长度逐字符输出。
  - eax=2：进入内核停机路径（打印提示，cli + hlt 死循环）。

4. 返回与结束
- 对于可返回的系统调用：
  - interrupts.asm 恢复上下文并执行 iretd 返回用户态。
- 对于演示中的 syscall 2：
  - 在内核内停机，不再返回用户态，避免后续重启干扰实验观察。

四、相关文件对照表
- 引导与镜像
  - boot/boot.asm
  - linker.ld
  - Makefile

- 内核初始化
  - kernel/entry.asm
  - kernel/main.c
  - kernel/gdt.c
  - kernel/gdt.asm
  - kernel/idt.c
  - kernel/idt.asm

- 系统调用与中断
  - kernel/user.asm
  - kernel/interrupts.asm
  - kernel/syscall.c
  - include/syscall.h

- 输出与调试
  - kernel/print.c
  - include/print.h
  - kernel/isr.asm
  - kernel/faults.c

五、为何会出现重新 Booting kernel
- 当内核触发未处理异常或三重故障时，QEMU 会复位。
- 复位后 BIOS 会重新从硬盘启动，因此再次看到 Booting from Hard Disk 与 Booting kernel。
- 本工程通过补充异常入口、屏蔽未处理 IRQ、以及 syscall 2 的停机路径降低了该现象干扰。

六、可继续扩展的方向
1. 新增 IRQ 重映射与时钟中断处理，支持真正开中断运行。
2. 设计统一 syscall ABI（返回值、错误码、参数布局）。
3. 在用户态加入更多系统调用示例（读键盘、获取时间等）。
4. 增加页表与内存保护，构建更完整的用户态隔离机制。