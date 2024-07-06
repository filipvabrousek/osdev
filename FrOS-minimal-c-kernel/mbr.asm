
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
%include "paging.asm"

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
