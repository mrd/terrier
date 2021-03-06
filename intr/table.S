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

#include<arm/status.h>

/* Macro to access a per-cpu variable */
#define asm_cpu_pointer(x)                                                    \
        MOVW    r12, #:lower16:_per_cpu_spaces; /* r12 = _per_cpu_spaces */   \
        MOVT    r12, #:upper16:_per_cpu_spaces;                               \
        MRC     p15,0,r0,c0,c0,5;  /* r0 = cpu index */                       \
        AND     r0, r0, #7;        /* mask first 3 bits */                    \
        LDR     r12, [r12, r0, LSL #2]; /* r12 = _per_cpu_spaces[r0] */       \
        MOVW    r0, #:lower16:x;                                              \
        MOVT    r0, #:upper16:x;                                              \
        ADD     r12, r12, r0 /* r12 = &_per_cpu_spaces[old_r0][newer_r0] */

/* Beginning of vector table: ARM processor sets PC to entry in vector
 * table when encountering an exception.  */

        .globl _vector_table
        .align 8
_vector_table:
        LDR     pc, _vector_table_data+0
        LDR     pc, _vector_table_data+4
        B       _swi_stub
        B       _prefetch_abort_stub
        B       _data_abort_stub
        NOP
        B       _irq_stub
        LDR     pc, _vector_table_data+24

/* ARM architectural reference manual B9.3.17 STM (User registers):

 * In a PL1 mode other than System mode, Store Multiple (user registers) stores multiple User mode registers to
consecutive memory locations using an address from a base register. The processor reads the base register value
normally, using the current mode to determine the correct Banked version of the register. This instruction cannot
writeback to the base register. */

/* Therefore, "STMFD r13!, {r0-r15}^" is not allowed. Go figure. */

_prefetch_abort_stub:
        STMFD   r13, {r0-r15}^            /* save usr registers on stack for examination */
        SUB     r13, r13, #64
        LDR     pc, _vector_table_data+12 /* far call, don't expect return at this time */

_data_abort_stub:
        STMFD   r13, {r0-r15}^            /* save usr registers on stack for examination */
        SUB     r13, r13, #64
        LDR     pc, _vector_table_data+16 /* far call, don't expect return at this time */

_swi_stub:
        /* in SVC mode */
        SRSDB   sp!, #MODE_SVC            /* save {r14, spsr} on stack */
        STMFD   r13!, {r0-r3,r12}         /* put caller-save regs on stack */
        MOV     r0, lr                    /* set interrupt return address as argument */
        ADR     lr, _far_call_return      /* set return address */
        LDR     pc, _vector_table_data+8  /* far call */

_irq_stub:
        /* in IRQ mode */
        SUB     lr, lr, #4
        SRSDB   sp!, #MODE_SVC            /* save {r14, spsr} on SVC mode stack */
        CPS     #MODE_SVC                 /* change to SVC mode */
        STMFD   r13!, {r0-r3,r12}         /* put caller-save regs on stack */
        ADR     lr, _far_call_return      /* set return address */
        LDR     pc, _vector_table_data+20 /* far call */

        /* stack layout at this point:
         *       ____________
         * 0x18  | spsr_irq | SRS-saved
         * 0x14  | r14_irq  | ___________
         * 0x10  | r12      |
         * 0x0c  | r3       |
         * 0x08  | r2       | caller-save
         * 0x04  | r1       |
         * 0x00  | r0       |
         *       ------------
         */
_far_call_return:
        LDR     lr, [r13, #0x14]  /* lr = interrupt return address */

        /* _prev_context == current or actual previous context */ \
        asm_cpu_pointer(_prev_context)

        /* r12 = &_prev_context */
        LDR     r12, [r12]        /* r12 = *r12 */

        LDR     r1, [r12, #0x4c]  /* r1 = process_t.end_entry */
        CMP     r1, lr
        BGT     1f                /* skip save if end_entry > return address */

        LDMFD   r13, {r0-r3}      /* restore caller-save {r0-r3} */
        ADD     r12, r12, #8      /* r12 = &_prev_context->usr.r[0] */
        STMIA   r12, {r0-r11}     /* save registers {r0-r11} */
        LDR     r0, [r13, #0x18]  /* restore caller-save spsr */
        STMDB   r12, {r0,lr}      /* save status and interrupt return address */
        MOV     r0, r12           /* r0 = r12 = _prev_context */
        LDR     r12, [r13, #0x10] /* restore caller-save r12 */
        ADD     r0, r0, #0x30     /* r0 = &_prev_context->usr.r[12] */
        STMIA   r0, {r12-r14}^    /* save user/sys registers {r12,r13_usr,r14_usr} */
        STR     lr, [r0,#0xc]     /* save interrupt return address as r15_usr */

1:
        ADD     r13, r13, #0x1c   /* unwind stack */

        /* now setup the context for the next process to use */
        asm_cpu_pointer(_next_context)

        /* r12 = &_next_context */
        LDMIA   r12, {r13}^                /* r13_usr = *r12 */
        LDMIA   r12, {r12}                 /* r12 = *r12 */
        LDR     r0, [r12]                  /* r0 = spsr */
        MSR     spsr_cx, r0                /* store spsr (control, extension) */
        LDR     lr, [r12, #0x48]           /* lr = entry point (process_t.entry)
                                            * assumes ctxt is embedded in process_t */

        asm_cpu_pointer(irq_status_table)
        LDMIA   r12, {r12}^                /* r12_user = *r12, just in case of FIQ mode  */

        CLREX
        MOVS    pc, lr                     /* jump to entry point, load SPSR */

_vector_table_data:
        .word   _handle_reset
        .word   _handle_undefined_instruction
        .word   _handle_swi
        .word   _handle_prefetch_abort
        .word   _handle_data_abort
        .word   _handle_irq
        .word   _handle_fiq

        .globl  _vector_table_size
_vector_table_size:
        .word   _vector_table_size - _vector_table
