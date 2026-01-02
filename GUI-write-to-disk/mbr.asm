[bits 16]
[org 0x7c00]

; Memory offset where the kernel will be loaded
KERNEL_OFFSET equ 0x1000 

; BIOS stores the boot drive number in DL
mov [BOOT_DRIVE], dl 

; Set up the stack
mov bp, 0x9000 
mov sp, bp

; 1. Load the kernel from disk to memory
call load_kernel

; 2. SWITCH TO VGA GRAPHICS MODE 13h
; This must happen in 16-bit mode because it uses BIOS Interrupts.
; AH = 0x00 (Set Video Mode)
; AL = 0x13 (320x200 256-color graphics)
mov ah, 0x00
mov al, 0x13
int 0x10



; 3. Transition to 32-bit Protected Mode
call switch_to_32bit

; Safety hang
jmp $

; Include our helper files
%include "disk.asm"
%include "gdt.asm"
%include "switch-to-32bit.asm"

[bits 16]
load_kernel:
    mov bx, KERNEL_OFFSET 
    mov dh, 25            ; Number of sectors to read
    mov dl, [BOOT_DRIVE]
    call disk_load
    ret

[bits 32]
BEGIN_32BIT:
    ; Now we are in 32-bit mode. 
    ; The screen is now a 320x200 pixel canvas starting at 0xA0000.
    call KERNEL_OFFSET 
    jmp $

; Data storage
BOOT_DRIVE db 0

; Bootsector padding and magic number
times 510 - ($-$$) db 0
dw 0xaa55