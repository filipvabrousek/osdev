## Step by step 


### mbr.asm
* bootloader

```asm
; specifies that the following assembly code in 16 bit asm and CPU operates in real mode
[bits 16]

; set starting adress of the bootloader to standard location for bootloaders 
[org 0x7c00]

; define constant with memory adress 0x1000 where the kernel will be loaded
; The same one we used when linking the kernel
KERNEL_OFFSET equ 0x1000 

; Stores the value of the dl register (which contains the boot drive number) into the memory location pointed to by BOOT_DRIVE.

; Remember that the BIOS sets us the boot drive in 'dl' on boot
; set up stack pointer by initializing it with value 0x9000
; stack is used for managing function calls and variables
; we need this to interface with C!
mov [BOOT_DRIVE], dl 
mov bp, 0x9000
mov sp, bp

call load_kernel ; read the kernel from disk
call switch_to_32bit ; disable interrupts, load GDT,  etc. Finally jumps to 'BEGIN_PM'
jmp $ ; Never executed

%include "disk.asm"
%include "gdt.asm"
%include "switch-to-32bit.asm"

[bits 16] ; asm written in 16-bit mode
load_kernel:
    ;set up memory adress where kernel will be loaded
    ;set up disk operations
    mov bx, KERNEL_OFFSET ; Read from disk and store in 0x1000
    mov dh, 2
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
BEGIN_32BIT:
    call KERNEL_OFFSET ; Give control to the kernel
    jmp $ ; Stay here when the kernel returns control to us (if ever)


; store BOOT_DRIVE number
BOOT_DRIVE db 0 ; It is a good idea to store it in memory because 'dl' may get overwritten


; padding
; fill the remaining bytes in the 512-byte boot sector with zeros
times 510 - ($-$$) db 0

; boot singature (magic number 0xaa55)
; this indicates that this is valid boot sector
; BIOS checks for this signature before executing bootloader 
dw 0xaa55
```

### gdt.asm
* switch from real mode to protected mode

```asm
; switch from real mode to protected mode
; 32 bit operational mode so we can adress 32GBs of memory
; we can implement paging and multitasking
; flat memory model no segmentation (it is form of it)
; also exists:
; 1) flat memory
; 2) segmentation,
; 3) paging models
; GDT = general description table  
; segments = how big am I, who can access me, am I executable?

gdt_start: ; don't remove the labels, they're needed to compute sizes and jumps
    ; the GDT starts with a null 8-byte
    dd 0x0 ; 4 byte needs to be empt descriptor
    dd 0x0 ; 4 byte

; GDT for code segment. base = 0x00000000, length = 0xfffff
; for flags, refer to os-dev.pdf document, page 36
; code segment descriptor has
; present = 1 for used segments,
; privilege = two bit value 00 | 01 | 10 | 11 and type
; privilege = controls hiearchy and memory protection
; type = 1 if it is code or data segment

; type flags 10
; 1st, code segment => 1
; 2nd conforming can this code be executed from higher privilege segment
; it is highest privilege so 0
; 3rd readable: 1 to read constants in segment
; 4rd managed by CPU ? => 0

; other flags
; 5th granularity => 1 we can use 4GBs of memory
; 6th is this segment going to use 32-bit memory? => 1
; we are not gonna use 7th and 8th bits, so we set them to 0

; https://www.youtube.com/watch?v=Wh5nPn2U_1w
; db: define bytes
; dw: define words
; dd: define double words
gdt_code:
    dw 0xffff    ; segment length, bits 0-15 VIDEO: first 16 bits of the limit
    dw 0x0       ; segment base, bits 0-15
    db 0x0       ; segment base, bits 16-23 define VIDEO: 1st 24 bits of the base
    db 10011010b ; flags (8 bits) TYPE FLAGS
    db 11001111b ; flags (4 bits) + segment length, bits 16-19 OTHER FLAGS
    db 0x0       ; segment base, bits 24-31 LAST 8 BITS

; GDT for data segment. base and length identical to code segment
; some flags changed, again, refer to os-dev.pdf
gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

; GDT descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; size (16 bit), always one less of its true size
    dd gdt_start ; address (32 bit)

; offset of the segment descriptor relative to the beginning of GDT
; define some constants for later use
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
```



## disk.asm

```asm
; CHS adressing
; cylinder, head, sector

; load 'dh' sectors from drive 'dl' into ES:BX
disk_load:
    pusha
    ; reading from disk requires setting specific values in all registers
    ; so we will overwrite our input parameters from 'dx'. Let's save it
    ; to the stack for later use.
    push dx

    mov ah, 0x02 ; ah <- int 0x13 function. 0x02 = 'read'
    mov al, dh   ; al <- number of sectors to read (0x01 .. 0x80)
    mov cl, 0x02 ; cl <- sector (0x01 .. 0x11)
                 ; 0x01 is our boot sector, 0x02 is the first 'available' sector
    mov ch, 0x00 ; ch <- cylinder (0x0 .. 0x3FF, upper 2 bits in 'cl')
    ; dl <- drive number. Our caller sets it as a parameter and gets it from BIOS
    ; (0 = floppy, 1 = floppy2, 0x80 = hdd, 0x81 = hdd2)
    mov dh, 0x00 ; dh <- head number (0x0 .. 0xF)

    ; [es:bx] <- pointer to buffer where the data will be stored
    ; caller sets it up for us, and it is actually the standard location for int 13h
    int 0x13      ; BIOS interrupt
    jc disk_error ; if error (stored in the carry bit)

    pop dx
    cmp al, dh    ; BIOS also sets 'al' to the # of sectors read. Compare it.
    jne sectors_error
    popa
    ret


disk_error:
    jmp disk_loop

sectors_error:
    jmp disk_loop

disk_loop:
    jmp $
```

## kernel_entry.asm
```asm
[bits 32]
[extern main] ; Define calling point. Must have same name as kernel.c 'main' function
call main ; Calls the C function. The linker will know where it is placed in memory
jmp $
```

## kernel.c
```c
void main() {
    char* video_memory = (char*) 0xb8000;
    *video_memory = 'L';
}
```


## makefile

```make
# $@ = target file
# $< = first dependency
# $^ = all dependencies

# First rule is the one executed when no parameters are fed to the Makefile
all: run

# Notice how dependencies are built as needed
kernel.bin: kernel_entry.o kernel.o
	x86_64-elf-ld -m elf_i386 -o $@ -Ttext 0x1000 $^ --oformat binary

kernel_entry.o: kernel_entry.asm
	nasm $< -f elf -o $@

paging.o: paging.asm
	nasm $< -f elf -o $@

# kernel_paging_only/kernel_paging.c
kernel.o: kernel.c
	x86_64-elf-gcc -m32 -ffreestanding -c $< -o $@

# Disassemble
kernel.dis: kernel.bin
	ndisasm -b 32 $< > $@

mbr.bin: mbr.asm
	nasm $< -f bin -o $@

os-image.bin: mbr.bin kernel.bin
	cat $^ > $@

run: os-image.bin
	qemu-system-i386 -fda $<

echo: os-image.bin
	xxd $<

clean:
	$(RM) *.bin *.o *.dis
```
