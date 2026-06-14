/* kernel.h -- shared types and interfaces. Fully freestanding: no libc,
 * no compiler-provided headers. */
#pragma once

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed char    i8;
typedef short          i16;
typedef int            i32;
typedef u32            uptr;
typedef int            bool;

#define true  1
#define false 0
#define NULL  ((void *)0)

/* ---- port I/O ---- */
static inline void outb(u16 port, u8 val)
{
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u8 inb(u16 port)
{
    u8 r;
    __asm__ volatile("inb %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

/* ---- boot info written by stage2 at 0x9000 ---- */
typedef struct __attribute__((packed)) {
    u32 framebuffer;
    u16 pitch;
    u16 width;
    u16 height;
    u8  bpp;
} BootInfo;

#define BOOT_INFO ((volatile BootInfo *)0x9000)

/* ---- freestanding mem helpers (gcc may emit calls to these) ---- */
void *memcpy(void *dst, const void *src, u32 n);
void *memset(void *dst, int c, u32 n);
void *memmove(void *dst, const void *src, u32 n);
int   memcmp(const void *a, const void *b, u32 n);

/* ---- interrupts / timer (idt.c) ---- */
extern volatile u32 ticks;          /* PIT ticks, ~100 Hz */
void interrupts_init(void);
void timer_handler(void);

/* ---- PS/2 keyboard + mouse (ps2.c) ---- */
extern volatile int mouse_x, mouse_y;
extern volatile u8  mouse_buttons;  /* bit0 left, bit1 right */
void ps2_init(void);
void kbd_handler(void);
void mouse_handler(void);

/* ---- graphics (gfx.c) ---- */
extern int screen_w, screen_h;
void gfx_init(void);
void px(int x, int y, u32 color);
void fill_rect(int x, int y, int w, int h, u32 color);
void shade_rect(int x, int y, int w, int h);          /* 50% darken */
void rect_outline(int x, int y, int w, int h, u32 color);
void vgradient(int x, int y, int w, int h, u32 top, u32 bottom);
void hgradient(int x, int y, int w, int h, u32 left, u32 right);
void fill_circle(int cx, int cy, int r, u32 color);
void draw_text(int x, int y, const char *s, u32 color, int scale);
int  text_width(const char *s, int scale);
void present(void);

/* ---- window manager (wm.c) ---- */
void wm_init(void);
void wm_frame(void);
void wm_on_key(char c);
