OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(main)
SECTIONS
{
 PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x00008000));
 . = SEGMENT_START("text-segment", 0x00008000) + SIZEOF_HEADERS;
 .text : { *(.text) PROVIDE(_etext = .); }
 .rodata : { *(.rodata) }
 .data : { *(.data) }
 .bss : { *(.bss) }
 /* To make arm-none-linux-gnueabi-ld shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
