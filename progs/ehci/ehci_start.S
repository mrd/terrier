        .globl _start
        .section .entry
_start: CMP     r13, #0         /* r13==0 iff initial entry */
        LDREQ   sp, =_stack
        LDREQ   pc, =main

        LDR     r11, =irq_state /* check IRQ state for value of 3 */
        LDR     r1, [r11]
        CMP     r1, #3
        BEQ     3f              /* state 3 -- restore saved context */

        LDRB    r0, [r12, #77]  /* r12 is irq status table, IRQ77 is EHCI */
        CMP     r0, #1
        BNE     2f              /* if no IRQ then nevermind this stuff */

        CMP     r1, #1          /* check IRQ state for 0, 1, or 2 */
        BEQ     1f              /* if = to 1 */
        BGT     2f              /* if > than 1 and = 2 */

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
        /* control flow ends */
3:
        /* If IRQ state == 3 then restore saved context */
        /* assumption: r13 == pointer to kernel saved context */
        LDR     r1, =saved_context
        LDMIA   r1!, {r2-r10}
        STMIA   r13!, {r2-r10}
        LDMIA   r1!, {r2-r10}
        STMIA   r13!, {r2-r10}
        SUB     r13, r13, #0x48

        MOV     r0, #0
        STR     r0, [r11]       /* set IRQ state to 0 */

        /* fall-thru to normal context loading */
2:
        LDR     r0, [r13]       /* r13 is pointer to saved context */
        MSR     cpsr_sf, r0     /* restore cpsr (status, flags) */
        ADD     r13, r13, #8
        LDMIA   r13, {r0-r15}   /* load context, branch to it */
        /* control flow ends */

        .section .bss
        .space 2048
_stack:
        .space 2048
_irq_stack: