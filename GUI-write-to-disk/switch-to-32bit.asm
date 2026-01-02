; THE CODE HERE IS LIKELY UNNECESSARY


; 08:50 https://www.youtube.com/watch?v=Wh5nPn2U_1w
; https://chatgpt.com/c/7b00881b-ed3a-48ed-b298-066ebbdb3f0f

; Switch to 32-bit instruction mode.
; Initialize all segment registers to the same data segment.
; Set up the stack at a specific memory address (0x90000).
; Transfer control to a subroutine for further processing.

; This is necessary because:
; interupts must be disabled to avoid conflicts during transition (cli)
; GDT must be loaded to define new memory segments used in 32-bit mode
; PE bit in CR0 must be set to switch CPU to protected mode
; a far ump at the end to start executing 32 bit code

[bits 16]
switch_to_32bit:
    cli ; 1. disable interrupts (clears the interrupt flag IF in EFLAGS register)
        ; important as an interrupt (event CPU should react to) occurs during the switch,
        ; as interrupts handlers may not be set up correctly for new 32-bit mode
    lgdt [gdt_descriptor] ; 2. load the GDT descriptor
   
   ; eax contains control flags that modify basic operations of the CPU
   ; by moving cr0 to EAX register you can modify specific bits without affecting others
    mov eax, cr0 
   
   ; set PE bit in cr0 (which is now in eax), 
   ; the PE bit (bit 0) of the CR0 register enables protected mode when set
    or eax, 0x1

    ; writes the modified eax back to cr0, enabling protected mode
    mov cr0, eax
    jmp CODE_SEG:init_32bit ; 4. far jump by using a different segment

; the following is written in 32 bit mode
; ensures that the code is assembled correctly for 32-bit operation
[bits 32]
init_32bit: ; we are now using 32-bit instructions
    ; by setting all these segments to the same value (AX),
    ; we ensure that all segment-based adressing will use the same base adress simplifying memory managment
    mov ax, DATA_SEG ; 5. move adress of DATA_SEG into AX register ("variable")
    mov ds, ax ; set ds to ax (to DATA_SEG)
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; establish a stack base adress at 0x90000
    ; right at the top of the free space
    mov ebp, 0x90000 ; 6. update the stack right at the top of the free space
    mov esp, ebp

    call BEGIN_32BIT ; 7. Call a well-known label with useful code
