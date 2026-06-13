#include "common.h"
#include "idt.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

static struct idt_entry idt[256];

extern void isr_default(void);
extern void isr_80(void);
extern void idt_load(uint32_t base, uint16_t limit);

static void idt_set_gate(uint8_t vector, uint32_t handler,
                         uint16_t selector, uint8_t type_attr) {
    idt[vector].offset_low = (uint16_t)(handler & 0xFFFFu);
    idt[vector].offset_high = (uint16_t)((handler >> 16) & 0xFFFFu);
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
}

void idt_init(void) {
    for (uint16_t i = 0; i < 256; ++i) {
        idt_set_gate((uint8_t)i, (uint32_t)isr_default, 0x08, 0x8E);//内核态调用;0×08表示的是内核代码段选择子
    }

    idt_set_gate(0x80, (uint32_t)isr_80, 0x08, 0xEE);//单独设置0x80号门为DPL=3，允许用户态调用，决定的是0×EE也就是1110，1110的最后两位是10，表示是32位门，DPL=3
    //注意0×EE=11101110，第七位是1，第六位和第五位共同组成11也就是DPL=3，后面0×0E表示的是中断门，中断门的意思是硬件中断，F就是软件中断也叫陷阱门
    idt_load((uint32_t)idt, (uint16_t)(sizeof(idt) - 1));
}
