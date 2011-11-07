/*
 * Begin initialization in C.
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

#include "types.h"
#include "omap3/early_uart3.h"
#include "arm/memory.h"

ALIGNED(0x4000, static u32, l1table[4096]);

void build_identity_l1table(void)
{
  u32 i, c, b;
  for(i=0; i<4096; i++) {
    if (0x800 <= i && i < 0xc00)
      c = b = 1;
    else
      c = b = 0;
    l1table[i] =
      (i << 20) |      /* base address */
      0x2  |           /* section entry */
      0x10 |           /* reserved bit */
      (0x0 << 5) |     /* domain */
      (0x3 << 10) |    /* access perm. */
      (c<<3) |         /* cached */
      (b<<2);          /* buffered/writeback */
  }
}

void init_mmu(void)
{
  build_identity_l1table();
  arm_mmu_flush_tlb();
  arm_mmu_domain_access_ctrl(~0, ~0); /* set all domains = MANAGER */
  arm_mmu_set_ttb(l1table);
  arm_mmu_ctrl(MMU_CTRL_MMU | MMU_CTRL_DCACHE | MMU_CTRL_ICACHE,
               MMU_CTRL_MMU | MMU_CTRL_DCACHE | MMU_CTRL_ICACHE);
}

void c_entry()
{
  void identify_device(void);
  void physical_init(void);

  reset_uart3();
  identify_device();

  physical_init();

  init_mmu();

  print_uart3("Hello world!\n");
}

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
