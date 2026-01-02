// --- 1. TYPE DEFINITIONS ---
typedef unsigned char  uint8_t;
typedef signed char    int8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VIDEO_MEMORY ((uint8_t*)0xA0000)
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 200

// --- 2. HARDWARE I/O HELPERS ---
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// --- 3. GRAPHICS FUNCTIONS ---
void draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VIDEO_MEMORY[y * SCREEN_WIDTH + x] = color;
    }
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

void draw_cursor(int x, int y, uint8_t color) {
    draw_pixel(x, y, color);
    draw_pixel(x, y+1, color);
    draw_pixel(x, y+2, color);
    draw_pixel(x, y+3, color);
    draw_pixel(x+1, y+1, color);
    draw_pixel(x+2, y+2, color);
}

// --- 4. MOUSE DRIVER ---
void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) while (timeout--) { if ((inb(0x64) & 1) == 1) return; }
    else           while (timeout--) { if ((inb(0x64) & 2) == 0) return; }
}

void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

void init_mouse() {
    uint8_t status;
    mouse_wait(1);
    outb(0x64, 0xA8);
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    mouse_write(0xF6);
    inb(0x60); // Ack
    mouse_write(0xF4);
    inb(0x60); // Ack
}

// --- 5. MAIN WINDOW MANAGER ---
void main() {
    init_mouse();
    
    // Window State
    int win_x = 100, win_y = 50, win_w = 80, win_h = 50;
    int mouse_x = 160, mouse_y = 100;
    int dragging = 0;

    // Initial Background
    draw_rect(0, 0, 320, 200, 1); // Blue screen

    

    while(1) {
        if (inb(0x64) & 0x01) {
            if (inb(0x64) & 0x20) {
                uint8_t status = inb(0x60);
                int8_t rel_x = (int8_t)inb(0x60);
                int8_t rel_y = (int8_t)inb(0x60);

                // 1. Erase old positions
                draw_cursor(mouse_x, mouse_y, 1); 
                if (dragging) draw_rect(win_x, win_y, win_w, win_h, 1);

                // 2. Update coordinates
                mouse_x += rel_x;
                mouse_y -= rel_y;

                // Bounds checking
                if (mouse_x < 0) mouse_x = 0; if (mouse_y < 0) mouse_y = 0;
                if (mouse_x > 315) mouse_x = 315; if (mouse_y > 195) mouse_y = 195;

                // 3. Handle Dragging Logic
                if (status & 0x01) { // Left Button Pressed
                    if (!dragging) {
                        // Check if click is on the title bar (top 10 pixels of window)
                        if (mouse_x >= win_x && mouse_x <= win_x + win_w &&
                            mouse_y >= win_y && mouse_y <= win_y + 10) {
                            dragging = 1;
                        }
                    }
                    if (dragging) {
                        win_x = mouse_x - (win_w / 2);
                        win_y = mouse_y - 5;
                    }
                } else {
                    dragging = 0;
                }

                // 4. Redraw Everything
                draw_rect(win_x, win_y, win_w, win_h, 7); // Body
                draw_rect(win_x, win_y, win_w, 10, 4);    // Title Bar
                draw_cursor(mouse_x, mouse_y, 15);       // White Cursor
            }
        }
    }
}