#define VIDEO_MEMORY ((unsigned char*)0xA0000)

void draw_pixel(int x, int y, unsigned char color) {
    VIDEO_MEMORY[y * 320 + x] = color;
}

void main() {
    // Draw a red square in the middle of the screen
    for(int y = 80; y < 120; y++) {
        for(int x = 140; x < 180; x++) {
            draw_pixel(x, y, 2); // 4 is Red in the default VGA palette
        }
    }
    
    while(1);
}

// 175843 wow!!!!

