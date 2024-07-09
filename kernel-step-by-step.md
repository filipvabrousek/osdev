## Writing to video memory

```c
#define VGA_ADDRESS 0xB8000

int main(){
 char *v = (char *)VGA_ADDRESS;
 *v = 'A';
}
```

## Writing array to screen
```c
    char letters[7] = "ABCDEFG\0"; // crashes with 20
    int start = 24;                // not 23, we need to leave 1 bit for styles

    for (int i = 0; i < letters[i] != '\0'; i++) {
        // i * 2 is important, we need to leave bit
        *((v + start) + i * 2) = letters[i];
    }

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

```


## Clear screen

```c
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
```



## Reading from keyboard

```c

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


```


## Counting using itoa

```c


int count(int a, int b) {
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

int main(){
 char buffer[20];
 int result = count(2, 9);
 itoa(result, buffer);
 return 0;
}
```
