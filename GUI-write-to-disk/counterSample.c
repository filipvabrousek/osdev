
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;


#define VGA_ADDRESS 0xB8000
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64


// Make counter

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

void waitForEnter(){  
    uint8_t scancode;
    char *v = (char *)VGA_ADDRESS;
    int counter = -1;
    char letters[20] = {'a', 'b', 'c'};

    while (1){
        while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
        scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x1C) {
            counter++;
          
          //  *v = letters[counter];
          
                *v = letters[counter];
           
        }
    }
}


void GPT_correct(){
    uint8_t scancode;
    char *v = (char *)VGA_ADDRESS;
    int counter = -1;
    char letters[] = {'a', 'b', 'c'};

    while (1) {
        while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
        scancode = inb(KEYBOARD_DATA_PORT);
       
        if (scancode == 0x1C) {
            counter++;
            *v = letters[counter];
        }
    }
}



int main(){
      char *v = (char *)VGA_ADDRESS;
      *v = 'a';
     // waitForEnter();
     GPT_correct();
}
