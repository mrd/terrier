/*
 * Per-CPU Variables
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

#ifndef _SMP_PER_CPU_H_
#define _SMP_PER_CPU_H_
#include "types.h"
#include "status.h"
#include "debug/cassert.h"
#include "arm/asm.h"

#define _PER_CPU_SECTION ".percpu"
#define _PER_CPU_CTOR_SECTION _PER_CPU_SECTION".ctor"

#define PER_CPU_ATTR __attribute__((section(_PER_CPU_SECTION)))
#define PER_CPU_CTOR_ATTR __attribute__((section(_PER_CPU_CTOR_SECTION)))

/* ************************************************** */
/* API */

/* Define a per-CPU variable */
#define DEF_PER_CPU(type,var) \
  CASSERT(sizeof(type)==4,_per_cpu_var_size_must_be_4); PER_CPU_ATTR type var

/* Define an initialization function for a per-CPU variable */
#define INIT_PER_CPU(var)                               \
  void __##var##_ctor_func (void);                      \
  PER_CPU_CTOR_ATTR void (*__##var##_ctor_ptr) (void) = \
    __##var##_ctor_func;                                \
  void __##var##_ctor_func (void)

extern u32 *_per_cpu_spaces[];

#define cpu_index() GETBITS(arm_multiprocessor_affinity(),0,2)

/* read/write macros */
#define cpu_read(x) _per_cpu_spaces[cpu_index()][(u32) &x]
#define cpu_write(x,v) _per_cpu_spaces[cpu_index()][(u32) &x] = v;

status per_cpu_init(void);

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
