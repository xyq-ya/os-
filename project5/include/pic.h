#ifndef PIC_H
#define PIC_H

#define PIC_IRQ_TIMER 0

void pic_init(void);
void pic_send_eoi(uint8_t irq);

#endif
