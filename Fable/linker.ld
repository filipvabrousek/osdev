OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

SECTIONS
{
    . = 0x10000;

    .text : {
        *(.entry)
        *(.text*)
    }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*) }

    __bss_start = .;
    .bss : {
        *(.bss*)
        *(COMMON)
    }
    __bss_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note*)
        *(.comment)
    }
}
