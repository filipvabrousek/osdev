/* main.c -- kernel entry: bring up subsystems, then run the frame loop. */
#include "kernel.h"

void kmain(void);

void kmain(void)
{
    gfx_init();
    interrupts_init();
    ps2_init();
    wm_init();

    __asm__ volatile("sti");

    u32 last = (u32)-1;
    for (;;) {
        __asm__ volatile("hlt");        /* sleep until next interrupt */
        if (ticks != last) {            /* redraw once per PIT tick */
            last = ticks;
            wm_frame();
        }
    }
}
