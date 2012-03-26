        .globl _start
        .section .text
_start: CMP     r13, #0
        BEQ     _begin

        LDMIA   r13!, {r0,lr}   /* load status register, return address */
        MSR     spsr_cxsf, r0   /* prep saved process status register */
        LDMIA   r13, {r0-r14}^  /* load user registers (incl. r13_usr, r14_usr) */

        /* and return to program */
        LDR     r13, =_svc_stack_top
        LDR     r13, [r13]
        MOVS    pc, lr                     /* jump to program, restore spsr */

_begin:
        LDR     sp, =_stack
        LDR     pc, =main

        .section .data
_svc_stack_top:
        .word   0xc001e000

        .section .bss
        .space  1024
_stack:
