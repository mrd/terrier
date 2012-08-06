/*
 * Low memory stub - setup page tables for remaining init
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

#ifdef USE_VMM

#include "types.h"
#include "omap/early_uart3.h"
#include "arm/memory.h"

void stub_init(void)
{
  extern void *_l1table_phys, *_kernel_start, *_physical_start;
  u32 *l1table_phys = (u32 *) &_l1table_phys;
  u32 phystart = (u32) &_physical_start, krnstart = (u32) &_kernel_start;

  arm_memset_log2(l1table_phys, 0, 14);

  /* Map stub as identity, and kernel virtual -> physical */
  l1table_phys[phystart >> 20] = (phystart & 0xFFF00000) | 0xc1e;
  l1table_phys[krnstart >> 20] = (phystart & 0xFFF00000) | 0xc1e;
  
  arm_mmu_flush_tlb();
  arm_mmu_domain_access_ctrl(~0, ~0); /* set all domains = MANAGER */

  /* Stub will remain mapped during early initialization. */
  arm_mmu_set_ttb0((physaddr) l1table_phys);
  arm_mmu_set_ttb1((physaddr) l1table_phys);

  /* Enable MMU */
  arm_ctrl(CTRL_MMU | CTRL_DCACHE | CTRL_ICACHE
          ,CTRL_MMU | CTRL_DCACHE | CTRL_ICACHE);
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
