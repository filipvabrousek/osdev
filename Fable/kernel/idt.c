/* idt.c -- IDT setup, PIC remap, PIT timer. */
#include "kernel.h"

volatile u32 ticks;

struct __attribute__((packed)) idt_entry {
    u16 offset_lo;
    u16 selector;
    u8  zero;
    u8  flags;
    u16 offset_hi;
};

struct __attribute__((packed)) idt_ptr {
    u16 limit;
    u32 base;
};

static struct idt_entry idt[256];

extern void irq_timer_stub(void);
extern void irq_kbd_stub(void);
extern void irq_mouse_stub(void);
extern void isr_unhandled_stub(void);
extern void isr_exception_stub(void);

static void set_gate(int n, void (*handler)(void))
{
    u32 addr = (u32)handler;
    idt[n].offset_lo = addr & 0xFFFF;
    idt[n].selector  = 0x08;
    idt[n].zero      = 0;
    idt[n].flags     = 0x8E;        /* present, ring 0, 32-bit int gate */
    idt[n].offset_hi = addr >> 16;
}

#define PIT_HZ      100
#define PIT_DIVISOR (1193182 / PIT_HZ)

void interrupts_init(void)
{
    for (int i = 0; i < 256; i++)
        set_gate(i, isr_unhandled_stub);
    for (int i = 0; i < 32; i++)
        set_gate(i, isr_exception_stub);

    set_gate(0x20, irq_timer_stub);     /* IRQ0  */
    set_gate(0x21, irq_kbd_stub);       /* IRQ1  */
    set_gate(0x2C, irq_mouse_stub);     /* IRQ12 */

    /* remap PICs: master -> 0x20..0x27, slave -> 0x28..0x2F */
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xF8);   /* unmask IRQ0 (timer), IRQ1 (kbd), IRQ2 (cascade) */
    outb(0xA1, 0xEF);   /* unmask IRQ12 (mouse) */

    struct idt_ptr ptr = { sizeof(idt) - 1, (u32)idt };
    __asm__ volatile("lidt %0" : : "m"(ptr));

    /* PIT channel 0, mode 3 (square wave), ~100 Hz */
    outb(0x43, 0x36);
    outb(0x40, PIT_DIVISOR & 0xFF);
    outb(0x40, PIT_DIVISOR >> 8);
}

void timer_handler(void)
{
    ticks++;
    outb(0x20, 0x20);
}
