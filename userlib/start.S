        .globl _start
        .section .text
_start: LDR     sp, =_stack
        LDR     pc, =main
        .section .bss
        .space  1024
_stack:
