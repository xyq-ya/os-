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
extern void isr_irq0(void);
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
        idt_set_gate((uint8_t)i, (uint32_t)isr_default, 0x08, 0x8E);
    }

    idt_set_gate(0x20, (uint32_t)isr_irq0, 0x08, 0x8E);
    idt_set_gate(0x80, (uint32_t)isr_80, 0x08, 0xEE);
    idt_load((uint32_t)idt, (uint16_t)(sizeof(idt) - 1));
}
