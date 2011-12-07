/*
 * Test Context Switching
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

#ifdef TEST_CSWITCH

#include "types.h"
#include "omap3/early_uart3.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#define MODULE "test_cswitch"
#include "debug/log.h"

#define TEST_ASID

#ifdef TEST_ASID
#define SW(x,i) do {                            \
    vmm_activate_pagetable(x);                  \
    data_sync_barrier();                        \
    arm_mmu_set_context_id(i);                  \
    prefetch_flush();                           \
  } while (0)
#else
#define SW(x,i) do {                            \
    vmm_activate_pagetable(x);                  \
    arm_mmu_flush_tlb();                        \
    arm_cache_invl_instr();                     \
    data_sync_barrier();                        \
    prefetch_flush();                           \
  } while (0)
#endif

static status alloc_pt(pagetable_t *pt)
{
  /* get four contiguous naturally aligned physical pages */
  if(physical_alloc_pages(4, 4, &pt->ptpaddr) != OK) {
    DLOG(1, "physical_alloc_pages for pagetable failed.\n");
    return ENOSPACE;
  }
  region_t tmp = { pt->ptpaddr, NULL, &kernel_l2pt, 4, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
  /* map to kernel virtual memory */
  if(vmm_map_region_find_vstart(&tmp) != OK) {
    DLOG(1, "vmm_map_region_find_vstart for pagetable failed.\n");
    return ENOSPACE;
  }
  pt->ptvaddr = tmp.vstart;
  return OK;
}

status test_cswitch(void)
{
  pagetable_t usr1_l1pt = { (void *) 0x00000000, 0, NULL, &usr1_l1pt, PT_MASTER, 0 };
  pagetable_t usr2_l1pt = { (void *) 0x00000000, 0, NULL, &usr2_l1pt, PT_MASTER, 0 };
  u32 t1, t2;
  region_t r1 = { 0x84000000, (void *) 0x100000, &usr1_l1pt, 1, 20, R_C | R_B, R_NG, R_RW };
  region_t r2 = { 0x84100000, (void *) 0x100000, &usr2_l1pt, 1, 20, R_C | R_B, R_NG, R_RW };

  alloc_pt(&usr1_l1pt);
  alloc_pt(&usr2_l1pt);

  printf_uart3("usr1_l1table phys=%#x usr2_l1table phys=%#x\n",
               usr1_l1pt.ptpaddr, usr2_l1pt.ptpaddr);
  printf_uart3("usr1_l1table virt=%#x usr2_l1table virt=%#x\n",
               usr1_l1pt.ptvaddr, usr2_l1pt.ptvaddr);

  SW(&usr1_l1pt,1);

  if(vmm_map_region(&r1) == OK && vmm_map_region(&r2) == OK) {
    u32 *p = (u32 *) r1.vstart;
    u32 p1, p2;
    printf_uart3("r1.vstart=%#x\n", r1.vstart);

    r1.pt->ptvaddr[((u32) r1.vstart >> 20) & 0xFFF] |= BIT(17); /* not global */
    r2.pt->ptvaddr[((u32) r2.vstart >> 20) & 0xFFF] |= BIT(17); /* not global */

    arm_cache_clean_invl_data_mva_poc(&r1.pt->ptvaddr[((u32) r1.vstart >> 20) & 0xFFF]);
    arm_cache_clean_invl_data_mva_poc(&r2.pt->ptvaddr[((u32) r2.vstart >> 20) & 0xFFF]);

    printf_uart3("r1.pt->ptvaddr[%#x]=%#x\n",
                 ((u32) r1.vstart >> 20) & 0xFFF,
                 r1.pt->ptvaddr[((u32) r1.vstart >> 20) & 0xFFF]);

    SW(&usr1_l1pt,1);
    *p = 1;
    SW(&usr2_l1pt,2);
    *p = 2;
    SW(&usr1_l1pt,1);

    p1 = *p;
    t1 = arm_read_cycle_counter();
    SW(&usr2_l1pt,2);
    t2 = arm_read_cycle_counter();
    p2 = *p;
    printf_uart3("p1=%#x p2=%#x\n", p1, p2);
    printf_uart3("t2-t1=%d\n", t2-t1);
    if (p1 == 1 && p2 == 2)
      return OK;
    else
      return EINVALID;
  }
  return EINVALID;
  /* free temp regions */
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
