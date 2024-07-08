// BASE

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int size_t;

#define VGA_ADDRESS 0xB8000
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

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


// Allow user to type a string, when he hits enter,
// the reversed string in green will be written




void reverseOnEnter(){
    uint8_t scancode;
    uint16_t pos = 0;
    char *v = (char*)VGA_ADDRESS;

    char str[20];
     char reversed[20];

    while(1){
       while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
       scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0x1E){
            *(v + pos * 2) = 'A';
            pos++;
            str[pos - 1] = 'A';
        }

         if (scancode == 0x30){
            *(v + pos * 2) = 'B';
            pos++;
             str[pos - 1] = 'B';
        }

         if (scancode == 0x2E){
            *(v + pos * 2) = 'C';
            pos++;
             str[pos - 1] = 'C';
        }

         if (scancode == 0x1C){

          for (int i = 0; str[i] != '\0'; i++){
            reversed[i] = str[pos - i - 1];
          }

//pos++;
//*(v + pos * 2) = '-';
          for (int i = 0; reversed[i] != '\0'; i++){
             pos++;
             *(v + pos * 2) = reversed[i];
             *(v + pos * 2 + 1) = 0x0A; // Green 0x0B Blue (and should be green)
          }
        }
    }
}









int main(){
      
    reverseOnEnter();
}