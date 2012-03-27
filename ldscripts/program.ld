OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
 PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x00008000));
 . = SEGMENT_START("text-segment", 0x00008000) + SIZEOF_HEADERS;
 .text : { *(.entry); _end_entry = .; *(.text) PROVIDE(_etext = .); }
 .rodata : { *(.rodata) }
 .data : { *(.data) }
 .bss : { *(.bss) }
 /* To make arm-none-linux-gnueabi-ld shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
