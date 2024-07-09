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

    // *(v + start + 0) = letters[0];
    for (int i = 0; i < letters[i] != '\0'; i++)
    {
        // i * 2 is important !!! we need to leave bit
        *((v + start) + i * 2) = letters[i];
    }
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
