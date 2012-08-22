/*
 * Semaphores
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

#ifndef _SMP_SEMAPHORE_H_
#define _SMP_SEMAPHORE_H_
#include "types.h"
#include "status.h"
#include "arm/smp/per-cpu.h"
#include "arm/asm.h"
#include "arm/memory.h"
#include "omap/early_uart3.h"

typedef PACKED_STRUCT(semaphore) { PACKED_FIELD(u32, count); PACKED_FIELD(u32, _pad[7]); } PACKED_END semaphore_t;
#define SEMAPHORE_INIT { 0 }
#define DEFSEMAPHORE(x) ALIGNED(CACHE_LINE, semaphore_t, x) = SEMAPHORE_INIT

/* ARM Synchronization Primitives Development Article 1.3.3 */
static inline status semaphore_down(semaphore_t *sem)
{
  register u32 r, t;
  ASM("1:\n\t"
      "LDREX   %[r], [%[s]]\n\t" /* get count */
      "TEQ     %[r], #0\n\t"     /* if count is zero */
      "WFEEQ\n\t"                /* then wait for signal */
      "BEQ     1b\n\t"           /*      and retry */
      "SUB     %[r], %[r], #1\n\t" /* else decrement temp copy of count */
      "STREX   %[t], %[r], [%[s]]\n\t" /* attempt to store temp copy atomically */
      "TEQ     %[t], #0\n\t"           /* if attempt fails */
      "BNE     1b\n\t"                 /* then retry */
      "DMB\n\t"                        /* memory barrier required before resource usage */
      :[r] "=&r" (r), [t] "=&r" (t):[s] "r" (&sem->count):"cc");
  return OK;
}

static inline status semaphore_up(semaphore_t *sem)
{
  register u32 r, t;
  ASM("1:\n\t"
      "LDREX   %[r], [%[s]]\n\t" /* get count */
      "ADD     %[r], %[r], #1\n\t" /* increment temp copy of count */
      "STREX   %[t], %[r], [%[s]]\n\t" /* attempt to store temp copy atomically */
      "TEQ     %[t], #0\n\t"           /* if attempt fails */
      "BNE     1b\n\t"                 /* then retry */
      "TEQ     %[r], #1\n\t"           /* else if count was zero and now is one, then: */
      "DSB\n\t"                        /* (sync/memory barrier required before SEV) */
      "SEVEQ"                          /* use SEV to signal waiting processors, if any */
      :[r] "=&r" (r), [t] "=&r" (t):[s] "r" (&sem->count):"cc");

  return OK;
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
