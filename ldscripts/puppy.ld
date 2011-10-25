ENTRY(_reset)
SECTIONS
{
 . = 0x80008000;
 .startup . : { init/startup.o(.text) }
 .text : { *(.text) }
 .data : { *(.data) }
 .bss : { *(.bss) }
 . = . + 0x1000; /* 4kB of stack memory */
 stack_top = .;

 /* To make arm-none-linux-gnueabi-gcc shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
