#ifndef PRINT_H
#define PRINT_H

#include "common.h"

// 屏幕输出相关的简单接口（基于 VGA 文本模式 80x25）
// kputc: 输出单字符到当前光标位置，处理换行 \n 的逻辑
void kputc(char c);
// kputs: 输出以 '\0' 结尾的字符串，内部通过 kputc 循环调用
void kputs(const char* s);
// kprintf: 简易格式化输出，支持以下格式说明符：
// - %s: 字符串
// - %c: 字符
// - %d: 带符号十进制整数
// - %x: 无符号十六进制整数
// - %%: 输出一个百分号
// 该实现为轻量实现，不支持宽度/对齐/精度等高级特性
void kclear(void);
void kprintf(const char* fmt, ...);

#endif
