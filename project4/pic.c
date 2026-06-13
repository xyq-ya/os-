#include "common.h"
#include "pic.h"

#define PIC1_CMD 0x20//
#define PIC1_DATA 0x21//主PIC的命令端口
#define PIC2_CMD 0xA0//从PIC的命令端口
#define PIC2_DATA 0xA1//从PIC的命令端口

#define ICW1_INIT 0x11//初始化命令
#define ICW4_8086 0x01//8086模式

void pic_init(void) {
    outb(PIC1_CMD, ICW1_INIT);//初始化主PIC
    outb(PIC2_CMD, ICW1_INIT);//初始化从PIC
    outb(PIC1_DATA, 0x20);//设置主PIC的偏移量
    outb(PIC2_DATA, 0x28);//设置主PIC的偏移量
    outb(PIC1_DATA, 0x04);//设置主PIC的级联
    outb(PIC2_DATA, 0x02);//设置从PIC的偏移量
    outb(PIC1_DATA, ICW4_8086);//设置主PIC的8086模式
    outb(PIC2_DATA, ICW4_8086);//设置从PIC的8086模式

    outb(PIC1_DATA, 0xFE);//设置主PIC的屏蔽
    outb(PIC2_DATA, 0xFF);//设置从PIC的屏蔽
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, 0x20);
    }
    outb(PIC1_CMD, 0x20);
}
