; switch from real mode to protected mode
; 32 bit operational mode so we can adress 32GBs of memory
; we can implement paging and multitasking
; flat memory model no segmentation
; (also exists: 1) flat memory
; 2) segmentation,
; 3) paging models)
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
