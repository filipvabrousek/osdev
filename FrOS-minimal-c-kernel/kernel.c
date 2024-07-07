
void itoa(int num, char *str);

typedef struct kernel
{
    // char name [20];
    int age;
};

typedef unsigned int uint32_t;
typedef char int8_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define NULL ((void *)0)

// Disk write
// Define base I/O ports for disk controller (assuming ATA)
#define ATA_DATA_PORT 0x1F0
#define ATA_SECTOR_COUNT 0x1F2
#define ATA_LBA_LOW 0x1F3
#define ATA_LBA_MID 0x1F4
#define ATA_LBA_HIGH 0x1F5
#define ATA_DEVICE_SELECT 0x1F6
#define ATA_COMMAND 0x1F7
#define ATA_STATUS 0x1F7

// PAGING
#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_DIRECTORY_ENTRIES 1024

// MALLOC
#define MEMORY_POOL_SIZE 65536
// Align to 8 bytes for simplicity
#define ALIGN 8
#define ALIGN_MASK (ALIGN - 1)

typedef struct BlockHeader {
    size_t size;
    struct BlockHeader* next;
    int free;
} BlockHeader;

static uint8_t memory_pool[MEMORY_POOL_SIZE];
static BlockHeader* free_list = NULL;

/*
void initialize_memory_poolaaa() {
    free_list = (BlockHeader*)memory_pool;
    free_list->size = MEMORY_POOL_SIZE - sizeof(BlockHeader);
    free_list->next = NULL;
    free_list->free = 1;
} SCREEN FLICKER WITH THIS*/

/*void initialize_memory_poolaaa() {
    free_list = (BlockHeader*)memory_pool;
}also with this*/

void initialize_memory_poolaaa() {
   // free_list = (BlockHeader*)memory_pool;
}
/*
typedef struct
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t ignored : 1;
    uint32_t available : 3;
    uint32_t frame : 20;
} page_table_entry_t;

typedef struct
{
    uint32_t present : 1;
    uint32_t rw : 1;
    uint32_t user : 1;
    uint32_t write_through : 1;
    uint32_t cache_disabled : 1;
    uint32_t accessed : 1;
    uint32_t reserved : 1;
    uint32_t page_size : 1;
    uint32_t ignored : 1;
    uint32_t available : 3;
    uint32_t table_addr : 20;
} page_directory_entry_t;
*/

#define PAGE_DIR_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024
#define PAGE_SIZE 4096
#define KERNEL_BASE 0xC0000000


#define NUM_PAGES 1024
#define PAGE_FRAME_SIZE 4096

uint32_t page_directory[PAGE_DIR_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
uint32_t first_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));


//page_directory_entry_t page_directory[PAGE_DIRECTORY_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
//page_table_entry_t first_page_table[PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));


// continue with paging array definition here
// https://gayan1999malinda.medium.com/build-your-own-operating-system-7-9f3a9cc34605
#define NUM_PAGES 1024

typedef struct page
{
   unsigned int present    : 1;   // Page present in memory
   unsigned int rw         : 1;   // Read-only if clear, readwrite if set
   unsigned int user       : 1;   // Supervisor level only if clear
   unsigned int accessed   : 1;   // Has the page been accessed since last refresh?
   unsigned int dirty      : 1;   // Has the page been written to since last refresh?
   unsigned int unused     : 7;   // Amalgamation of unused and reserved bits
   unsigned int frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table
{
   page_t pages[1024] __attribute__((aligned(4096)));
} page_table_t;

unsigned int page_directory[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));
unsigned int page_table[NUM_PAGES] __attribute__((aligned(PAGE_FRAME_SIZE)));

void init_pagingaaa() {  // not even called
  //  int i; // okay
 // int i = 0; FLICKERS


 /* CRASHES
 
 for (i = 0; i < NUM_PAGES; i++) {
		page_directory[i] = 0x00000002;
        //page_directory[i] = (unsigned int)directory;    
     	}   */



   /* int i;

	// Create page directory, supervisor mode, read/write, not present : 0 1 0 = 2   
	for (i = 0; i < NUM_PAGES; i++) {
		page_directory[i] = 0x00000002;
        //page_directory[i] = (unsigned int)directory;    
     	}     

	// Create page table, supervisor mode, read/write, present : 0 1 1 = 3   
	// As the address is page aligned, it will always leave 12 bits zeroed.  
	for (i = 0; i < NUM_PAGES; i++) { 
	        page_table[i] = (i * 0x1000) | 3;
	}	
*/
	// put page_table into page_directory supervisor level, read/write, present
//	page_directory[0] = ((unsigned int)page_table) | 3;
	 	

// problem!!!
//  Write to the page in the higher half
//first_page_table[0] = (0 * PAGE_SIZE) | 3;

// also flickering
  //  uint32_t *higher_half_page = (uint32_t *)KERNEL_BASE;
  //  *higher_half_page = 0x12345678; // Example write operation









    // Clear the page directory
    // Huge problems with that
    // Try simple malloc() without paging (heap)
    // Then try to add paging

// first_page_table[0] = (0 * PAGE_SIZE) | 3;

     // Map the first 4 MB of physical memory to the higher half
   /* for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | 3; // 0x3 means present and read/write
    }*/

    // Set the first page table in the page directory
   /* page_directory[0] = (uint32_t)first_page_table | 3; // Map the first 4 MB of physical memory
    page_directory[KERNEL_BASE >> 22] = (uint32_t)first_page_table | 3; // Map the first 4 MB to the higher half
*/
    // Set the remaining entries in the page directory to not present
   /* for (uint32_t i = 1; i < PAGE_DIR_ENTRIES; i++) {
        if (i != (KERNEL_BASE >> 22)) {
            page_directory[i] = 0;
        }
    }*/

}


/* adding method makes the screen glitch memory problem???
void a(){

}
*/

int count(int a, int b)
{
    return a * b; //'3' + 3 + 3;
    //  return '0' + a * b; for (2, 3) without itoa

    /*
    When you want to directly get the character representation
    of a SINGLE digit (0-9), you use '0' + a * b.
    This approach is suitable when the product is guaranteed to be a single digit and you want to store or print it as a character.

     char ch = '0' + a * b;
     imagine
     char ch = '0' + 6;
     '0' (ASCII 48) + 6 = ASCII 54 which is the ASCII value of 6
     ASCII 55 = 7 so this checks out!
     */
}

void clear_screen(uint8_t color)
{
    // cast VGA text mode memory adress to pointer uint16_t
    uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;

    // create blank character entry
    // convert space character ' ' to uint16_t lower 8 bits character, upper 8 bits the color
    // (uint16_t color << 8) shifts the color byte (argument of the function) left by 8 bits
    // to set the color attribute for the blank space
    uint16_t blank = (uint16_t)' ' | ((uint16_t)color << 8);

    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga_buffer[i] = blank;
    }
}

// READING FROM KEYBOARD
static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data)
{
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void write_char_to_screen(char c, uint16_t pos, uint8_t color)
{
    char *v = (char *)VGA_ADDRESS;
    v[pos * 2] = c;         // Set the character
    v[pos * 2 + 1] = color; // Set the attribute byte (color)
}

void waitForEnter()
{
    uint8_t scancode;
    uint16_t pos = 30;    // Position in video memory
    uint8_t color = 0x0F; // Default color (white on black)

    char *v = (char *)VGA_ADDRESS;

    while (1)
    {
        while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0)
            ;
        scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x1C)
        {                                         // 0x1C is the scan code for Enter
            write_char_to_screen('?', pos, 0x4F); // Red '?'
            pos++;
        }

        if (scancode == 0x1E)
        {                         // 0x1C is the scan code for Enter
            *(v + pos * 2) = 'A'; // scancode_to_char[0x1E]; //'D';
            pos++;
        }

        if (scancode == 0x30)
        {
            *(v + pos * 2) = 'B'; // scancode_to_char[0x1E]; //'D';
            pos++;
        }

        if (scancode == 0x2E)
        {
            *(v + pos * 2) = 'C'; // scancode_to_char[0x1E]; //'D';
            pos++;
        }

        //   write_char_to_screen(scancode_to_char[scancode], pos, 0x4F); // Red '?'
        //    pos++;
    }
}

//---------------------------------MAIN
void main()
{
  
/*

#define VGA_ADDRESS 0xB8000


int main(){
    int j = 3; done not flicker
     char *v = (char *)VGA_ADDRESS;
     *v = 'H';

}
 */


  int j = 0; 
 // FLICKERS 07/07/24 16:27:40, we likely cause stack overflow... When I comment out clear_screen it is okay
// init_paging();

    clear_screen(0x07);

    char *v = (char *)0xb8000;
    //  *v = 'a';
    *v = count(2, 3); // 6  (2, 8) => @                     //'3' + 3 + 3;

    char buffer[20];
    int result = count(2, 9);
    itoa(result, buffer);

    /* v is the pointer to video memory at 0xb8000
     in x86 each character represented by 2 bytes
     one for the character, one for the attribute (e.g.g color)
     the i * 2 ensures that characters are written to the correct positions
     in video memory, leaving space for attributes */

    /*
    If cell contains 18
    0xb8000 : Cell 0 (character) - '1'
   0xb8001 : Cell 1 (attribute)
   0xb8002 : Cell 2 (character) - '8'
   0xb8003 : Cell 3 (attribute)
   0xb8004 : Cell 4 (empty)
   0xb8005 : Cell 5 (empty)

   The loop effectively writes characters to every second byte (v, v + 2, v + 4, etc.) in video memory, which corresponds to the character cells.
   This leaves the odd addresses (v + 1, v + 3, v + 5, etc.) untouched, preserving them for attributes like color or other display properties.
     */

    for (int i = 0; buffer[i] != '\0'; i++)
    {
        *(v + i * 2) = buffer[i];
        // setting attribute byte at odd adress to green color
        *(v + i * 2 + 1) = 0x0A;
    }

    *(v + 20) = 'p';
    *(v + 21) = 'o'; // not displayed
    *(v + 22) = 'm';

    // 18          pmABCDEFG18 (why is 18 at the end???)
    char letters[7] = "ABCDEFG\0"; // crashes with 20
    int start = 24;                // not 23, we need to leave 1 bit for styles

    // *(v + start + 0) = letters[0];
    for (int i = 0; i < letters[i] != '\0'; i++)
    {
        // i * 2 is important !!! we need to leave bit
        *((v + start) + i * 2) = letters[i];
    }

    waitForEnter(); // will write '-' after enter pressed
}

//---------------------------------ITOA----------------
void itoa(int num, char *str)
{
    int i = 0;
    int isNegative = 0;

    // Handle 0 and add terminting character
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Convert back to positive and set isNegative to 1
    if (num < 0)
    {
        isNegative = 1;
        num = -num;
    }

    int len = 0;

    while (num != 0)
    {
        len += 1;
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
    }

    if (isNegative)
    {
        str[i++] = '-';
        str[i] = '\0';
    }

    // own reverse string
    char res[20];
    char arr[3] = {'o', 't', 'h'}; // one two three

    for (int j = 0; j < len; j++)
    {
        int index = len - j - 1;
        res[j] = str[index];
    }

    for (int k = 0; k < len; k++)
    {
        str[k] = res[k];
    }
}