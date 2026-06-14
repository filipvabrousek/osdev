/* gfx.c -- linear-framebuffer graphics: double buffering, primitives,
 * and a hand-drawn 8x8 bitmap font (uppercase + digits + punctuation). */
#include "kernel.h"

int screen_w, screen_h;

static u8 *fb;                  /* hardware framebuffer (from VBE) */
static u32 fb_pitch;
static u8  fb_bpp;

/* backbuffer: always 32bpp, parked at 3 MiB (well above kernel + stack) */
#define BACKBUFFER ((u32 *)0x300000)
static u32 *back;

/* ---------- freestanding mem helpers ---------- */

void *memcpy(void *dst, const void *src, u32 n)
{
    u8 *d = dst;
    const u8 *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, u32 n)
{
    u8 *d = dst;
    while (n--)
        *d++ = (u8)c;
    return dst;
}

void *memmove(void *dst, const void *src, u32 n)
{
    u8 *d = dst;
    const u8 *s = src;
    if (d < s) {
        while (n--)
            *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--)
            *--d = *--s;
    }
    return dst;
}

int memcmp(const void *a, const void *b, u32 n)
{
    const u8 *x = a, *y = b;
    for (; n--; x++, y++)
        if (*x != *y)
            return *x - *y;
    return 0;
}

/* rep-string variants for the hot paths */
static inline void copy32(void *dst, const void *src, u32 ndwords)
{
    __asm__ volatile("cld; rep movsl"
                     : "+D"(dst), "+S"(src), "+c"(ndwords)
                     :
                     : "memory");
}

static inline void fill32(void *dst, u32 val, u32 ndwords)
{
    __asm__ volatile("cld; rep stosl"
                     : "+D"(dst), "+c"(ndwords)
                     : "a"(val)
                     : "memory");
}

/* ---------- init / present ---------- */

void gfx_init(void)
{
    fb       = (u8 *)BOOT_INFO->framebuffer;
    fb_pitch = BOOT_INFO->pitch;
    fb_bpp   = BOOT_INFO->bpp;
    screen_w = BOOT_INFO->width;
    screen_h = BOOT_INFO->height;
    back     = BACKBUFFER;
    fill32(back, 0, (u32)(screen_w * screen_h));
}

void present(void)
{
    if (fb_bpp == 32) {
        for (int y = 0; y < screen_h; y++)
            copy32(fb + (u32)y * fb_pitch, back + y * screen_w, screen_w);
    } else {                    /* 24bpp fallback */
        for (int y = 0; y < screen_h; y++) {
            u8 *d = fb + (u32)y * fb_pitch;
            u32 *s = back + y * screen_w;
            for (int x = 0; x < screen_w; x++) {
                u32 c = *s++;
                *d++ = c & 0xFF;
                *d++ = (c >> 8) & 0xFF;
                *d++ = (c >> 16) & 0xFF;
            }
        }
    }
}

/* ---------- primitives (clipped, backbuffer) ---------- */

void px(int x, int y, u32 color)
{
    if (x < 0 || y < 0 || x >= screen_w || y >= screen_h)
        return;
    back[y * screen_w + x] = color;
}

static bool clip(int *x, int *y, int *w, int *h)
{
    if (*x < 0) { *w += *x; *x = 0; }
    if (*y < 0) { *h += *y; *y = 0; }
    if (*x + *w > screen_w) *w = screen_w - *x;
    if (*y + *h > screen_h) *h = screen_h - *y;
    return *w > 0 && *h > 0;
}

void fill_rect(int x, int y, int w, int h, u32 color)
{
    if (!clip(&x, &y, &w, &h))
        return;
    for (int row = 0; row < h; row++)
        fill32(back + (y + row) * screen_w + x, color, (u32)w);
}

void shade_rect(int x, int y, int w, int h)
{
    if (!clip(&x, &y, &w, &h))
        return;
    for (int row = 0; row < h; row++) {
        u32 *p = back + (y + row) * screen_w + x;
        for (int col = 0; col < w; col++, p++)
            *p = (*p >> 1) & 0x7F7F7F;
    }
}

void rect_outline(int x, int y, int w, int h, u32 color)
{
    fill_rect(x, y, w, 1, color);
    fill_rect(x, y + h - 1, w, 1, color);
    fill_rect(x, y, 1, h, color);
    fill_rect(x + w - 1, y, 1, h, color);
}

static u32 lerp_color(u32 a, u32 b, int num, int den)
{
    int ar = (a >> 16) & 0xFF, ag = (a >> 8) & 0xFF, ab = a & 0xFF;
    int br = (b >> 16) & 0xFF, bg = (b >> 8) & 0xFF, bb = b & 0xFF;
    int r = ar + (br - ar) * num / den;
    int g = ag + (bg - ag) * num / den;
    int bl = ab + (bb - ab) * num / den;
    return (u32)((r << 16) | (g << 8) | bl);
}

void vgradient(int x, int y, int w, int h, u32 top, u32 bottom)
{
    int den = h > 1 ? h - 1 : 1;
    for (int row = 0; row < h; row++)
        fill_rect(x, y + row, w, 1, lerp_color(top, bottom, row, den));
}

void hgradient(int x, int y, int w, int h, u32 left, u32 right)
{
    int den = w > 1 ? w - 1 : 1;
    for (int col = 0; col < w; col++)
        fill_rect(x + col, y, 1, h, lerp_color(left, right, col, den));
}

void fill_circle(int cx, int cy, int r, u32 color)
{
    for (int dy = -r; dy <= r; dy++)
        for (int dx = -r; dx <= r; dx++)
            if (dx * dx + dy * dy <= r * r)
                px(cx + dx, cy + dy, color);
}

