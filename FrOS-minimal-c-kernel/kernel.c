
void itoa(int num, char * str);

typedef struct kernel
{
    //char name [20];
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


int count(int a, int b){
    return a * b;//'3' + 3 + 3;

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

void clear_screen(uint8_t color){
    // cast VGA text mode memory adress to pointer uint16_t
    uint16_t *vga_buffer = (uint16_t *) VGA_ADDRESS;

    // create blank character entry
    // convert space character ' ' to uint16_t lower 8 bits character, upper 8 bits the color
    // (uint16_t color << 8) shifts the color byte (argument of the function) left by 8 bits
    // to set the color attribute for the blank space
    uint16_t blank = (uint16_t) ' ' | ((uint16_t) color << 8);

    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++){
        vga_buffer[i] = blank;
    }
}


char scancode_to_char[256] = {
    [0x1E] = 'A', [0x30] = 'B', [0x2E] = 'C', [0x20] = 'D',
    [0x12] = 'E', [0x21] = 'F', [0x22] = 'G', [0x23] = 'H',
    [0x17] = 'I', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
    [0x32] = 'M', [0x31] = 'N', [0x18] = 'O', [0x19] = 'P',
    [0x10] = 'Q', [0x13] = 'R', [0x1F] = 'S', [0x14] = 'T',
    [0x16] = 'U', [0x2F] = 'V', [0x11] = 'W', [0x2D] = 'X',
    [0x15] = 'Y', [0x2C] = 'Z',
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x39] = ' ', // Space
    [0x1C] = '\n' // Enter
};

//char lettersa[3] = {'A', 'B', 'C'};
//char* scancodes[3] = {"0x1E", "0x30", "0x2E"};
/*
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int find_index_of_scancode(char* scancodes[], size_t length, const char* target) {
    for (size_t i = 0; i < length; i++) {
        if (scancodes[i] != NULL && strcmp(scancodes[i], target) == 0) {
            return i;
        }
    }
    return -1; // Return -1 if not found
}
*/

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}


void write_char_to_screen(char c, uint16_t pos, uint8_t color) {
    char *v = (char *)VGA_ADDRESS;
    v[pos * 2] = c;       // Set the character
    v[pos * 2 + 1] = color; // Set the attribute byte (color)
}

void waitForEnter() {
    uint8_t scancode;
    uint16_t pos = 30; // Position in video memory
    uint8_t color = 0x0F; // Default color (white on black)
   
 char *v = (char *)VGA_ADDRESS;



     while (1) {
          while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);
           scancode = inb(KEYBOARD_DATA_PORT);
           
            if (scancode == 0x1C) {  // 0x1C is the scan code for Enter
            write_char_to_screen('?', pos, 0x4F); // Red '?'
            pos++;
        }



           if (scancode == 0x1E) {  // 0x1C is the scan code for Enter
           // write_char_to_screen('A', pos, 0x4F); // Red '?'
          // write_char_to_screen(*(char*)scancode_to_char[0x1E], pos, 0x4F); writes S
     //     process_key(scancode, &pos);
      
      //   char el = 'E';//scancode_to_char[0x1E];
       // write_char_to_screen(el, pos, 0x4F);
         //   pos++;
/*
if (scancode_to_char[0x1E] != NULL) {
  *(v + pos * 2) = 'D'; 
}*/


         *(v + pos * 2) = 'A';  // scancode_to_char[0x1E]; //'D'; 
        pos++;
            
        }

         if (scancode == 0x30) { 
            *(v + pos * 2) = 'B';  // scancode_to_char[0x1E]; //'D'; 
            pos++;
         }

          if (scancode == 0x2E) { 
            *(v + pos * 2) = 'C';  // scancode_to_char[0x1E]; //'D'; 
            pos++;
         }

       //   write_char_to_screen(scancode_to_char[scancode], pos, 0x4F); // Red '?'
        //    pos++;


          
          }
  
    while (1) {
        // Wait until the keyboard controller has data
        while ((inb(KEYBOARD_STATUS_PORT) & 1) == 0);

        // Read the scan code
        scancode = inb(KEYBOARD_DATA_PORT);

        // Convert scan code to character
        char c = scancode_to_char[0x1C];

        // If valid character, write it to the screen
        if (c) {

             write_char_to_screen('?', pos, 0x4F);

            if (c == '\n') {
                // Move to next line
                pos = (pos / SCREEN_WIDTH + 1) * SCREEN_WIDTH;
            } else {
                write_char_to_screen(c, pos, color);
                pos++;
            }
        }

        // Check for specific keys to change color or break the loop
       /* if (scancode == 0x1C) {  // 0x1C is the scan code for Enter
            write_char_to_screen('?', pos, 0x4F); // Red '?'
            pos++;
        }*/
    }
}

void main(){



    clear_screen(0x07);


    char * v = (char*) 0xb8000;
  //  *v = 'a';
    *v = count(2, 3);  // 6  (2, 8) => @                     //'3' + 3 + 3;
*v = scancode_to_char[0x1E];
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

    for (int i = 0; buffer[i] != '\0'; i++){
        *(v + i * 2) = buffer[i];
        // setting attribute byte at odd adress to green color
        *(v + i * 2 + 1) = 0x0A; 
    }

    *(v + 20) = 'p';
    *(v + 21) = 'o'; // not displayed
    *(v + 22) = 'm';

 

// 18          pmABCDEFG18 (why is 18 at the end???)
    char letters[7] = "ABCDEFG\0"; // crashes with 20
    int start = 24; // not 23, we need to leave 1 bit for styles
 

// *(v + start + 0) = letters[0];
    for (int i = 0; i < letters[i] != '\0'; i++){
 // i * 2 is important !!! we need to leave bit
         *((v + start) + i * 2) = letters[i];
    }

      waitForEnter(); // will write '-' after enter pressed
}


void itoa(int num, char * str){
    int i = 0;
    int isNegative = 0;

// Handle 0 and add terminting character
    if (num == 0){
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

// Convert back to positive and set isNegative to 1
    if (num < 0){ 
        isNegative = 1;
        num = -num;
    }

    int len = 0;

    while (num != 0){
        len += 1;
        int rem = num % 10;
        str[i++] = rem + '0';
        num = num / 10;
        
    }

    if (isNegative){
        str[i++] = '-';
        str[i] = '\0';
    }

    // reverse string

    // reverse string
    char res[20];
    char arr[3] = {'o', 't', 'h'}; // one two three

    for (int j = 0; j < len; j++){
        int index = len - j - 1;
      //  str[index] = arr[j];
        res[j] = str[index];
    }

     for (int k = 0; k < len; k++){
        str[k] = res[k];
     }

       //  waitForEnter();

}

/* for (int i = 0; i < 3; i++) {
    *(v + start + i) = letters[i];
}*/

    // *(v + start + 1) = 'a'; + 0 wont work
    //   *(v + start + 3) = 'a';

    // Light magenta = 0xD (works!) 0x0 black
    // No date in kernel mode

    // multiply larger numbers




   /*  struct kernel a = {24};
   *v = 24; // arr. up
    *v = a.age; // Why arrow up ? also this *v = 24; 
    */
    // *v = 'a' + a.age; yea




// malloc(); #include <stdlib.h> cannot use
    
    //obj kernel = {24};

  /*  *v = '@';

    int a = 3;
    int b = 4;

    if (b > a){
        *v = 'U';
    }*/