; stage1.asm -- 512-byte MBR boot sector.
; Loads stage2 (8 sectors, LBA 1..8) to 0x7E00 using BIOS INT 13h
; extensions, then jumps to it with the boot drive still in DL.

BITS 16
ORG 0x7C00

start:
    cli
    xor     ax, ax
    mov     ds, ax
    mov     es, ax
    mov     ss, ax
    mov     sp, 0x7C00
    sti

    mov     [boot_drive], dl

    mov     si, dap
    mov     ah, 0x42
    int     0x13
    jc      disk_error

    mov     dl, [boot_drive]
    jmp     0x0000:0x7E00

disk_error:
    mov     si, msg_err
.print:
    lodsb
    test    al, al
    jz      .hang
    mov     ah, 0x0E
    mov     bx, 0x0007
    int     0x10
    jmp     .print
.hang:
    hlt
    jmp     .hang

msg_err     db "H7: disk read error", 0
boot_drive  db 0

align 4
dap:
    db 0x10                 ; DAP size
    db 0
    dw 8                    ; sector count (stage2 = 8 sectors)
    dw 0x7E00               ; destination offset
    dw 0x0000               ; destination segment
    dq 1                    ; starting LBA

times 510-($-$$) db 0
dw 0xAA55
