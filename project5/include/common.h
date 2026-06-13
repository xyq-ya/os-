#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdarg.h>

// outb: 将一个字节写入指定的 I/O 端口
// 参数：port - I/O 端口号，value - 要写入的 8 位值
// 说明：该函数使用内联汇编执行 outb 指令，常用于向 PIC、PIT、键盘控制器
// 等传统 x86 外设写寄存器或控制端口。使用时要注意同步与中断屏蔽。
static inline void outb(uint16_t port, uint8_t value) {
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

#endif
