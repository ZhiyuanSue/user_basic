OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS {

    . = 0x400000 ;
    .text : { 
        *(.text.entry)
        *(.text .text.*)
    }

    .bss : { 
        *(.bss .bss.*)
        *(.sbss .sbss.*)
    }

    .data : { 
        *(.data .rodata) 
        *(.sdata .sdata.*)
    }
    
    /DISCARD/ : { *(.eh_*) }
}