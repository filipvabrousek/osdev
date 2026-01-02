[bits 32]
global _start
_start:
    ; Set up the data segment registers
    mov ax, 0x10 ; 0x10 is the data segment in your GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up a safe stack
    mov ebp, 0x90000
    mov esp, ebp

    extern main
    call main
    jmp $