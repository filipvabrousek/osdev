; entry.asm -- kernel entry point (placed first at 0x10000 by the
; linker script) plus the interrupt stubs that bridge into C.

BITS 32

section .entry progbits alloc exec nowrite align=16

global _start
extern kmain
extern __bss_start
extern __bss_end

_start:
    mov     esp, 0x90000
    ; flat binary carries no .bss -- zero it before C runs
    mov     edi, __bss_start
    mov     ecx, __bss_end
    sub     ecx, edi
    xor     eax, eax
    cld
    rep stosb
    call    kmain
.hang:
    cli
    hlt
    jmp     .hang

section .text

extern timer_handler
extern kbd_handler
extern mouse_handler

%macro IRQ_STUB 2
global %1
%1:
    pusha
    cld
    call    %2
    popa
    iretd
%endmacro

IRQ_STUB irq_timer_stub, timer_handler
IRQ_STUB irq_kbd_stub,   kbd_handler
IRQ_STUB irq_mouse_stub, mouse_handler

; spurious / unhandled hardware interrupt: acknowledge both PICs, return
global isr_unhandled_stub
isr_unhandled_stub:
    push    eax
    mov     al, 0x20
    out     0xA0, al
    out     0x20, al
    pop     eax
    iretd

; CPU exception: halt hard (no recovery story in a toy kernel)
global isr_exception_stub
isr_exception_stub:
    cli
.hang:
    hlt
    jmp     .hang
