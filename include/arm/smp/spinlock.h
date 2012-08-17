/*
 * Spinlocks
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

#ifndef _SMP_SPINLOCK_H_
#define _SMP_SPINLOCK_H_
#include "types.h"
#include "status.h"
#include "arm/smp/per-cpu.h"
#include "arm/asm.h"
#include "arm/memory.h"

typedef PACKED_STRUCT(spinlock) { PACKED_FIELD(u32, flag); PACKED_FIELD(u32, _pad[7]); } PACKED_END spinlock_t;
#define SPINLOCK_INIT { 0 }
#define DEFSPINLOCK(x) ALIGNED(CACHE_LINE, spinlock_t, x) = SPINLOCK_INIT;

static inline status spinlock_lock(spinlock_t *lock)
{
#ifdef __GNUC__
  u32 i = cpu_index() + 1, j;
  while ((j = __sync_val_compare_and_swap(&lock->flag, 0, i))) {
    if(j == i) {
      DO_WITH_REGS(regs) { printf_uart3_regs(regs); } END_WITH_REGS;
      printf_uart3("cpu%d: lock=%#x held_by=%d\n", cpu_index(), lock, j - 1);
      early_panic("recursive spin lock");
    }
  }
  return OK;
#else
#error "spinlocks unimplemented for this compiler"
  return EUNDEFINED;
#endif
}

static inline void spinlock_unlock(spinlock_t *lock)
{
#ifdef __GNUC__
  __sync_lock_release(&lock->flag);
#else
#error "spinlocks unimplemented for this compiler"
#endif
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
