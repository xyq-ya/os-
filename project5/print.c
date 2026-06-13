#include "common.h"
#include "print.h"

// VGA 文本缓冲区基址，文本模式下屏幕内容直接映射到这块内存。
static volatile uint16_t* const vga_buffer = (uint16_t*)0xB8000;
// 当前输出光标所在的行。
static uint16_t vga_row = 0;
// 当前输出光标所在的列。
static uint16_t vga_col = 0;
// 默认颜色：黑底灰字。
static uint8_t vga_color = 0x07;

// 在指定坐标写入一个字符及其颜色属性。
static void vga_put_entry_at(char c, uint8_t color, uint16_t x, uint16_t y) {
    const uint16_t index = y * 80 + x;
    vga_buffer[index] = (uint16_t)c | (uint16_t)color << 8;
}

// 处理换行：列归零；满屏时整体上移一行，而不是回到顶部覆盖。
static void vga_newline(void) {
    vga_col = 0;
    if (++vga_row >= 25) {
        for (uint16_t y = 0; y < 24; ++y) {
            for (uint16_t x = 0; x < 80; ++x) {
                vga_buffer[y * 80 + x] = vga_buffer[(y + 1) * 80 + x];
            }
        }
        for (uint16_t x = 0; x < 80; ++x) {
            vga_put_entry_at(' ', vga_color, x, 24);
        }
        vga_row = 24;
    }
}

void kclear(void) {
    for (uint16_t y = 0; y < 25; ++y) {
        for (uint16_t x = 0; x < 80; ++x) {
            vga_put_entry_at(' ', vga_color, x, y);
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void kputc(char c) {
    // 遇到换行字符时只移动光标，不直接写入屏幕字符。
    if (c == '\n') {
        vga_newline();
        return;
    }

    // 在当前位置写入字符，然后推进列位置。
    vga_put_entry_at(c, vga_color, vga_col, vga_row);
    if (++vga_col >= 80) {
        vga_newline();
    }
}

void kputs(const char* s) {
    // 逐字符输出字符串，直到遇到字符串结尾符号。
    while (*s) {
        kputc(*s++);
    }
}

// 简单整数转字符串：用于 %d 和 %x 的格式化输出。
static void itoa(unsigned int value, char* out, int base) {
    char buf[32];
    int i = 0;

    // 0 是特例，直接输出单个字符 '0'。
    if (value == 0) {
        out[0] = '0';
        out[1] = 0;
        return;
    }

    // 先按低位到高位收集，再反转成正常顺序。
    while (value > 0 && i < (int)sizeof(buf) - 1) {
        unsigned int digit = value % base;
        buf[i++] = (digit < 10) ? (char)('0' + digit) : (char)('a' + digit - 10);
        value /= base;
    }

    int j = 0;
    while (i > 0) {
        out[j++] = buf[--i];
    }
    out[j] = 0;
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // 按格式串逐段解析，支持常见的字符串、字符、十进制和十六进制输出。
    for (const char* p = fmt; *p; ++p) {
        if (*p != '%') {
            kputc(*p);
            continue;
        }

        // 处理 % 后面的格式符号。
        ++p;
        if (*p == 's') {
            const char* s = va_arg(args, const char*);
            kputs(s ? s : "(null)");
        } else if (*p == 'c') {
            char c = (char)va_arg(args, int);
            kputc(c);
        } else if (*p == 'd') {
            int value = va_arg(args, int);
            if (value < 0) {
                kputc('-');
                value = -value;
            }
            char buf[32];
            itoa((unsigned int)value, buf, 10);
            kputs(buf);
        } else if (*p == 'x') {
            unsigned int value = va_arg(args, unsigned int);
            char buf[32];
            itoa(value, buf, 16);
            kputs(buf);
        } else if (*p == '%') {
            kputc('%');
        } else {
            kputc('%');
            kputc(*p);
        }
    }

    va_end(args);
}
