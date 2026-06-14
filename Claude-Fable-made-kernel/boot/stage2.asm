; stage2.asm -- second-stage loader (8 sectors at 0x7E00).
;   1. enables the A20 line
;   2. loads the kernel (LBA 9, 256 sectors) to 0x10000 via INT 13h ext
;   3. picks a 640x480 VESA mode with a linear framebuffer (32bpp
;      preferred, 24bpp fallback), sets it, and stores a boot-info
;      struct at 0x9000 for the kernel:
;        u32 framebuffer phys, u16 pitch, u16 width, u16 height, u8 bpp
;   4. enters 32-bit protected mode and jumps to the kernel at 0x10000

BITS 16
ORG 0x7E00

KERNEL_LBA     equ 9
KERNEL_SECTORS equ 256          ; 128 KiB ceiling
KERNEL_SEG     equ 0x1000       ; physical 0x10000
CHUNK          equ 32           ; sectors per INT 13h call (16 KiB)

MODEINFO       equ 0x8C00       ; VBE mode info scratch buffer
CTRLINFO       equ 0x8800       ; VBE controller info scratch buffer
BOOTINFO       equ 0x9000

start:
    mov     [boot_drive], dl

    ; ---- A20: BIOS call, then fast gate as backup ----
    mov     ax, 0x2401
    int     0x15
    in      al, 0x92
    or      al, 2
    out     0x92, al

    ; ---- load kernel ----
    mov     word [dap_lba], KERNEL_LBA
    mov     word [dap_seg], KERNEL_SEG
    mov     word [remaining], KERNEL_SECTORS
.load_loop:
    mov     ax, [remaining]
    test    ax, ax
    jz      .load_done
    cmp     ax, CHUNK
    jbe     .count_ok
    mov     ax, CHUNK
.count_ok:
    mov     [dap_count], ax
    mov     ah, 0x42
    mov     dl, [boot_drive]
    mov     si, dap
    int     0x13
    jc      disk_error
    mov     ax, [dap_count]
    sub     [remaining], ax
    add     [dap_lba], ax
    mov     ax, [dap_count]
    shl     ax, 5               ; sectors -> paragraphs (512/16)
    add     [dap_seg], ax
    jmp     .load_loop
.load_done:

    ; ---- VBE: get controller info ----
    xor     ax, ax
    mov     es, ax
    mov     di, CTRLINFO
    mov     dword [es:di], 'VBE2'
    mov     ax, 0x4F00
    int     0x10
    cmp     ax, 0x004F
    jne     vbe_error

    mov     ax, [CTRLINFO+14]   ; mode list: offset
    mov     [mode_off], ax
    mov     ax, [CTRLINFO+16]   ; mode list: segment
    mov     [mode_seg], ax

.mode_loop:
    mov     ax, [mode_seg]
    mov     fs, ax
    mov     si, [mode_off]
    mov     cx, [fs:si]
    add     word [mode_off], 2
    cmp     cx, 0xFFFF
    je      .modes_done

    ; fetch mode info for CX
    xor     ax, ax
    mov     es, ax
    mov     di, MODEINFO
    mov     ax, 0x4F01
    int     0x10
    cmp     ax, 0x004F
    jne     .mode_loop

    ; need: supported + graphics + linear framebuffer
    mov     ax, [MODEINFO]
    and     ax, 0x0091
    cmp     ax, 0x0091
    jne     .mode_loop
    cmp     word [MODEINFO+18], 640
    jne     .mode_loop
    cmp     word [MODEINFO+20], 480
    jne     .mode_loop
    mov     al, [MODEINFO+25]
    cmp     al, 32
    je      .found
    cmp     al, 24
    jne     .mode_loop
    cmp     word [fallback_mode], 0
    jne     .mode_loop
    mov     [fallback_mode], cx
    jmp     .mode_loop
.found:
    mov     [chosen_mode], cx
    jmp     .set_mode
.modes_done:
    mov     ax, [fallback_mode]
    test    ax, ax
    jz      vbe_error
    mov     [chosen_mode], ax
.set_mode:
    ; re-fetch info for the chosen mode, then set it with the LFB bit
    mov     cx, [chosen_mode]
    xor     ax, ax
    mov     es, ax
    mov     di, MODEINFO
    mov     ax, 0x4F01
    int     0x10
    mov     bx, [chosen_mode]
    or      bx, 0x4000
    mov     ax, 0x4F02
    int     0x10
    cmp     ax, 0x004F
    jne     vbe_error

    ; ---- boot info for the kernel ----
    mov     eax, [MODEINFO+40]  ; PhysBasePtr
    mov     [BOOTINFO+0], eax
    mov     ax, [MODEINFO+16]   ; BytesPerScanLine
    mov     [BOOTINFO+4], ax
    mov     ax, [MODEINFO+18]   ; XResolution
    mov     [BOOTINFO+6], ax
    mov     ax, [MODEINFO+20]   ; YResolution
    mov     [BOOTINFO+8], ax
    mov     al, [MODEINFO+25]   ; BitsPerPixel
    mov     [BOOTINFO+10], al

    ; ---- protected mode ----
    cli
    lgdt    [gdt_desc]
    mov     eax, cr0
    or      eax, 1
    mov     cr0, eax
    jmp     0x08:pm_start

disk_error:
    mov     si, msg_disk
    jmp     fatal
vbe_error:
    mov     si, msg_vbe
fatal:
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

BITS 32
pm_start:
    mov     ax, 0x10
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     ss, ax
    mov     esp, 0x90000
    jmp     0x10000

BITS 16

msg_disk db "H7: kernel load failed", 0
msg_vbe  db "H7: no 640x480 LFB VESA mode", 0

boot_drive    db 0
remaining     dw 0
mode_off      dw 0
mode_seg      dw 0
chosen_mode   dw 0
fallback_mode dw 0

align 4
dap:
    db 0x10
    db 0
dap_count:
    dw 0
dap_off:
    dw 0
dap_seg:
    dw 0
dap_lba:
    dq 0

align 8
gdt:
    dq 0                        ; null
    dq 0x00CF9A000000FFFF       ; code: base 0, limit 4G, 32-bit
    dq 0x00CF92000000FFFF       ; data: base 0, limit 4G, 32-bit
gdt_desc:
    dw gdt_desc - gdt - 1
    dd gdt

times 4096-($-$$) db 0          ; pad to exactly 8 sectors
