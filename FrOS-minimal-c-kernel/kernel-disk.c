// BASE

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;

typedef unsigned int uint32_t;
typedef char int8_t;


#define VGA_ADDRESS 0xB8000

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CONTROL 0x3F6
#define ATA_SECONDARY_IO 0x170
#define ATA_SECONDARY_CONTROL 0x376

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_WRITE_PIO 0x30

#define FS_MAX_FILES 16
#define FS_FILENAME_LEN 32
#define FS_SECTOR_SIZE 512

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*) ptr;
    while (num--) {
        *p++ = (unsigned char) value;
    }
    return ptr;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*) str1 - *(unsigned char*) str2;
}


char* strncpy(char* dest, const char* src, size_t num) {
    char* start = dest;
    while (num && (*dest++ = *src++)) {
        num--;
    }
    while (num--) {
        *dest++ = '\0';
    }
    return start;
}


static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void insl(uint16_t port, void* addr, uint32_t cnt) {
    asm volatile("cld; rep insl" : "=D"(addr), "=c"(cnt) : "d"(port), "0"(addr), "1"(cnt) : "memory", "cc");
}

static inline void outsl(uint16_t port, const void* addr, uint32_t cnt) {
    asm volatile("cld; rep outsl" : "=S"(addr), "=c"(cnt) : "d"(port), "0"(addr), "1"(cnt) : "memory", "cc");
}

void ata_read(uint32_t lba, uint8_t sector_count, void* buffer) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, sector_count);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t) lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t) (lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t) (lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    for (int i = 0; i < 4; i++) inb(ATA_PRIMARY_IO + ATA_REG_STATUS);

    insl(ATA_PRIMARY_IO + ATA_REG_DATA, buffer, 128);
}

void ata_write(uint32_t lba, uint8_t sector_count, const void* buffer) {
    outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO + ATA_REG_SECCOUNT0, sector_count);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA0, (uint8_t) lba);
    outb(ATA_PRIMARY_IO + ATA_REG_LBA1, (uint8_t) (lba >> 8));
    outb(ATA_PRIMARY_IO + ATA_REG_LBA2, (uint8_t) (lba >> 16));
    outb(ATA_PRIMARY_IO + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    for (int i = 0; i < 4; i++) inb(ATA_PRIMARY_IO + ATA_REG_STATUS);

    outsl(ATA_PRIMARY_IO + ATA_REG_DATA, buffer, 128);
   // char *v = (char *)VGA_ADDRESS;
   // *v = buffer[0];
}



typedef struct {
    char name[FS_FILENAME_LEN];
    uint32_t start_sector;
    uint32_t size;
} file_t;

static file_t fs_files[FS_MAX_FILES];
static uint32_t fs_file_count = 0;


int main(){
    char *v = (char *)VGA_ADDRESS;
   // *v = 'D';

    const char* data = "Hello, Disk File System!";
 
    ata_write(1, 1, data);  // Write to sector 1

    char buffer[512];
   // buffer[0] = 'A';
   // buffer[1] = 'B';
   // buffer[2] = 'C';

    ata_read(1, 1, buffer);  // Read from sector 1


*(v + 2 * 2) = buffer[0];
*(v + 2 * 2 + 1) = 0x07;
*(v + 3 * 2) = buffer[1];
*(v + 3 * 2 + 1) = 0x07;
*(v + 4 * 2) = buffer[2];
*(v + 4 * 2 + 1) = 0x07;

   // char msg[2] = {buffer[0], '\0'};  // Read the first character
  //  *v = msg[0];
    //*(v + 1) = msg[2];
}