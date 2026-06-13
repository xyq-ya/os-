#include "common.h"
#include "pic.h"
//PIC是可编程中断控制器，本程序将8259 中断控制器重映射,将将 IRQ0-IRQ15 映射到 CPU 中断向量 0x20-0x2F
#define PIC1_CMD 0x20//主PIC命令端口
#define PIC1_DATA 0x21//主PIC数据端口
#define PIC2_CMD 0xA0//从PIC命令端口
#define PIC2_DATA 0xA1//从PIC数据端口

#define ICW1_INIT 0x11//初始化命令
#define ICW4_8086 0x01//8086模式
//级联关系理解：链接在IRQ2是因为IBM PC 在 1981 年设计时，定了一个标准
// 1. 网卡 → 从PIC的IRQ10
// 2. 从PIC收到后，通过它的INT引脚 → 主PIC的IRQ2
// 3. 主PIC收到IRQ2的信号
// 4. 主PIC检查自己的ICW3（0x04），发现IRQ2是级联口
// 5. 主PIC不直接报IRQ2，而是问从PIC："你那边谁叫了？"
// 6. 从PIC回答："IRQ10"
// 7. 主PIC把 IRQ10 映射成中断号（比如0x28+2=0x2A），发给CPU
void pic_init(void) {
    outb(PIC1_CMD, ICW1_INIT);
    outb(PIC2_CMD, ICW1_INIT);
    outb(PIC1_DATA, 0x20);//主PIC初始化，中断号为0x20
    outb(PIC2_DATA, 0x28);//从PIC初始化，中断号为0x28
    outb(PIC1_DATA, 0x04);//主PIC级联，级联到从PIC
    outb(PIC2_DATA, 0x02);//从PIC级联，级联到主PIC
    outb(PIC1_DATA, ICW4_8086);//主PIC模式，8086模式
    outb(PIC2_DATA, ICW4_8086);//从PIC模式，8086模式

    outb(PIC1_DATA, 0xFE);//主PIC屏蔽所有中断11111110除了0全部屏蔽
    outb(PIC2_DATA, 0xFF);//从PIC屏蔽所有中断11111111
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);//从PIC发送EOI
    }
    outb(PIC1_CMD, 0x20);//主PIC发送EOI
}//发送EOI信号也就是0×20信号，告诉PIC中断已经处理完毕