/* ---------- font ----------
 * 8 bytes per glyph, bit 7 = leftmost pixel, glyphs 5px wide, 7px tall.
 * Covers ASCII 32..95; lowercase is folded to uppercase when drawing. */

static const u8 font[64][8] = {
    /* ' ' */ {0, 0, 0, 0, 0, 0, 0, 0},
    /* '!' */ {0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x20, 0},
    /* '"' */ {0x50, 0x50, 0x50, 0x00, 0x00, 0x00, 0x00, 0},
    /* '#' */ {0x50, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x50, 0},
    /* '$' */ {0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0},
    /* '%' */ {0xC8, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x98, 0},
    /* '&' */ {0x60, 0x90, 0xA0, 0x40, 0xA8, 0x90, 0x68, 0},
    /* ''' */ {0x20, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0},
    /* '(' */ {0x10, 0x20, 0x40, 0x40, 0x40, 0x20, 0x10, 0},
    /* ')' */ {0x40, 0x20, 0x10, 0x10, 0x10, 0x20, 0x40, 0},
    /* '*' */ {0x00, 0x50, 0x20, 0xF8, 0x20, 0x50, 0x00, 0},
    /* '+' */ {0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0},
    /* ',' */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20},
    /* '-' */ {0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0},
    /* '.' */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60, 0},
    /* '/' */ {0x08, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0},
    /* '0' */ {0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0},
    /* '1' */ {0x20, 0x60, 0x20, 0x20, 0x20, 0x20, 0x70, 0},
    /* '2' */ {0x70, 0x88, 0x08, 0x30, 0x40, 0x80, 0xF8, 0},
    /* '3' */ {0xF0, 0x08, 0x08, 0x70, 0x08, 0x08, 0xF0, 0},
    /* '4' */ {0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0},
    /* '5' */ {0xF8, 0x80, 0xF0, 0x08, 0x08, 0x88, 0x70, 0},
    /* '6' */ {0x30, 0x40, 0x80, 0xF0, 0x88, 0x88, 0x70, 0},
    /* '7' */ {0xF8, 0x08, 0x10, 0x20, 0x40, 0x40, 0x40, 0},
    /* '8' */ {0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0},
    /* '9' */ {0x70, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60, 0},
    /* ':' */ {0x00, 0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0},
    /* ';' */ {0x00, 0x60, 0x60, 0x00, 0x60, 0x20, 0x40, 0},
    /* '<' */ {0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0},
    /* '=' */ {0x00, 0x00, 0xF8, 0x00, 0xF8, 0x00, 0x00, 0},
    /* '>' */ {0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0},
    /* '?' */ {0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20, 0},
    /* '@' */ {0x70, 0x88, 0xB8, 0xA8, 0xB0, 0x80, 0x78, 0},
    /* 'A' */ {0x70, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0},
    /* 'B' */ {0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0},
    /* 'C' */ {0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0},
    /* 'D' */ {0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0},
    /* 'E' */ {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8, 0},
    /* 'F' */ {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0x80, 0},
    /* 'G' */ {0x70, 0x88, 0x80, 0xB8, 0x88, 0x88, 0x78, 0},
    /* 'H' */ {0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88, 0},
    /* 'I' */ {0x70, 0x20, 0x20, 0x20, 0x20, 0x20, 0x70, 0},
    /* 'J' */ {0x38, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0},
    /* 'K' */ {0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0},
    /* 'L' */ {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8, 0},
    /* 'M' */ {0x88, 0xD8, 0xA8, 0xA8, 0x88, 0x88, 0x88, 0},
    /* 'N' */ {0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x88, 0},
    /* 'O' */ {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0},
    /* 'P' */ {0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80, 0},
    /* 'Q' */ {0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0},
    /* 'R' */ {0xF0, 0x88, 0x88, 0xF0, 0xA0, 0x90, 0x88, 0},
    /* 'S' */ {0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0},
    /* 'T' */ {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0},
    /* 'U' */ {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0},
    /* 'V' */ {0x88, 0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0},
    /* 'W' */ {0x88, 0x88, 0x88, 0xA8, 0xA8, 0xA8, 0x50, 0},
    /* 'X' */ {0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0},
    /* 'Y' */ {0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20, 0},
    /* 'Z' */ {0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0},
    /* '[' */ {0x70, 0x40, 0x40, 0x40, 0x40, 0x40, 0x70, 0},
    /* '\' */ {0x80, 0x80, 0x40, 0x20, 0x10, 0x08, 0x08, 0},
    /* ']' */ {0x70, 0x10, 0x10, 0x10, 0x10, 0x10, 0x70, 0},
    /* '^' */ {0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0},
    /* '_' */ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8},
};

#define GLYPH_ADVANCE 6

static void draw_char(int x, int y, char ch, u32 color, int scale)
{
    if (ch >= 'a' && ch <= 'z')
        ch -= 32;
    if (ch < 32 || ch > 95)
        ch = 32;
    const u8 *glyph = font[ch - 32];
    for (int row = 0; row < 8; row++) {
        u8 bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                if (scale == 1)
                    px(x + col, y + row, color);
                else
                    fill_rect(x + col * scale, y + row * scale,
                              scale, scale, color);
            }
        }
    }
}

void draw_text(int x, int y, const char *s, u32 color, int scale)
{
    for (; *s; s++) {
        draw_char(x, y, *s, color, scale);
        x += GLYPH_ADVANCE * scale;
    }
}

int text_width(const char *s, int scale)
{
    int n = 0;
    while (s[n])
        n++;
    return n * GLYPH_ADVANCE * scale;
}
