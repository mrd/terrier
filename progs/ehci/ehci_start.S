        .globl _start
        .section .entry
_start: CMP     r13, #0         /* r13==0 iff initial entry */
        LDREQ   sp, =_stack
        LDREQ   pc, =main

        LDRB    r0, [r12, #77]  /* r12 is irq status table, IRQ77 is EHCI */
        CMP     r0, #1
        BNE     2f

        LDR     r11, =irq_state /* check IRQ state */
        LDR     r0, [r11]
        CMP     r0, #1
        BEQ     1f
        BGT     2f

        /* Reentrant because kernel will not overwrite r13-context
         * while pc is still in this section. */

        /* If IRQ state == 0 then save context in our own space */
        LDR     r1, =saved_context
        LDMIA   r13!, {r2-r10}
        STMIA   r1!, {r2-r10}
        LDMIA   r13!, {r2-r10}
        STMIA   r1!, {r2-r10}

        MOV     r0, #1
        STR     r0, [r11]       /* set IRQ state to 1 */

        /* If IRQ state == 1 then load fresh IRQ handling context */
1:
        MOV     r0, #0
        MSR     cpsr_sf, r0
        LDR     r0, =ehci_irq_handler
        LDR     sp, =_irq_stack
        MOV     pc, r0          /* C code will set IRQ state to 2 */

2:
        LDR     r0, [r13]       /* r13 is pointer to saved context */
        MSR     cpsr_sf, r0     /* restore cpsr (status, flags) */
        ADD     r13, r13, #8
        LDMIA   r13, {r0-r15}   /* load context, branch to it */

        .section .bss
        .space 1024
_stack:
        .space 1024
_irq_stack:
