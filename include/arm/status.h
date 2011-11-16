/*
 * ARM status register
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

#ifndef _ARM_STATUS_H_
#define _ARM_STATUS_H_

#define MODE_USR 0x10           /* User */
#define MODE_FIQ 0x11           /* Fast Interrupt Request */
#define MODE_IRQ 0x12           /* Interrupt Request */
#define MODE_SVC 0x13           /* Supervisor */
#define MODE_ABT 0x17           /* Abort */
#define MODE_UND 0x1b           /* Undefined instruction */
#define MODE_SYS 0x1f           /* System */

#define MASK_IRQ 0x80
#define MASK_FIQ 0x40
#define MASK_ALL 0xc0

#ifndef __ASSEMBLER__
static inline u32 arm_cpsr_c(u32 set, u32 mask)
{
  u32 sr;
  ASM("MRS %0, cpsr":"=r"(sr));
  sr &= ~mask;
  sr |= set;
  ASM("MSR cpsr_c, %0"::"r"(sr));
  return sr;
}

static inline u32 disable_interrupts(void)
{
  return arm_cpsr_c(MASK_ALL, MASK_ALL);
}

static inline u32 enable_interrupts(void)
{
  return arm_cpsr_c(0, MASK_ALL);
}

static inline u32 enable_IRQ(void)
{
  return arm_cpsr_c(0, MASK_IRQ);
}

static inline u32 enable_FIQ(void)
{
  return arm_cpsr_c(0, MASK_FIQ);
}
#endif

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
