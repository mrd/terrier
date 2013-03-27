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
#include "arm/asm.h"
#include "arm/memory-basics.h"

#define CTRL_MMU    BIT(0)
#define CTRL_ALIGN  BIT(1)
#define CTRL_DCACHE BIT(2)
#define CTRL_ICACHE BIT(12)
#define CTRL_HIGHVT BIT(13)

static inline void arm_ctrl(u32 set, u32 mask)
{
  u32 cr;
  ASM("MRC p15, #0, %0, c1, c0, #0":"=r"(cr));
  cr &= ~mask;
  cr |= set;
  ASM("MCR p15, #0, %0, c1, c0, #0"::"r"(cr));
}

static inline u32 arm_config_base_address(void)
{
  u32 r;
  ASM("MRC p15,4,%0,c15,c0,0":"=r"(r));
  return r;
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

static inline u32 memcpy(void *dest, void *src, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  u8 *src8 = (u8 *) src;
  for(i=0;i<count;i++)
    dest8[i] = src8[i];
  return i;
}

static inline u32 memset(void *dest, u32 byte, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  for(i=0;i<count;i++)
    dest8[i] = (u8) byte;
  return i;
}

/* TTB0 -- process-specific virtual mappings */
/* TTB1 -- OS or I/O specific virtual mappings */

/* Set translation table base address for MMU. ttb must be 16kB aligned. */
static inline void arm_mmu_set_ttb0(physaddr ttbp)
{
  u32 ttb = (u32) ttbp;
  ttb &= 0xffffc000;
  ttb |= 0x18;
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
  ttb |= 0x18;
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

/* Invalidate entire instruction cache to PoU. Also flush branch
 * target cache. */
static inline void arm_cache_invl_instr(void)
{
  ASM("MCR p15, #0, %0, c7, c5, #0"::"r"(0));
}

/* Invalidate entire branch predictor array. */
static inline void arm_cache_invl_branch_pred_array(void)
{
  ASM("MCR p15, #0, %0, c7, c5, #6"::"r"(0));
}

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

#define DEF_CACHE_OP_ALL_SETWAY(name,op)                                \
  static inline void name(void)                                         \
  {                                                                     \
  /* Loop through set/way */                                            \
  u32 clidr, l;                                                         \
  ASM("MRC p15, #1, %0, c0, c0, #1":"=r"(clidr));                       \
  for(l=0;l<7;l++) {                                                    \
    u32 ctype = GETBITS(clidr,l+l+l,3), ccsidr, numsets, assocs;        \
    if(ctype >= 2) {                                                    \
      /* CSSELR: Select current CCSIDR by Level and Type=0 (Data or Unified) */ \
      ASM("MCR p15, #2, %0, c0, c0, #0"::"r"((l<<1)|0));                \
      /* CCSIDR: Cache Size ID registers */                             \
      ASM("MRC p15, #1, %0, c0, c0, #0":"=r"(ccsidr));                  \
      assocs = GETBITS(ccsidr,3,10)+1;                                  \
      numsets = GETBITS(ccsidr,13,15)+1;                                \
      u32 A = ceiling_log2(assocs);                                     \
      u32 S = ceiling_log2(numsets);                                    \
      u32 L = GETBITS(ccsidr, 0, 3)+4;                                  \
      u32 s, w;                                                         \
      for(w=0;w<assocs;w++) {                                           \
        for(s=0;s<numsets;s++) {                                        \
          u32 Rt = SETBITS(l,1,3) | SETBITS(s,L,S) | SETBITS(w,32-A,A); \
          ASM(op::"r"(Rt));                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
  }                                                                     \
}

/* Clean and invalidate entire data cache */
DEF_CACHE_OP_ALL_SETWAY(arm_cache_clean_invl_data,"MCR p15, #0, %0, c7, c14, #2");
/* Clean entire data cache. */
DEF_CACHE_OP_ALL_SETWAY(arm_cache_clean_data,"MCR p15, #0, %0, c7, c10, #2");
/* Invalidate entire data cache */
DEF_CACHE_OP_ALL_SETWAY(arm_cache_invl_data,"MCR p15, #0, %0, c7, c6, #2");

/* The least significant byte is the Address Space ID. The remainder
 * forms a general-purpose process ID. There can be many process IDs
 * per ASID. The ASID is used for tagged TLB entries. */
static inline void arm_mmu_set_context_id(u32 cid)
{
  ASM("MCR p15, #0, %0, c13, c0, #1"::"r"(cid));
}

static inline u32 arm_mmu_data_fault_status(void)
{
  u32 stat;
  ASM("MRC p15, #0, %0, c5, c0, #0":"=r"(stat));
  return stat;
}

static inline u32 arm_mmu_instr_fault_status(void)
{
  u32 stat;
  ASM("MRC p15, #0, %0, c5, c0, #1":"=r"(stat));
  return stat;
}

static inline u32 arm_mmu_data_fault_addr(void)
{
  u32 stat;
  ASM("MRC p15, #0, %0, c6, c0, #0":"=r"(stat));
  return stat;
}

static inline u32 arm_mmu_watchpoint_fault_addr(void)
{
  u32 stat;
  ASM("MRC p15, #0, %0, c6, c0, #1":"=r"(stat));
  return stat;
}

static inline u32 arm_mmu_instr_fault_addr(void)
{
  u32 stat;
  ASM("MRC p15, #0, %0, c6, c0, #2":"=r"(stat));
  return stat;
}

#define arm_mmu_va_to_pa(vaddr, op2, res)                       \
  ASM("MCR p15,0,%1,c7,c8,%2\n"                                 \
      "MRC p15,0,%0,c7,c4,0":"=r"(res):"r"(vaddr),"I"(op2))

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
