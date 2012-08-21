/*
 * Interrupt handling
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

#ifndef _INTR_INTERRUPTS_H_
#define _INTR_INTERRUPTS_H_

#include "types.h"

typedef void (*irq_handler_t)(u32 activeirq);

void intr_init(void);
status intc_set_intid_handler(u32 intid, void (*handler)(u32));
status intc_set_irq_handler(u32 irq_num, void (*handler)(u32));
status intc_mask_irq(u32 irq_num);
status intc_unmask_irq(u32 irq_num);
status intc_mask_intid(u32);
status intc_unmask_intid(u32);
status intc_set_priority(u32 irq_num, u32 prio);
u32 intc_get_targets(u32 irq_num);
status intc_set_targets(u32 irq_num, u32 targets);
status intc_set_int_type(u32 irq_num, u32 is_edge);
u32 intc_get_running_priority(void);
u32 intc_get_priority_mask(void);
status intc_set_priority_mask(u32 prio);

static inline void arm_set_vector_base_address(u32 addr)
{
  ASM("MCR p15, 0, %0, c12, c0, 0"::"r"(addr));
}

static inline u32 arm_get_vector_base_address(void)
{
  u32 addr;
  ASM("MCR p15, 0, %0, c12, c0, 0":"=r"(addr));
  return addr;
}

#endif

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
