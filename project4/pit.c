#include "common.h"
#include "pit.h"

#define PIT_CMD  0x43
#define PIT_CH0  0x40
#define PIT_HZ   1193182

void pit_init(void) {
    // PIT 分频值只有 16 位（最大 65535）
    uint32_t divisor = PIT_HZ / 20;

    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, (uint8_t)(divisor & 0xFFu));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFFu));
}
