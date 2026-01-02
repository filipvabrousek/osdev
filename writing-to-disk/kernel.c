typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VGA_ADDRESS 0xB8000
#define ATA_STATUS  0x1F7
#define ATA_DATA    0x1F0

// Change this to a high number so we don't overwrite the kernel code!
// If your kernel is 10 sectors long, writing to Sector 2 destroys your OS.
#define STORAGE_LBA 50 

// --- 1. HARDWARE HELPERS ---

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

// --- 2. DISK FUNCTIONS ---

void read_simple_file(char* buffer) {
    while ((inb(0x1F7) & 0x80)); // Wait for drive not busy
    
    outb(0x1F6, 0xE0);           // Select master drive
    outb(0x1F2, 1);              // Read 1 sector
    outb(0x1F3, STORAGE_LBA);    // LBA Low byte
    outb(0x1F4, 0);              // LBA Mid byte
    outb(0x1F5, 0);              // LBA High byte
    outb(0x1F7, 0x20);           // Command: Read with retry

    while (!(inb(0x1F7) & 0x08)); // Wait for DRQ (Data Request)

    for (int i = 0; i < 256; i++) {
        uint16_t word = inw(0x1F0);
        buffer[i * 2] = (char)(word & 0xFF);
        buffer[i * 2 + 1] = (char)((word >> 8) & 0xFF);
    }
}

void write_simple_file(char* text) {
    while ((inb(0x1F7) & 0x80)); 
    
    outb(0x1F6, 0xE0);
    outb(0x1F2, 1);
    outb(0x1F3, STORAGE_LBA); 
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0x30);           // Command: Write sectors

    while (!(inb(0x1F7) & 0x08)); 

    for (int i = 0; i < 256; i++) {
        // Construct word from two bytes
        uint16_t word = (uint8_t)text[i * 2] | ((uint16_t)(uint8_t)text[i * 2 + 1] << 8);
        outw(0x1F0, word);
    }
}

// --- 3. SCREEN FUNCTIONS ---

void write_char_to_screen(char c, uint16_t pos, uint8_t color) {
    char *v = (char *)VGA_ADDRESS;
    v[pos * 2] = c;
    v[pos * 2 + 1] = color;
}

void clear_screen(uint8_t color) {
    uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;
    uint16_t blank = (uint16_t)' ' | ((uint16_t)color << 8);
    for (int i = 0; i < 80 * 25; i++) vga_buffer[i] = blank;
}

// --- 4. DATA ---

static char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0, 0,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

static char file_buffer[512];
static char loaded_buffer[512];

// --- 5. MAIN ---

void main() {
    clear_screen(0x07);
    
    // Initialize buffers to zero
    for(int i=0; i<512; i++) {
        file_buffer[i] = 0;
        loaded_buffer[i] = 0;
    }

    // Flush keyboard
    while (inb(0x64) & 1) inb(0x60);

    // READ existing content
    read_simple_file(loaded_buffer);
    
    int screen_pos = 0;
    // Check if sector has data (not empty or uninitialized)
    if (loaded_buffer[0] != 0 && (uint8_t)loaded_buffer[0] != 0xFF) {
        char* msg = "Disk Data: ";
        for(int i = 0; msg[i] != 0; i++) write_char_to_screen(msg[i], screen_pos++, 0x0B);
        for(int i = 0; i < 40 && loaded_buffer[i] != 0; i++) {
            write_char_to_screen(loaded_buffer[i], screen_pos++, 0x0F);
        }
    }

    screen_pos = 80; 
    char* prompt = "Type to save to Disk (Press ENTER):";
    for(int i = 0; prompt[i] != 0; i++) write_char_to_screen(prompt[i], screen_pos++, 0x0E);
    
    screen_pos = 160; 
    int buffer_index = 0;

  /*  while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (scancode & 0x80) continue; // Key release

            if (scancode == 0x1C) { // ENTER KEY
                write_simple_file(file_buffer);
                write_char_to_screen('D', 78, 0x0A); // 'Done' indicator
                write_char_to_screen('N', 79, 0x0A);
                break;
            }

            if (scancode < sizeof(scancode_to_ascii) && buffer_index < 510) {
                char c = scancode_to_ascii[scancode];
                if (c != 0) {
                    file_buffer[buffer_index++] = c;
                    write_char_to_screen(c, screen_pos++, 0x0F);
                }
            }
        }
    }*/

    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (scancode & 0x80) continue; 

            if (scancode == 0x1C) { // ENTER KEY
                // 1. Write the current buffer to disk
                write_simple_file(file_buffer);
                
                // 2. Visual confirmation
                write_char_to_screen('D', 78, 0x0A); 
                write_char_to_screen('N', 79, 0x0A);

                // 3. READ it back into the loaded_buffer to verify
                for(int i=0; i<512; i++) loaded_buffer[i] = 0; // Clear old data first
                read_simple_file(loaded_buffer);

                // 4. Update the top line with the NEWLY read data
                int verify_pos = 13; // Position after "Disk Data: "
                for(int i = 0; i < 40; i++) {
                    if (loaded_buffer[i] != 0) {
                        write_char_to_screen(loaded_buffer[i], verify_pos++, 0x0B); // Light Blue
                    } else {
                        write_char_to_screen(' ', verify_pos++, 0x0B);
                    }
                }
                
                // Optional: Clear typing line for next message
                buffer_index = 0;
                for(int i=0; i<80; i++) write_char_to_screen(' ', 160 + i, 0x0F);
                screen_pos = 160;
                
            } else if (scancode < sizeof(scancode_to_ascii) && buffer_index < 510) {
                char c = scancode_to_ascii[scancode];
                if (c != 0) {
                    file_buffer[buffer_index++] = c;
                    write_char_to_screen(c, screen_pos++, 0x0F);
                }
            }
        }
    }
    
    // Halt
    while(1) { __asm__("hlt"); }
}

// 173010 Amazing!!!!
// Thank you Gemini!

