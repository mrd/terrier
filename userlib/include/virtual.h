/*
 * Virtual memory helper functions for userlib
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2014 Matthew Danish.
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

#ifndef _USERLIB_VIRTUAL_H_
#define _USERLIB_VIRTUAL_H_

#include"../../include/types.h"
#include"../../include/status.h"

#ifdef USE_VMM

#ifndef arm_mmu_va_to_pa
#define arm_mmu_va_to_pa(vaddr, op2, res)                       \
  ASM("MCR p15,0,%1,c7,c8,%2\n"                                 \
      "MRC p15,0,%0,c7,c4,0":"=r"(res):"r"(vaddr),"I"(op2))
#endif

static inline status vmm_get_phys_addr(void *vaddr, physaddr *paddr)
{
  u32 addr = (u32) vaddr;
  u32 res;
  arm_mmu_va_to_pa((void *) (addr & 0xFFFFF000), 0, res);
  if (res & 1) return EINVALID;
  *paddr = (res & 0xFFFFF000) + (addr & 0xFFF);
  return OK;
}

#else  /* USE_VMM disabled */

status vmm_get_phys_addr(void *vaddr, physaddr *paddr)
{
  *paddr = (u32) vaddr;
  return OK;
}

#endif  /* if USE_VMM */

#endif
