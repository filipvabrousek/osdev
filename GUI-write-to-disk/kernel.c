typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VIDEO_MEMORY ((uint8_t*)0xA0000)
#define STORAGE_LBA 100 

// --- 1. GLOBAL STATE ---
static volatile uint8_t file_buffer[512];
static int cursor_pos = 0;
static int win_active = 1;
static int ctrl_pressed = 0;
static int needs_redraw = 1;

// --- 2. HARDWARE I/O ---
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t data) {
    __asm__ volatile("outw %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t result;
    __asm__ volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void io_wait() {
    __asm__ volatile("outb %%al, $0x80" : : "a"(0));
}

// --- 3. DISK DRIVER ---
void wait_disk() {
    while ((inb(0x1F7) & 0x80) || !(inb(0x1F7) & 0x40)) io_wait();
}

void read_disk(volatile uint8_t* buffer) {
    wait_disk();
    outb(0x1F6, 0xE0 | ((STORAGE_LBA >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)STORAGE_LBA);
    outb(0x1F4, (uint8_t)(STORAGE_LBA >> 8));
    outb(0x1F5, (uint8_t)(STORAGE_LBA >> 16));
    outb(0x1F7, 0x20); 

    while (!(inb(0x1F7) & 0x08)) io_wait();

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2] = (uint8_t)(data & 0xFF);
        buffer[i * 2 + 1] = (uint8_t)((data >> 8) & 0xFF);
    }
}

void write_disk(volatile uint8_t* buffer) {
    wait_disk();
    outb(0x1F6, 0xE0 | ((STORAGE_LBA >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)STORAGE_LBA);
    outb(0x1F4, (uint8_t)(STORAGE_LBA >> 8));
    outb(0x1F5, (uint8_t)(STORAGE_LBA >> 16));
    outb(0x1F7, 0x30); 

    while (!(inb(0x1F7) & 0x08)) io_wait();

    for (int i = 0; i < 256; i++) {
        uint16_t low = buffer[i * 2];
        uint16_t high = buffer[i * 2 + 1];
        outw(0x1F0, low | (high << 8));
    }
    outb(0x1F7, 0xE7); // Flush
    wait_disk();
}

// --- 4. DATA TABLES ---
unsigned char font8x8[128][8] = {
    ['A']={0x18,0x3C,0x66,0x7E,0x66,0x66,0x66,0x00}, ['B']={0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    ['C']={0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, ['D']={0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    ['E']={0x7E,0x60,0x7C,0x60,0x60,0x60,0x7E,0x00}, ['F']={0x7E,0x60,0x7C,0x60,0x60,0x60,0x60,0x00},
    ['G']={0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00}, ['H']={0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    ['I']={0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00}, ['J']={0x1E,0x0C,0x0C,0x0C,0x0C,0x6C,0x38,0x00},
    ['K']={0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00}, ['L']={0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    ['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00}, ['N']={0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},
    ['O']={0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, ['P']={0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    ['Q']={0x3C,0x66,0x66,0x66,0x6E,0x3C,0x0E,0x00}, ['R']={0x7C,0x66,0x66,0x7C,0x78,0x6C,0x66,0x00},
    ['S']={0x3C,0x66,0x30,0x1C,0x06,0x66,0x3C,0x00}, ['T']={0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    ['U']={0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00}, ['V']={0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    ['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00}, ['X']={0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    ['Y']={0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00}, ['Z']={0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
    ['0']={0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00}, ['1']={0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['2']={0x3C,0x66,0x06,0x1C,0x30,0x60,0x7E,0x00}, ['3']={0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    ['4']={0x0C,0x1C,0x3C,0x6C,0x7E,0x0C,0x0C,0x00}, ['5']={0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    ['6']={0x1C,0x30,0x60,0x7C,0x66,0x66,0x3C,0x00}, ['7']={0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
    ['8']={0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, ['9']={0x3C,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    [' ']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, ['.']={0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00}
};

const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0,
    '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' '
};

// --- 5. GRAPHICS ---
void draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < 320 && y >= 0 && y < 200) VIDEO_MEMORY[y * 320 + x] = color;
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = 0; i < h; i++) for (int j = 0; j < w; j++) draw_pixel(x + j, y + i, color);
}

void draw_char(int x, int y, char c, uint8_t color) {
    if (c < 32 || c > 126) return;
    unsigned char* b = font8x8[(int)c];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (b[i] & (0x80 >> j)) draw_pixel(x + j, y + i, color);
        }
    }
}

// --- 6. MAIN ---
void main() {
    // Fill screen with blue
    for(int i=0; i<320*200; i++) VIDEO_MEMORY[i] = 1; 

    // Load existing data from sector 100
    read_disk(file_buffer);

    // Restore cursor position based on existing data
    cursor_pos = 0;
    for (int i = 0; i < 512; i++) {
        if (file_buffer[i] >= 32 && file_buffer[i] <= 126) cursor_pos = i + 1;
    }

    while(1) {
        if (inb(0x64) & 0x01) { // Check keyboard buffer
            uint8_t code = inb(0x60);
            
            if (code == 0x1D) ctrl_pressed = 1;
            else if (code == 0x9D) ctrl_pressed = 0;

            if (!(code & 0x80)) { // Key Down
                // SHORTCUTS
                if (ctrl_pressed && code == 0x2D) { // CTRL+X
                    win_active = 0; 
                    for(int i=0; i<320*200; i++) VIDEO_MEMORY[i] = 1;
                }
                else if (ctrl_pressed && code == 0x18) { // CTRL+O
                    win_active = 1; needs_redraw = 1;
                }
                else if (code == 0x1C) { // ENTER = SAVE
                    write_disk(file_buffer);
                    draw_rect(20, 30, 280, 5, 2); // Save Indicator
                }
                // TYPING
                else if (win_active) {
                    char ascii = kbd_us[code];
                    if (ascii >= 32 && cursor_pos < 512) {
                        file_buffer[cursor_pos++] = ascii;
                        needs_redraw = 1;
                    } else if (ascii == '\b' && cursor_pos > 0) {
                        file_buffer[--cursor_pos] = 0;
                        needs_redraw = 1;
                    }
                }
            }
        }

        if (needs_redraw && win_active) {
            draw_rect(20, 30, 280, 140, 7); // Body
            draw_rect(20, 30, 280, 10, 4);  // Red Title Bar
            for(int i = 0; i < 512; i++) {
                if (file_buffer[i] >= 32) {
                    int tx = 25 + (i % 30) * 9;
                    int ty = 45 + (i / 30) * 10;
                    draw_char(tx, ty, file_buffer[i], 0);
                }
            }
            needs_redraw = 0;
        }
    }
}