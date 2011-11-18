/*
 * ARM memory management
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

#ifndef _ARM_MEMORY_H_
#define _ARM_MEMORY_H_

#include "types.h"

#define PAGE_SIZE_LOG2 12
#define PAGE_SIZE (1<<PAGE_SIZE_LOG2)

#define CTRL_MMU    BIT(0)
#define CTRL_DCACHE BIT(2)
#define CTRL_ICACHE BIT(12)
#define CTRL_HIGHVT BIT(13)
#define CTRL_XP     BIT(23)

static inline void arm_ctrl(u32 set, u32 mask)
{
  u32 cr;
  ASM("MRC p15, #0, %0, c1, c0, #0":"=r"(cr));
  cr &= ~mask;
  cr |= set;
  ASM("MCR p15, #0, %0, c1, c0, #0"::"r"(cr));
}

/* Translation Table Base Control Register */

/* A translation table base register is selected in the following fashion:
 *
 * If N is set to 0, always use Translation Table Base Register
 * 0. This is the default case at reset. It is backwards compatible
 * with ARMv5 and earlier processors.
 *
 * If N is set to a value greater than 0, and bits [31:32-N] of the VA
 * are all zeros, use Translation Table Base Register 0. Otherwise,
 * use Translation Table Base Register 1. N must be in the range
 * 0-7. */

#define MMU_TTBCR_N    BITMASK(0,3)
#define MMU_TTBCR_PD0  BIT(4)
#define MMU_TTBCR_PD1  BIT(5)
static inline void arm_mmu_ttbcr(u32 set, u32 mask)
{
  u32 cr;
  ASM("MRC p15, 0, %0, c2, c0, 2":"=r"(cr));
  cr &= ~mask;
  cr |= set;
  ASM("MCR p15, 0, %0, c2, c0, 2"::"r"(cr));
}

#define MMU_DOMAIN_NO_ACCESS 0x0
#define MMU_DOMAIN_CLIENT    0x1
#define MMU_DOMAIN_MANAGER   0x3

static inline void arm_mmu_domain_access_ctrl(u32 set, u32 mask)
{
  u32 cr;
  ASM("MRC p15, #0, %0, c3, c0, #0":"=r"(cr));
  cr &= ~mask;
  cr |= set;
  ASM("MCR p15, #0, %0, c3, c0, #0"::"r"(cr));
}

static inline void arm_memset_log2(void *addr, u32 v, u32 exp)
{
  u32 count;

  if (exp < 3) {
    u8 *ptr = (u8 *) addr;

    count = 1 << exp;

    for (; count > 0; --count)
      *ptr++ = (u8) v;

  } else {
    u32 *ptr = (u32 *) addr;
    u32 v4;

    v &= 0x000000ff;
    v4 = v;
    v4 |= (v << 8);
    v4 |= (v << 16);
    v4 |= (v << 24);
        
    count = 1 << (exp - 2);

    for(; count > 0; --count)
      *ptr++ = v;
  }
}

/* TTB0 -- process-specific virtual mappings */
/* TTB1 -- OS or I/O specific virtual mappings */

/* Set translation table base address for MMU. ttb must be 16kB aligned. */
static inline void arm_mmu_set_ttb0(physaddr ttbp)
{
  u32 ttb = (u32) ttbp;
  ttb &= 0xffffc000;
  ASM("MCR p15, #0, %0, c2, c0, #0"::"r"(ttb));
}

/* Get translation table base address for MMU. */
static inline physaddr arm_mmu_get_ttb0(void *ttbp)
{
  u32 ttb;
  ASM("MRC p15, #0, %0, c2, c0, #0":"=r"(ttb));
  return ttb;
}

/* Set translation table base address for MMU. ttb must be 16kB aligned. */
static inline void arm_mmu_set_ttb1(physaddr ttbp)
{
  u32 ttb = (u32) ttbp;
  ttb &= 0xffffc000;
  ASM("MCR p15, #0, %0, c2, c0, #1"::"r"(ttb));
}

/* Get translation table base address for MMU. */
static inline physaddr arm_mmu_get_ttb1(void *ttbp)
{
  u32 ttb;
  ASM("MRC p15, #0, %0, c2, c0, #1":"=r"(ttb));
  return ttb;
}

/* Flush the entire translation lookaside buffer. */
static inline void arm_mmu_flush_tlb(void)
{
  ASM("MCR p15, #0, %0, c8, c7, #0"::"r"(0));
}

/* MVA - Modified Virtual Address (64-byte aligned) (ARMv6+ accepts any address)
 * SAW - Set and Way
 * 
 * PoC - Point of Coherency - The time when the imposition of any more
 * cache becomes transparent for instruction, data, and translation
 * table walk accesses to that address by any processor in the system.
 * 
 * PoU - Point of Unification - The time when the instruction and data
 * caches, and the TLB translation table walks have merged for a
 * uniprocessor system.
 *
 * Invalidate - Line no longer available for hits until re-assigned.
 * Clean - if write-back enabled then flush writes to memory
 */

/* Invalidate data (or unified) cache by MVA to PoC */
static inline void arm_cache_invl_data_mva_poc(void *vaddr)
{
  ASM("MCR p15, #0, %0, c7, c6, #1"::"r"(vaddr));
}

/* Clean data or unified cache by MVA to PoC. */
static inline void arm_cache_clean_data_mva_poc(void *vaddr)
{
  ASM("MCR p15, #0, %0, c7, c10, #1"::"r"(vaddr));
}

/* Clean and invalidate data (or unified) cache by MVA to PoC */
static inline void arm_cache_clean_invl_data_mva_poc(void *vaddr)
{
  ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(vaddr));
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
