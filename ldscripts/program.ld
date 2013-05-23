OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
 PROVIDE (__executable_start = SEGMENT_START("text-segment", 0x00000000));
 . = SEGMENT_START("text-segment", 0x00000000);
 .text : { *(.entry); _end_entry = .; *(.text*) PROVIDE(_etext = .); }
 .rodata : { *(.rodata) }
 .data : { *(.data) }
 .bss : { *(.bss) }
  /* align device memory to page */
 .device : ALIGN(0x1000) { *(.device) }
 . = ALIGN(4096);               /* end on page boundary */
 /* To make arm-none-linux-gnueabi-ld shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
