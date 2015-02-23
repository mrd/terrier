        .globl _start
        .section .entry
_start: CMP     r13, #0         /* r13==0 iff initial entry */
        LDREQ   sp, =_stack
        LDREQ   pc, =main

        LDR     r0, [r13]
        MSR     cpsr_sf, r0     /* restore cpsr (status, flags) */
        ADD     r13, r13, #8
        LDMIA   r13, {r0-r15}   /* load context, branch to it */

        .section .bss
        .space  8192
_stack:
