/*
 * Virtual memory manager
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

#include "mem/virtual.h"
#include "types.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "arm/smp/per-cpu.h"
#define MODULE "vmm"
#include "omap/early_uart3.h"
#include "debug/log.h"
#include "debug/cassert.h"

#ifdef USE_VMM

ALIGNED(0x4000, u32, l1table[4096]);
extern void *_l1table_phys;
pagetable_t l1pt = { (void *) 0x00000000, (physaddr) &_l1table_phys, (u32 *) &l1table, &l1pt, PT_MASTER, 0 };
ALIGNED(0x400, u32, kernel_l2table[256]);
extern void *_kernel_l2table_phys, *_kernel_pages;
pagetable_t kernel_l2pt = { (void *) 0xC0000000, (physaddr) &_kernel_l2table_phys, (u32 *) &kernel_l2table, &l1pt, PT_COARSE, 0 };
region_t kernel_region = { 0x80000000, (void *) 0xC0000000, &kernel_l2pt, (u32) &_kernel_pages + 9, 12, R_C | R_B, R_S, R_PM };

/* ARMv7 Short-descriptor first-level Section entry */
/* Section base physical address [20:31] */
/* Non Secure [19:19] */
/* Non Global [17:17] */
/* Shared [16:16] */
/* Access Permissions [10:11,15] */
/* TEX [12:14] */
/* Domain [5:8] */
/* Execute-Never [4:4] */
/* Cacheable [3:3] */
/* Bufferable [2:2] */
/* Type [1:1] = 1 */
/* Privileged Execute-Never [0:0] */

/* ARMv7 Short-descriptor second-level small page entry */
/* Small page base physical address [12:31] */
/* Non Global [11:11] */
/* Shared [10:10] */
/* Access Permissions [4:5,9] */
/* TEX [6:8] */
/* Cacheable [3:3] */
/* Bufferable [2:2] */
/* Type [1:1] = 1 */
/* Execute-Never [0:0] */

static u32 pt_sizes_log2[] = { 0, 14, 10 };                  /* size of pagetables */
static u32 pt_start_masks[] = { 0, 0xFFFFFFFF, 0x000FFFFF }; /* valid pt virtual start */
static u32 pt_entry_masks[] = { 0, 0x000FFFFF, 0x00000FFF }; /* valid region virtual start */
static u32 pt_entry_sizes_log2[] = { 0, 20, 12 };            /* size that an entry covers */
static u32 pt_type_bits[] = { 0, 0x2, 0x2 };                 /* ARM-specific tag bits */
static u32 pt_ap_offsets[] = { 0, 10, 4 };                   /* Access Perms. offset in entry */
static u32 pt_ap_masks[] = { 0, 0x3, 0x3 };                  /* Access Perms. bitmask */
static u32 pt_dom_offsets[] = { 0, 5, 0 };                   /* Domain offset in entry */
static u32 pt_dom_masks[] = { 0, 0xF, 0 };                   /* Domain bitmask */
static u32 pt_sng_offsets[] = { 0, 16, 10 };                 /* Shared and not-Global bit offsets */

CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_start_masks), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_entry_masks), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_entry_sizes_log2), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_type_bits), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_ap_offsets), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_ap_masks), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_dom_offsets), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_dom_masks), vmm);
CASSERT(sizeof(pt_sizes_log2) == sizeof(pt_sng_offsets), vmm);

#define PT_NUM_TYPES (sizeof(pt_sizes_log2) / sizeof(u32))

/* Set to the pagetable struct corresponding to the current value of
 * the TTB control register. */
DEF_PER_CPU(pagetable_t *, current_base_table);
INIT_PER_CPU(current_base_table) {
  cpu_write(pagetable_t *, current_base_table, &l1pt);
}

/* Assume freshly filled out pagetable_t struct, not yet used. */
status vmm_init_pagetable(pagetable_t *pt)
{
  if(pt->type == PT_FAULT) {
    DLOG(1, "Pagetable has type 'FAULT'\n");
    return EINVALID;
  }
  if(pt->type >= PT_NUM_TYPES) {
    DLOG(1, "Pagetable has invalid type=%d\n", pt->type);
    return EINVALID;
  }
  if((pt->ptpaddr & ((1 << pt_sizes_log2[pt->type]) - 1)) != 0) {
    DLOG(1, "Pagetable physical address (%#x) not aligned correctly (%#x).\n",
         pt->ptpaddr, ((1 << pt_sizes_log2[pt->type]) - 1));
    return EINVALID;
  }
  if(((u32) pt->vstart & pt_start_masks[pt->type]) != 0) {
    DLOG(1, "Pagetable vstart (%#x) does not satisfy mask (%#x).\n",
         pt->vstart, pt_start_masks[pt->type]);
    return EINVALID;
  }
  if(pt->type == PT_MASTER && pt->parent_pt != pt) {
    DLOG(1, "Level 1 (MASTER) pagetables must have self as parent.\n");
    return EINVALID;
  }
  if(pt->type == PT_COARSE && (pt->parent_pt == NULL || pt->parent_pt->type != PT_MASTER)) {
    DLOG(1, "Level 2 (COARSE) pagetables must have a MASTER pagetable as parent.\n");
    return EINVALID;
  }

  arm_memset_log2(pt->ptvaddr, 0, pt_sizes_log2[pt->type]);
  return OK;
}

/* Assume correctly filled out region struct. Writes entry into
 * associated pagetable. */
status vmm_map_region(region_t *r)
{
  int i;
  u32 *ptr, entry, idx;
  u32 t = r->pt->type;

  if(t >= PT_NUM_TYPES || t == PT_FAULT) {
    DLOG(1, "Unknown/invalid region pagetable type=%d.\n", t);
    return EINVALID;
  }
  if(((u32) r->vstart & pt_entry_masks[t]) != 0) {
    DLOG(1, "Region vstart (%#x) does not satisfy mask (%#x).\n",
         r->vstart, pt_entry_masks[t]);
    return EINVALID;
  }
  if((r->pstart & pt_entry_masks[t]) != 0) {
    DLOG(1, "Region pstart (%#x) does not satisfy mask (%#x).\n",
         r->pstart, pt_entry_masks[t]);
    return EINVALID;
  }
  if(r->page_size_log2 != pt_entry_sizes_log2[t]) {
    DLOG(1, "Region page_size_log2 (%d) is not valid (%d).\n",
         r->page_size_log2, pt_entry_sizes_log2[t]);
    return EINVALID;
  }

  idx = (u32) r->vstart;        /* index into pagetable */
  idx &= pt_start_masks[t];
  idx >>= pt_entry_sizes_log2[t];

  ptr = &r->pt->ptvaddr[idx + r->page_count - 1];

  entry = r->pstart;
  entry |= (r->access & pt_ap_masks[t]) << pt_ap_offsets[t]; /* access perms */
  entry |= (r->pt->domain & pt_dom_masks[t]) << pt_dom_offsets[t]; /* domain */
  entry |= pt_type_bits[t];         /* indicates type of entry */
  entry |= (r->cache_buf & 3) << 2; /* cache/buf bits */
  entry |= (r->shared_ng & 3) << pt_sng_offsets[t]; /* shared/not-global */

  for(i = r->page_count - 1; i >= 0; i--) {
    *ptr = entry + (i << pt_entry_sizes_log2[t]);
    /* The MMU is allowed to skip over caches when doing its
     * work. Make sure table entry is written to main memory. */
    /* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0344k/Bgbfdjaa.html */
    arm_cache_clean_invl_data_mva_poc(ptr--);
  }

  arm_mmu_flush_tlb();

  DLOG(4,"vmm_map_region: *** mapped %d pages: %#x ==> %#x (%#x)\n", r->page_count, r->pstart, r->vstart, entry);

  return OK;
}

/* Activate pagetable as or within current virtual address space. */
/* Assume valid pt struct. */
status vmm_activate_pagetable(pagetable_t *pt)
{
  u32 idx, entry;
  switch(pt->type) {
  case PT_MASTER:
    arm_mmu_set_ttb0(pt->ptpaddr);
    cpu_write(pagetable_t *, current_base_table, pt);
    break;
  case PT_COARSE:
    /* ARMv7 Short-descriptor first-level Page Table entry */
    /* Page table base address [10:31] */
    /* Domain [5:8] */
    /* Non-Secure [3:3] */
    /* Privileged Execute-Never [2:2] */
    /* Type [0:1] = 0b01 */
    entry = pt->ptpaddr & 0xFFFFFC00;
    entry |= (pt->domain & 0xF) << 5;
    entry |= 0x01;
    idx = (u32) pt->vstart >> 20;
    pt->parent_pt->ptvaddr[idx] = entry;
    /* MMU may skip L1 cache when reading page tables */
    arm_cache_clean_invl_data_mva_poc(&pt->parent_pt->ptvaddr[idx]);

    DLOG(4, "*** activate: ptpaddr=%#x ptvaddr=%#x idx=%#x entry=%#x\n", pt->parent_pt->ptpaddr, pt->parent_pt->ptvaddr, idx,
         pt->parent_pt->ptvaddr[idx]);
    break;
  default:
    DLOG(1, "Pagetable has invalid type=%d\n", pt->type);
    return EINVALID;
  }

  return OK;
}

void vmm_init(void)
{
  /* Below 1 GB use TTB0. Above 1 GB use TTB1. */
  arm_mmu_ttbcr(SETBITS(2,0,3), MMU_TTBCR_N | MMU_TTBCR_PD0 | MMU_TTBCR_PD1);
  /* Presume l1pt is already active in TTB0/TTB1, from stub. */
  vmm_init_pagetable(&kernel_l2pt);
  vmm_map_region(&kernel_region);
  vmm_activate_pagetable(&kernel_l2pt);
  arm_mmu_flush_tlb();
  u32 ttbcr;
  ASM("MRC p15, 0, %0, c2, c0, 2":"=r"(ttbcr));
  DLOG(1,"vmm_init: TTBCR=%#x\n", ttbcr);
}

status vmm_map_region_find_vstart(region_t *r)
{

  int i,j;
  if(r->pstart == 0) {
    DLOG(2,"vmm_map_region_find_vstart(%#x): r->pstart==0\n", r);
    return EINVALID;
  }
  if(r->page_size_log2 != PAGE_SIZE_LOG2) {
    DLOG(2,"vmm_map_region_find_vstart(%#x): r->page_size_log2!=%d\n", r, PAGE_SIZE_LOG2);
    return EINVALID;
  }
  if(r->page_count < 1 || r->page_count > 256) {
    DLOG(2,"vmm_map_region_find_vstart(%#x): r->page_count=%d invalid\n", r, r->page_count);
    return EINVALID;
  }
  if(r->pt == NULL || r->pt->type != PT_COARSE) {
    DLOG(2,"vmm_map_region_find_vstart(%#x): r->pt is NULL or invalid\n", r);
    return EINVALID;
  }

  for(i=0; i<(256 - r->page_count + 1); i++) {
    for(j=0; j<r->page_count; j++) {
      if(r->pt->ptvaddr[i+j] != 0)
        break;
    }
    if(j == r->page_count) {
      /* found one */
      r->vstart = r->pt->vstart + (i << r->page_size_log2);
      return vmm_map_region(r);
    }
  }

  DLOG(2, "vmm_map_region_find_vstart(%#x): unable to find free space for %d page(s).\n", r, r->page_count);
  return ENOSPACE;
}

status vmm_get_phys_addr(void *vaddr, physaddr *paddr)
{
  u32 addr = (u32) vaddr;
  u32 res;
  arm_mmu_va_to_pa((void *) (addr & 0xFFFFF000), 0, res);
  if (res & 1) return EINVALID;
  *paddr = (res & 0xFFFFF000) + (addr & 0xFFF);
  return OK;
}

void vmm_dump_kernel_l2pt(u32 start, u32 end)
{
  u32 i, mod=0;
  DLOG(1, "vmm_dump_kernel_l2pt(%#x, %#x):\n", start, end);
  for(i=start;i<end;i++) {
    if(mod == 0) printf_uart3("%x: ", i);
    printf_uart3("%.08X ", kernel_l2pt.ptvaddr[i]);
    mod++;
    if(mod == 4) { printf_uart3("\n"); mod = 0; }
  }
  if(mod != 0) printf_uart3("\n");
}

POOL_DEFN(pagetable_list,pagetable_list_t,16,4);

u32 vmm_find_mapping_entry(pagetable_list_t *ptl, void *addr)
{
  u32 ar = (u32) addr;
  for(;ptl != NULL;ptl = ptl->next) {
    u32 vstart = (u32) ptl->elt.vstart;
    u32 vend = vstart + (1 << pt_entry_sizes_log2[ptl->elt.type - 1]);
    if(ptl->elt.type == PT_COARSE && vstart <= ar && ar < vend) {
      u32 idx = (u32) addr;
      idx &= pt_start_masks[ptl->elt.type];
      idx >>= pt_entry_sizes_log2[ptl->elt.type];
      return ptl->elt.ptvaddr[idx];
    }
  }
  return 0;
}


#endif

/* Regions can be used with physical addresses */
POOL_DEFN(region_list,region_list_t,32,4);

void vmm_dump_region(region_t *r)
{
  char *access[] = {"NA", "PM", "RO", "RW"};
  DLOG(1, "region pstart=%#x vstart=%#x count=%#x pagesize=%#x %s%s%s%s %s\n",
       r->pstart, r->vstart, r->page_count, (1<<r->page_size_log2),
       ((r->cache_buf & R_B) ? "B" : "b"),
       ((r->cache_buf & R_C) ? "C" : "c"),
       ((r->shared_ng & R_S) ? "S" : "s"),
       ((r->shared_ng & R_NG) ? "NG" : "ng"),
       access[r->access & 0x3]);
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
