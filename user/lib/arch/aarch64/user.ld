OUTPUT_ARCH(aarch64)
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

    .tdata : {
        __tdata_start = .;
        *(.tdata .tdata.*)
        __tdata_end = .;
    }

    .tbss : {
        __tbss_start = .;
        *(.tbss .tbss.*)
        __tbss_end = .;
    }
    
    /DISCARD/ : { *(.eh_*) }
}