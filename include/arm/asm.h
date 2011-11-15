/*
 * ARM assembly routines
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

#ifndef _ARM_ASM_H_
#define _ARM_ASM_H_
#include "types.h"

/* The purpose of the data synchronization barrier operation is to
 * ensure that all outstanding explicit memory transactions complete
 * before any following instructions begin. This ensures that data in
 * memory is up to date before the processor executes any more
 * instructions. */
static inline void data_sync_barrier(void)
{
  asm volatile("MCR p15, #0, %0, c7, c10, #4"::"r"(0));
}

/* The purpose of the data memory barrier operation is to ensure that
 * all outstanding explicit memory transactions complete before any
 * following explicit memory transactions begin. This ensures that
 * data in memory is up to date for any memory transaction that
 * depends on it. */
static inline void data_mem_barrier(void)
{
  asm volatile("MCR p15, #0, %0, c7, c10, #5"::"r"(0));
}

static inline u32 count_leading_zeroes(u32 x)
{
  u32 n;
  asm volatile("CLZ %0, %1":"=r"(n):"r"(x));
  return n;
}

static inline u32 arm_read_cycle_counter(void)
{
  u32 c;
  asm volatile("MRC p15, 0, %0, c15, c12, 1":"=r"(c));
  return c;
}

static inline u32 arm_perfmon_ctrl(u32 set, u32 mask)
{
  u32 cr;
  asm volatile("MRC p15, 0, %0, c15, c12, 0":"=r"(cr));
  cr &= ~mask;
  cr |= set;
  asm volatile("MCR p15, 0, %0, c15, c12, 0"::"r"(cr));
  return cr;
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
