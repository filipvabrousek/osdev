/* wm.c -- window manager: desktop, taskbar, draggable windows with
 * z-ordering, focus, close buttons, and a few demo window contents. */
#include "kernel.h"

#define MAX_WIN  12
#define TITLE_H  20
#define TASK_H   28
#define BORDER   2

typedef enum { W_WELCOME, W_NOTEPAD, W_PALETTE, W_BOUNCE } WinKind;

typedef struct {
    int     x, y, w, h;
    char    title[24];
    WinKind kind;
    bool    alive;
    /* notepad */
    char    text[160];
    int     tlen;
    /* bounce */
    int     bx, by, bvx, bvy;
} Window;

static Window wins[MAX_WIN];
static int    zorder[MAX_WIN];      /* indices into wins, bottom..top */
static int    nz;
static int    focused = -1;
static int    drag = -1;
static int    drag_ox, drag_oy;
static u8     prev_buttons;
static int    spawn_count;

/* ---------- window list management ---------- */

static void copy_str(char *dst, const char *src, int cap)
{
    int i = 0;
    for (; src[i] && i < cap - 1; i++)
        dst[i] = src[i];
    dst[i] = 0;
}

static void raise_win(int i)
{
    int j = 0;
    while (j < nz && zorder[j] != i)
        j++;
    if (j == nz)
        return;
    for (; j < nz - 1; j++)
        zorder[j] = zorder[j + 1];
    zorder[nz - 1] = i;
}

static int spawn(WinKind kind, const char *title, int x, int y, int w, int h)
{
    int i = 0;
    while (i < MAX_WIN && wins[i].alive)
        i++;
    if (i == MAX_WIN)
        return -1;

    Window *win = &wins[i];
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;
    win->kind = kind;
    win->alive = true;
    copy_str(win->title, title, sizeof(win->title));
    win->tlen = 0;
    if (kind == W_NOTEPAD) {
        copy_str(win->text, "TYPE SOMETHING: ", sizeof(win->text));
        win->tlen = 16;
    }
    win->bx = 30;
    win->by = 30;
    win->bvx = 2;
    win->bvy = 3;

    zorder[nz++] = i;
    focused = i;
    spawn_count++;
    return i;
}

static void close_win(int i)
{
    wins[i].alive = false;
    int j = 0;
    while (j < nz && zorder[j] != i)
        j++;
    for (; j < nz - 1; j++)
        zorder[j] = zorder[j + 1];
    if (j < nz)
        nz--;
    if (drag == i)
        drag = -1;
    if (focused == i)
        focused = nz ? zorder[nz - 1] : -1;
}

static int hit_test(int x, int y)
{
    for (int j = nz - 1; j >= 0; j--) {
        Window *w = &wins[zorder[j]];
        if (x >= w->x && x < w->x + w->w && y >= w->y && y < w->y + w->h)
            return zorder[j];
    }
    return -1;
}

/* ---------- keyboard input ---------- */

void wm_on_key(char c)
{
    if (focused < 0 || !wins[focused].alive)
        return;
    Window *w = &wins[focused];
    if (w->kind != W_NOTEPAD)
        return;
    if (c == 8) {
        if (w->tlen > 0)
            w->tlen--;
    } else if (w->tlen < (int)sizeof(w->text) - 1) {
        w->text[w->tlen++] = c;
    }
    w->text[w->tlen] = 0;
}

/* ---------- drawing ---------- */

static void draw_desktop(void)
{
    vgradient(0, 0, screen_w, screen_h - TASK_H, 0x10304F, 0x5E94C4);
    draw_text(16, 14, "H7 OS", 0xE8F0F8, 3);
    draw_text(16, 42, "BAREBONES KERNEL - VESA 640X480", 0x9FBEDB, 1);
}

static void draw_close_box(Window *w, bool active)
{
    int cx = w->x + w->w - 18;
    int cy = w->y + 4;
    fill_rect(cx, cy, 14, 12, active ? 0xC0392B : 0x909090);
    rect_outline(cx, cy, 14, 12, 0x000000);
    draw_text(cx + 5, cy + 2, "X", 0xFFFFFF, 1);
}

static void draw_window_content(Window *w)
{
    int cx = w->x + BORDER;
    int cy = w->y + TITLE_H;
    int cw = w->w - 2 * BORDER;
    int ch = w->h - TITLE_H - BORDER;

    switch (w->kind) {
    case W_WELCOME:
        fill_rect(cx, cy, cw, ch, 0xF2F2EE);
        draw_text(cx + 10, cy + 10, "WELCOME TO H7 OS!", 0x202830, 1);
        draw_text(cx + 10, cy + 28, "- DRAG WINDOWS BY TITLE BAR", 0x405060, 1);
        draw_text(cx + 10, cy + 42, "- CLICK X TO CLOSE", 0x405060, 1);
        draw_text(cx + 10, cy + 56, "- CLICK + NEW IN TASKBAR", 0x405060, 1);
        draw_text(cx + 10, cy + 70, "- TYPE INTO THE NOTEPAD", 0x405060, 1);
        draw_text(cx + 10, cy + 92, "NO LIBS. JUST PORTS + PIXELS.", 0x8A4A20, 1);
        break;

    case W_NOTEPAD: {
        fill_rect(cx, cy, cw, ch, 0xFFFFFF);
        int cols = (cw - 12) / 6;
        if (cols < 1)
            cols = 1;
        int tx = cx + 6, ty = cy + 6, col = 0;
        for (int i = 0; i < w->tlen; i++) {
            char c = w->text[i];
            if (c == '\n' || col >= cols) {
                col = 0;
                ty += 10;
                if (c == '\n')
                    continue;
            }
            char s[2] = { c, 0 };
            draw_text(tx + col * 6, ty, s, 0x102030, 1);
            col++;
        }
        if ((ticks / 40) & 1)
            draw_text(tx + col * 6, ty, "_", 0x102030, 1);
        break;
    }

    case W_PALETTE: {
        fill_rect(cx, cy, cw, ch, 0x282830);
        static const u32 hues[7] = {
            0xE74C3C, 0xE67E22, 0xF1C40F, 0x2ECC71,
            0x3498DB, 0x9B59B6, 0xE91E63
        };
        int cell_w = (cw - 16) / 7;
        int cell_h = (ch - 16) / 5;
        for (int r = 0; r < 5; r++) {
            for (int c = 0; c < 7; c++) {
                u32 base = hues[c];
                int scale = 40 + r * 15;
                int rr = (int)((base >> 16) & 0xFF) * scale / 100;
                int gg = (int)((base >> 8) & 0xFF) * scale / 100;
                int bb = (int)(base & 0xFF) * scale / 100;
                fill_rect(cx + 8 + c * cell_w, cy + 8 + r * cell_h,
                          cell_w - 2, cell_h - 2,
                          (u32)((rr << 16) | (gg << 8) | bb));
            }
        }
        break;
    }

    case W_BOUNCE: {
        fill_rect(cx, cy, cw, ch, 0x101828);
        int r = 7;
        /* integrate ball position (called once per frame) */
        w->bx += w->bvx;
        w->by += w->bvy;
        if (w->bx < r)          { w->bx = r;          w->bvx = -w->bvx; }
        if (w->by < r)          { w->by = r;          w->bvy = -w->bvy; }
        if (w->bx > cw - r - 1) { w->bx = cw - r - 1; w->bvx = -w->bvx; }
        if (w->by > ch - r - 1) { w->by = ch - r - 1; w->bvy = -w->bvy; }
        fill_circle(cx + w->bx, cy + w->by, r, 0xF1C40F);
        fill_circle(cx + w->bx - 2, cy + w->by - 2, 2, 0xFFF4C0);
        break;
    }
    }
}

static void draw_window(int i)
{
    Window *w = &wins[i];
    bool active = (i == focused);

    shade_rect(w->x + 4, w->y + 4, w->w, w->h);     /* drop shadow */

    fill_rect(w->x, w->y, w->w, w->h, 0xC8C4BC);
    rect_outline(w->x, w->y, w->w, w->h, 0x1A1A1A);

    if (active)
        hgradient(w->x + 1, w->y + 1, w->w - 2, TITLE_H - 2,
                  0x0A2A6A, 0x3A6EA5);
    else
        fill_rect(w->x + 1, w->y + 1, w->w - 2, TITLE_H - 2, 0x7A7A7A);

    draw_text(w->x + 8, w->y + 6, w->title,
              active ? 0xFFFFFF : 0xD8D8D8, 1);
    draw_close_box(w, active);
    draw_window_content(w);
}

/* taskbar "+ NEW" button bounds */
#define BTN_X 8
#define BTN_W 58
#define BTN_H 20

static void format_clock(char *out)     /* "UP MM:SS" */
{
    u32 secs = ticks / 100;
    u32 m = (secs / 60) % 100;
    u32 s = secs % 60;
    out[0] = 'U'; out[1] = 'P'; out[2] = ' ';
    out[3] = '0' + m / 10;
    out[4] = '0' + m % 10;
    out[5] = ':';
    out[6] = '0' + s / 10;
    out[7] = '0' + s % 10;
    out[8] = 0;
}

