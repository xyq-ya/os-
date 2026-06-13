%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 32
%endif
; 如果没有显式给出 KERNEL_SECTORS，则默认从磁盘读取 32 个扇区作为内核
; 这样可以由构建系统/链接脚本控制要加载的内核大小（单位：扇区）
BITS 16
; 指定后续汇编为 16 位实模式指令（BIOS 引导时 CPU 处于实模式）
; 说明：实模式可以直接调用 BIOS 中断（如 int 0x10, int 0x13 等），
; 而保护模式才支持分页/任务特权等高级功能。本引导程序在实模式下
; 使用 BIOS 服务加载内核，然后启用 A20 并切换到保护模式执行内核。
ORG 0x7C00
;代码基址，告诉内核硬盘会把代码加载到0*7C00
start:
    ; ------------------------------
    ; 引导主流程（高层说明）：
    ; 1) 初始化段寄存器与栈，保证 BIOS 中断和字符串输出可用；
    ; 2) 从引导驱动器读取后续扇区（内核镜像）到物理内存 0x1000；
    ; 3) 启用 A20，加载 GDT，并设置 CR0.PE=1 进入保护模式；
    ; 4) 在保护模式下设置段寄存器并跳转到内核入口（0x1000）。
    ; 下面的指令块按上述步骤逐条实现，注意不要随意改动寄存器用法。
    cli
    xor ax, ax
    ;异或快速清零
    mov ds, ax
    ;数据段寄存器 DS
    mov es, ax
    ;附加段寄存器 ES
    mov ss, ax
    ;栈寄存器 SS
    mov sp, 0x7C00
    ;栈指针 SP指向0*7C00（向下递减？好像x86实模式是固定的向下递减） 
    ;x86 的栈是 满递减栈：不是指向下一个空栈而是指向最新压入栈的数据，每次压数据栈指向S=（S-2）
    sti

    mov [boot_drive], dl
    ;保存启动盘号（使被覆盖的时候还能被找回来）dl为启动驱动器号
    mov si, boot_msg
    ;打印消息的地址(我的理解是把消息存放的位置放进si寄存器，之后就好找到要打印的字符)
    call print_string

    mov bx, 0x1000
    ; 缓冲区偏移
    mov dh, 0
    ; 磁头号 = 0
    mov ch, 0
    ; 柱面号 = 0
    mov cl, 2
    ; 起始扇区号 = 2（扇区1为引导扇区，扇区2存放内核）
    mov al, KERNEL_SECTORS ; 读取扇区数 = 32
    mov dl, [boot_drive] ; 驱动器号 = 之前保存的，上面的过程中可能改变了所以重新加载回来
    mov ah, 0x02 ;写指令告诉操作系统我要读硬盘（用上面的配置）
    int 0x13 ;(int不是c里的int 而是interrupt，启用中断去硬盘读数据)
    jc disk_error ;如果读取失败跳转到错误处理

    ; 通过端口 0x92 的快速方式开启 A20 地址线，确保能够访问 1MB 以上内存。
    in al, 0x92 ; 读取端口0x92的值，这个能快速开启A20地址线，原来的CPU不能访问1M以上内存
    or al, 0x02 ;设置bit1设置为1（or为or运算）
    out 0x92, al ;写回去

    ; 加载全局描述符表并进入保护模式。
    lgdt [gdt_desc] ;告诉 CPU “全局描述符表 (GDT) 放在内存的哪个位置、有多大(保护模式专属用索引加偏移查找地址)
    cli ;必须得先中断，实模式用的是中断向量表，保护模式用的是中断描述符表，不关中断容易导致实模式的中断没执行完
    mov eax, cr0 ; 读取控制寄存器CR0
    or eax, 1   ; 设置最低位（PE位，保护模式使能位）
    mov cr0, eax ;写回去
    jmp 0x08:pmode_entry ;从实模式的老索引（直接地址）切换到保护模式的新索引（查 GDT），去执行 32 位代码。

print_string:
    lodsb   ; 从DS:SI读一个字节到AL，SI++，SI是偏移量
    or al, al ; 检查AL是否为0
    jz .done ; 如果是0（字符串结束），跳转到.done
    mov ah, 0x0E ; BIOS Teletype输出功能
    int 0x10    ; 调用视频中断，显示字符
    jmp print_string ; 循环处理下一个字符
.done:
    ret ; 返回调用者

disk_error:
    mov si, disk_msg ; 加载错误消息地址
    call print_string ; 打印错误
.hang:
    hlt ; Halt CPU，暂停执行
    jmp .hang ; 无限循环

[BITS 32] ;现在是32位了
pmode_entry:
    ; 保护模式入口（进入保护模式后执行此处代码）：
    ; 说明：此时已在保护模式（CR0.PE=1），但尚需设置段寄存器为内核段
    ; - 0x10 是内核数据段选择子（GDT 索引 2 * 8 = 0x10）
    ; - DS/ES/SS/FS/GS 均设置为内核数据段，保证后续内核访问线性地址时段描述正确
    ; - ESP 设置为临时内核栈顶（此处设为 0x90000），内核启动后可自行调整
    ; - 最后跳转到物理地址 0x1000，假定内核已被加载到该位置
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax ; FS段？
    mov gs, ax ; GS段？这两个是保护模式独属的，扩展用
    mov esp, 0x90000 ;新栈顶设在 0x90000（约576KB处），向下增长。
    jmp 0x1000 ;; 跳转到物理地址0x1000处的内核

gdt_start:
    ; 平坦 4GB 内存模型所需的描述符：空描述符、代码段描述符、数据段描述符。
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF

gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1 ;最大索引偏移量
    dd gdt_start ; 基址 = GDT 起始物理地址

boot_drive db 0 ; 1字节变量，保存启动盘号db是Define Byte，表示定义字节数据，例如这里就表示分配一字节数据为0。
boot_msg db "Booting kernel...", 0 ; 以0结尾的字符串
disk_msg db "Disk read error!", 0 ; 错误消息

times 510-($-$$) db 0 ;$ = 当前位置地址,$$ = 当前段起始地址（0x7C00）,$-$$ = 已汇编的字节数,510 - ($-$$) = 还需要填充多少字节到510字节
dw 0xAA55 ;在最后 2 字节写入签名 0x55 0xAA，告诉 BIOS“我是可引导的,这是特定签名，所以必须上一步凑够510字节来启动
