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

#ifndef _MEM_VIRTUAL_H_
#define _MEM_VIRTUAL_H_

#include "types.h"
#include "list.h"
#include "mem/pool.h"

typedef struct pagetable {
  void *vstart;                 /* starting virtual address managed by PT */
  physaddr ptpaddr;             /* address of pagetable in physical memory */
  u32 *ptvaddr;                 /* address of pagetable in virtual memory */
  struct pagetable *parent_pt;  /* parent pagetable or self if MASTER */
#define PT_FAULT 0
#define PT_MASTER 1
#define PT_COARSE 2
  u16 type;                     /* one of PT_* */
  u16 domain;                   /* domain of section entries if MASTER */
} pagetable_t;
DLIST_TYPE(pagetable,pagetable_t);
POOL_PROTO(pagetable_list,pagetable_list_t);

typedef struct {
  physaddr pstart;              /* starting physical address of region */
  void *vstart;                 /* starting virtual address of region */
  pagetable_t *pt;              /* pagetable managing this region */
  u32 page_count;               /* number of pages in this region */
  u16 page_size_log2;           /* size of pages in this region (log2) */
#define R_B 1                   /* set "buffered" bit */
#define R_C 2                   /* set "cached" bit */
  u8 cache_buf:4;               /* cache/buffer attributes */
#define R_S 1                   /* set "shared" bit */
#define R_NG 2                  /* set "not-global" bit */
  u8 shared_ng:4;               /* shared/not-global attributes */
#define R_NA 0                  /* no access (besides use of System/ROM bits) */
#define R_PM 1                  /* privileged mode only */
#define R_RO 2                  /* user-mode can read-only */
#define R_RW 3                  /* user-mode can read-write */
  u8 access;                    /* access permission attributes */
} region_t;
DLIST_TYPE(region,region_t);
POOL_PROTO(region_list,region_list_t);

extern pagetable_t l1pt;
extern pagetable_t kernel_l2pt;

status vmm_activate_pagetable(pagetable_t *pt);
status vmm_map_region(region_t *r);
status vmm_init_pagetable(pagetable_t *pt);
status vmm_map_region_find_vstart(region_t *r);
status vmm_get_phys_addr(void *vaddr, physaddr *paddr);
void vmm_dump_kernel_l2pt(u32 start, u32 end);
void vmm_dump_region(region_t *);

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
