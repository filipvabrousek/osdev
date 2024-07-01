## asm

### bootloader
``` times``` directive explained, ensures the total number of bytes up to this point is 510
```510-($-$$)```   
- ```$``` is current adress, 
- ```$$``` represents the start of the section (beginning of the program)
- ```510-($-$$)``` defines the number of bytes to fill up to 510 bytes
- ```db 0``` - 510 zeros
```asm
jmp $ ; infinite loop by jumping to current adress
times 510-($-$$) db 0  ;510 times 0
db 0x55, 0xaa ;510 zeroes needs to end with 55aa for the BIOS to find it in bootloader
```


### basics

```asm
mov ah, 0x0e ; BIOS routine, switch to teletype mode
mov al, 'A' ; move character we want to print into al
int 0x10 ; 0x10 interrupt to print (response of the CPU to an event)

inc al
int 0x10

inc al
int 0x10

; je = jump if equal if 3 == 3 it jumps to label which prints E
mov bx, 3
cmp bx, 3
je label
jmp $

label: 
         mov ah, 0x0e ; BIOS routine, switch to teletype mode
         mov al, 'E' ; move character we want to print into al
         int 0x10 ; 0x10 interrupt to print (response of the CPU to an event)

; while loop
;loop: ; while(1)
        ;inc al
        ;cmp al, 'Z' + 1 ; if (al == 'Z' + 1) break
        ;je exit ; break
        ;int 0x10; call interrupt to process write insctruction
        ;jmp loop; loop

;exit: 
        ;jmp $


[org 0x7c00]
;-----------------------------print single char string
;mov ah, 0x0e
;mov al, [name];[name + 0x7c00]
;int 0x10
;jmp $
;-----------------------------print single char  string

;-----------------------------print string
mov ah, 0x0e
mov bx, name;[name + 0x7c00]

printString:
    mov al, [bx]
    cmp al, 0
    je end
    int 0x10
    inc bx
    jmp printString
end:
;-----------------------------print string

;-----------------------------name needed for both samples
name:
    db "Ing.", 0


; keyboard input (not working)
char:
    db 0
mov ah, 0
int 0x16

mov al, [char]

; push 6    pop ax is same as mov ax, 6
; segmentation - divides memory to be able to use more adresses
; ds = data segment
; cs = code segment
; ss = stack segment

; 75200 = 16 * ds + offset
; 75200 = ds : offset
; [org 0x7co] OR move ds, 0x7co


; CHS adressing cylinder (divided into sectors), head (reading mechanism), sector

; read from disk (not working)
;mov ah, 2
;mov al, 1
;mov ch, 0
;mov cl, 2
;mov dh, 0
;mov dl, 0 ; [disknum]
;mov es, 0
;mov bx, 0x7e00
;int 0x13
;mov ah, 0x0e
;mov al, [0x7e00]
;int 0x10

; boot.asm:89: error: invalid combination of opcode and operands

jmp $ ; infinite loop
times 510-($-$$) db 0  ;510 times 0
db 0x55, 0xaa ;510 zeroes needs to end with 55aa for the BIOS to find it in bootloader



; brew install nasm
; brew install qemu
; nasm -f bin boot.asm -o boot.bin && qemu-system-x86_64 boot.bin
```