static void draw_taskbar(void)
{
    int ty = screen_h - TASK_H;
    fill_rect(0, ty, screen_w, TASK_H, 0x1F242E);
    fill_rect(0, ty, screen_w, 1, 0x4A5564);

    int by = ty + (TASK_H - BTN_H) / 2;
    fill_rect(BTN_X, by, BTN_W, BTN_H, 0x2ECC71);
    rect_outline(BTN_X, by, BTN_W, BTN_H, 0x0E5E33);
    draw_text(BTN_X + 8, by + 6, "+ NEW", 0x06301A, 1);

    draw_text(BTN_X + BTN_W + 14, ty + 10, "H7 OS 0.1", 0x8E99A8, 1);

    char clk[9];
    format_clock(clk);
    draw_text(screen_w - text_width(clk, 1) - 10, ty + 10, clk, 0xC8D2E0, 1);
}

static void draw_cursor(void)
{
    static const char *shape[] = {
        "X",
        "XX",
        "X.X",
        "X..X",
        "X...X",
        "X....X",
        "X.....X",
        "X......X",
        "X.......X",
        "X........X",
        "X.....XXXXX",
        "X..X..X",
        "X.X X..X",
        "XX  X..X",
        "X    X..X",
        "     X..X",
        "      X..X",
        "       XX",
    };
    int mx = mouse_x, my = mouse_y;
    for (int row = 0; row < 18; row++) {
        for (int col = 0; shape[row][col]; col++) {
            char c = shape[row][col];
            if (c == 'X')
                px(mx + col, my + row, 0x000000);
            else if (c == '.')
                px(mx + col, my + row, 0xFFFFFF);
        }
    }
}

/* ---------- per-frame logic ---------- */

static void spawn_demo_window(void)
{
    static const char *titles[4] = { "WELCOME", "NOTEPAD", "PALETTE", "BOUNCE" };
    WinKind kind = (WinKind)(spawn_count % 4);
    int n = spawn_count % 6;
    spawn(kind, titles[kind], 90 + n * 45, 60 + n * 32, 220, 150);
}

static void handle_mouse(void)
{
    int mx = mouse_x, my = mouse_y;
    u8 btn = mouse_buttons;
    bool pressed  = (btn & 1) && !(prev_buttons & 1);
    bool released = !(btn & 1) && (prev_buttons & 1);

    if (pressed) {
        int by = screen_h - TASK_H + (TASK_H - BTN_H) / 2;
        if (my >= screen_h - TASK_H) {
            if (mx >= BTN_X && mx < BTN_X + BTN_W &&
                my >= by && my < by + BTN_H)
                spawn_demo_window();
        } else {
            int i = hit_test(mx, my);
            if (i >= 0) {
                raise_win(i);
                focused = i;
                Window *w = &wins[i];
                if (my < w->y + TITLE_H) {
                    if (mx >= w->x + w->w - 18 && mx < w->x + w->w - 4 &&
                        my >= w->y + 4 && my < w->y + 16) {
                        close_win(i);
                    } else {
                        drag = i;
                        drag_ox = mx - w->x;
                        drag_oy = my - w->y;
                    }
                }
            } else {
                focused = -1;
            }
        }
    }

    if (released)
        drag = -1;

    if (drag >= 0 && wins[drag].alive) {
        Window *w = &wins[drag];
        w->x = mx - drag_ox;
        w->y = my - drag_oy;
        if (w->x < -w->w + 60)
            w->x = -w->w + 60;
        if (w->x > screen_w - 60)
            w->x = screen_w - 60;
        if (w->y < 0)
            w->y = 0;
        if (w->y > screen_h - TASK_H - TITLE_H)
            w->y = screen_h - TASK_H - TITLE_H;
    }

    prev_buttons = btn;
}

void wm_init(void)
{
    spawn(W_WELCOME, "WELCOME", 60, 70, 250, 156);
    spawn(W_NOTEPAD, "NOTEPAD", 345, 90, 230, 150);
    spawn(W_PALETTE, "PALETTE", 120, 255, 215, 155);
    spawn(W_BOUNCE,  "BOUNCE",  395, 275, 185, 155);
    focused = zorder[0];        /* start with WELCOME focused */
    raise_win(zorder[0]);
}

void wm_frame(void)
{
    handle_mouse();

    draw_desktop();
    for (int j = 0; j < nz; j++)
        draw_window(zorder[j]);
    draw_taskbar();
    draw_cursor();
    present();
}
