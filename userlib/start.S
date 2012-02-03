        .globl _start
_start: ADR     sp, _stack
        LDR     pc, =main
        .space  128
_stack: