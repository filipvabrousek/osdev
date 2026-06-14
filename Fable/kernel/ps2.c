/* ps2.c -- PS/2 keyboard and mouse drivers. */
#include "kernel.h"

volatile int mouse_x, mouse_y;
volatile u8  mouse_buttons;

/* ---------- controller helpers ---------- */

static void ps2_wait_write(void)
{
    for (int i = 0; i < 100000; i++)
        if (!(inb(0x64) & 2))
            return;
}

static void ps2_wait_read(void)
{
    for (int i = 0; i < 100000; i++)
        if (inb(0x64) & 1)
            return;
}

static void mouse_cmd(u8 cmd)
{
    ps2_wait_write();
    outb(0x64, 0xD4);           /* next byte goes to the aux device */
    ps2_wait_write();
    outb(0x60, cmd);
    ps2_wait_read();
    (void)inb(0x60);            /* eat the ACK (0xFA) */
}

void ps2_init(void)
{
    while (inb(0x64) & 1)
        (void)inb(0x60);        /* drain stale data */

    ps2_wait_write();
    outb(0x64, 0xA8);           /* enable aux port */

    /* command byte: enable IRQ1 + IRQ12, enable both clocks */
    ps2_wait_write();
    outb(0x64, 0x20);
    ps2_wait_read();
    u8 cb = inb(0x60);
    cb |= 0x03;
    cb &= ~0x30;
    ps2_wait_write();
    outb(0x64, 0x60);
    ps2_wait_write();
    outb(0x60, cb);

    mouse_cmd(0xF6);            /* set defaults */
    mouse_cmd(0xF4);            /* enable data reporting */

    while (inb(0x64) & 1)
        (void)inb(0x60);        /* drain again before interrupts go live */

    mouse_x = screen_w / 2;
    mouse_y = screen_h / 2;
}

/* ---------- keyboard ---------- */

/* scancode set 1 -> ASCII, unshifted */
static const char keymap[128] = {
       0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',   8,
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']','\n',   0,
     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';','\'', '`',   0,'\\',
     'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
};

void kbd_handler(void)
{
    u8 sc = inb(0x60);
    if (!(sc & 0x80)) {         /* make code only */
        char c = sc < 128 ? keymap[sc] : 0;
        if (c)
            wm_on_key(c);
    }
    outb(0x20, 0x20);
}

/* ---------- mouse ---------- */

static u8 cycle;
static u8 pkt[2];

void mouse_handler(void)
{
    u8 data = inb(0x60);

    switch (cycle) {
    case 0:
        if (data & 0x08) {      /* bit 3 always set in byte 0: stay synced */
            pkt[0] = data;
            cycle = 1;
        }
        break;
    case 1:
        pkt[1] = data;
        cycle = 2;
        break;
    case 2: {
        cycle = 0;
        u8 b0 = pkt[0];
        if (b0 & 0xC0)          /* overflow: drop packet */
            break;
        int dx = pkt[1] - ((b0 & 0x10) << 4);   /* 9-bit signed */
        int dy = data   - ((b0 & 0x20) << 3);
        int nx = mouse_x + dx;
        int ny = mouse_y - dy;                  /* PS/2 y+ is up */
        if (nx < 0) nx = 0;
        if (ny < 0) ny = 0;
        if (nx > screen_w - 1) nx = screen_w - 1;
        if (ny > screen_h - 1) ny = screen_h - 1;
        mouse_x = nx;
        mouse_y = ny;
        mouse_buttons = b0 & 0x07;
        break;
    }
    }

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
