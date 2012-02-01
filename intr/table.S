/*
 * ARM Interrupt/Exception Vector Table
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2011 Matthew Danish.
 *
 * All rights reserved. Please see enclosed LICENSE file for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Be careful: actual virtual starting address will be 0xFFFF0000
 * during operation. */

        .globl _vector_table
        .align 8
_vector_table:
        LDR     pc, _vector_table_data+0
        LDR     pc, _vector_table_data+4
        LDR     pc, _vector_table_data+8
        LDR     pc, _vector_table_data+12
        LDR     pc, _vector_table_data+16
        NOP
        B       _irq_stub
        LDR     pc, _vector_table_data+24

_irq_stub:
        STMFD   sp!, {r0-r3,r12,r14}      /* put caller-save regs on stack */
        MOV     lr, pc                    /* lr = pc + 8 */
        LDR     pc, _vector_table_data+20 /* far call */
        LDMFD   sp!, {r0-r3,r12,r14}      /* restore caller-save regs */
        /* check if context switch needed. r13 is available. */
        LDR     r13, _vector_table_data+28 /* r13 = &_prev_process */
        LDR     r13, [r13]                 /* r13 = *r13 */
        CMP     r13, #0

        /* prev == NULL, jump directly to userspace */
        LDREQ   r13, _vector_table_data+36 /* r13 = &irq_stack_top */
        SUBEQS  pc, lr, #4                 /* jump to userspace */

        /* prev != NULL implies context switch: now save old context */
        ADD     r13, r13, #8
        STMIA   r13, {r0-r14}^ /* store user registers (incl. r13_usr, r14_usr) */
        MRS     r0, spsr
        STMDB   r13, {r0,lr}   /* save status register, return address */

        /* now load the next process context */
        LDR     r13, _vector_table_data+32 /* r13 = &_next_process */
        LDR     r13, [r13]                 /* r13 = *r13 */
        ADD     r13, r13, #8
        LDMDB   r13, {r0,lr}    /* load status register, return address */
        MSR     spsr_cxsf, r0   /* prep saved process status register */
        LDMIA   r13, {r0-r14}^  /* load user registers (incl. r13_usr, r14_usr) */

        /* and return to userspace */
        LDR     r13, _vector_table_data+36 /* r13 = &irq_stack_top */
        SUBS    pc, lr, #4                 /* jump to userspace */

_vector_table_data:
        .word   _handle_reset
        .word   _handle_undefined_instruction
        .word   _handle_swi
        .word   _handle_prefetch_abort
        .word   _handle_data_abort
        .word   _handle_irq
        .word   _handle_fiq
        .word   _prev_process
        .word   _next_process
        .word   irq_stack_top

        .globl  _vector_table_size
_vector_table_size:
        .word   _vector_table_size - _vector_table
