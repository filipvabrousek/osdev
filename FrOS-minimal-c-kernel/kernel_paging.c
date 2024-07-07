
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
    int i; // okay
 // int i = 0; FLICKERS


// Create page directory, supervisor mode, read/write, not present : 0 1 0 = 2  
      for (i = 0; i < NUM_PAGES; i++) {
	    	page_directory[i] = 0x00000002;
       }  

 

 // Create page table, supervisor mode, read/write, present : 0 1 1 = 3   
	// As the address is page aligned, it will always leave 12 bits zeroed.  
	for (i = 0; i < NUM_PAGES; i++) { 
	        page_table[i] = (i * 0x1000) | 3;
	}	

    page_directory[0] = ((unsigned int)page_table) | 3;



   /* int i;

	 
	
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


#define VGA_ADDRESS 0xB8000


int main(){
     init_pagingaaa();
     
    int j = 3; //done not flicker
     char *v = (char *)VGA_ADDRESS;
     *v = 'H';

    

}