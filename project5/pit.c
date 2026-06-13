//PIT（可编程间隔定时器）
#include "common.h"
#include "pit.h"

#define PIT_CMD  0x43//命令端口用来给 PIT 下命令，告诉它“你要怎么工作
#define PIT_CH0  0x40//通道0端口用来把“分频数”这个数字真正喂给 PIT 的通道0
#define PIT_HZ   1193182//1193182Hz

void pit_init(void) {

    uint32_t divisor = PIT_HZ / 20;//每计数 59659 次，输出一个脉冲

    outb(PIT_CMD, 0x36);
    //0x36=00110110，011模式3方波输出，二进制计数，通道0，BCD计数 00表示选择通道0，11表示先读低字节，再读高字节，最后的0表示二进制计数（不是BCD）
    outb(PIT_CH0, (uint8_t)(divisor & 0xFFu));//写入低8位
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFFu));//写入高8位
}
