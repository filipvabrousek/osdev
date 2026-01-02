 ;load page directory (eax has the address of the page directory) 
   mov eax, [esp+4]
   mov cr3, eax        
;
; ;enable 4MBpage
;	mov ebx, cr4           ; read current cr4 
;	or  ebx, 0x00000010    ; set PSE  - enable 4MB page
;	mov cr4, ebx           ; update cr4
;
; ;enable paging 
   mov ebx, cr0          ; read current cr0
   or  ebx, 0x80000000   ; set PG .  set pages as read-only for both userspace and supervisor, replace 0x80000000 above with 0x80010000, which also sets the WP bit.
   mov cr0, ebx          ; update cr0
   ret                   ; now paging is enabled
;
  ; https://gayan1999malinda.medium.com/build-your-own-operating-system-7-9f3a9cc34605 This one
  ; https://medium.com/@connorstack/how-does-a-higher-half-kernel-work-107194e46a64
  ; https://medium.com/@connorstack/how-does-an-os-enable-virtual-memory-696a8f75f274#.qz5hxhbcs
