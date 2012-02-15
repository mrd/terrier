/*
 * Scheduler process support
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
#include "mem/virtual.h"
#include "mem/physical.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "arm/status.h"
#include "sched/process.h"
#include "sched/elf.h"
#include "sched/sched.h"
#include "omap3/early_uart3.h"
#define MODULE "process"
#include "debug/log.h"

process_t process[MAX_PROCESSES];

process_t *process_find(pid_t pid)
{
  if(0 < pid && pid <= MAX_PROCESSES)
    return process + (pid - 1);
  else {
    DLOG(1, "process_find(%d) invalid\n", pid);
    return NULL;
  }
}

/* Switch processes. Requires valid p. */
status process_switch_to(process_t *p)
{
  vmm_activate_pagetable(&p->tables->elt);
  data_sync_barrier();
  arm_mmu_set_context_id(p->pid - 1);
  prefetch_flush();
  return OK;
}

status process_new(process_t **return_p)
{
  int i;
  for(i=0; i<MAX_PROCESSES; i++) {
    if(process[i].pid == NOPID) {
      process_t *p = process + i;
      pagetable_t pt, *l1, *l2;
      /* initialize level-1 pt */

      /* get four contiguous naturally aligned physical pages */
      if(physical_alloc_pages(4, 4, &pt.ptpaddr) != OK) {
        DLOG(1, "process_new: physical_alloc_pages for pagetable failed.\n");
        return ENOSPACE;
      }
      region_t rtmp = { pt.ptpaddr, NULL, &kernel_l2pt, 4, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
      /* map to kernel virtual memory */
      if(vmm_map_region_find_vstart(&rtmp) != OK) {
        DLOG(1, "process_new: vmm_map_region_find_vstart for pagetable failed.\n");
        /* FIXME: clean-up previous resources */
        return ENOSPACE;
      }
      /* fill out remaining struct */
      pt.ptvaddr = rtmp.vstart;
      pt.parent_pt = &pt;       /* temp */
      pt.type = PT_MASTER;
      pt.domain = 0;
      pt.vstart = 0;
      vmm_init_pagetable(&pt);
      pagetable_list_t *ptl1 = pagetable_list_pool_alloc();
      if (ptl1 == NULL) {
        DLOG(1, "process_new: pagetable_list_pool_alloc failed.\n");
        /* FIXME: clean-up previous resources */
        return ENOSPACE;
      }
      ptl1->elt = pt;
      p->tables = NULL;
      pagetable_append(&p->tables, ptl1);
      /* copied pagetable, therefore, fix parent_pt self-reference */
      l1 = p->tables->elt.parent_pt = &p->tables->elt;

      /* initialize level-2 pt */

      if(physical_alloc_pages(1, 1, &pt.ptpaddr) != OK) {
        DLOG(1, "process_new: physical_alloc_pages for pagetable failed.\n");
        /* FIXME: clean-up previous resources */
        return ENOSPACE;
      }
      region_t rtmp2 = { pt.ptpaddr, NULL, &kernel_l2pt, 1, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
      /* map to kernel virtual memory */
      if(vmm_map_region_find_vstart(&rtmp2) != OK) {
        DLOG(1, "process_new: vmm_map_region_find_vstart for pagetable failed.\n");
        /* FIXME: clean-up previous resources */
        return ENOSPACE;
      }
      /* fill out remaining struct */
      pt.ptvaddr = rtmp2.vstart;
      pt.parent_pt = l1;
      pt.type = PT_COARSE;
      pt.domain = 0;
      pt.vstart = 0;
      vmm_init_pagetable(&pt);
      pagetable_list_t *ptl2 = pagetable_list_pool_alloc();
      if(ptl2 == NULL) {
        DLOG(1, "process_new: pagetable_list_pool_alloc failed.\n");
        /* FIXME: clean-up previous resources */
        return ENOSPACE;
      }
      ptl2->elt = pt;
      pagetable_append(&p->tables, ptl2);
      l2 = &ptl2->elt;
      vmm_activate_pagetable(l2);

      p->regions = NULL;

      /* set pid */
      p->pid = (pid_t) (i + 1);
      *return_p = p;
      return OK;
    }
  }
  return ENOSPACE;
}

void process_init(void)
{
  int i;
  for(i=0; i<MAX_PROCESSES; i++)
    process[i].pid = NOPID;
}

status program_load(void *pstart, process_t **return_p)
{
  process_t *p;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Phdr *pph = (void *) pe + pe->e_phoff, *loadph = NULL;
  void *entry = (void *) pe->e_entry;
  int i, memsz = 0;

  /* Parse ELF */
  DLOG(1, "Magic: %X%X%X%X\n", pe->e_ident[0], pe->e_ident[1], pe->e_ident[2], pe->e_ident[3]);
  if(!(pe->e_ident[EI_MAG0] == ELFMAG0 &&
       pe->e_ident[EI_MAG1] == ELFMAG1 &&
       pe->e_ident[EI_MAG2] == ELFMAG2 &&
       pe->e_ident[EI_MAG3] == ELFMAG3)) {
    DLOG(1, "Bad Magic\n");
    return EINVALID;
  }
  DLOG(1, "phnum=%d phoff=%#x entry=%#x\n", pe->e_phnum, pe->e_phoff, entry);
  for(i=0;i<pe->e_phnum;i++) {
    if (pph->p_type == PT_LOAD) {
      memsz += pph->p_memsz;
      loadph = pph;
    }
    DLOG(1, " ph[%d]: type=%#x flags=%#x filesz=%#x offset=%#x memsz=%#x vaddr=%#x align=%#x\n",
         i, pph->p_type, pph->p_flags, pph->p_filesz, pph->p_offset,
         pph->p_memsz, pph->p_vaddr, pph->p_align);
    pph = (void *) pph + pe->e_phentsize;
  }
  if(loadph == NULL) {
    DLOG(1, "No loadable program header found.\n");
    return EINVALID;
  }

  /* Create process */
  if(process_new(&p) != OK) {
    DLOG(1, "process_new failed\n");
    return EINVALID;
  }
  p->entry = entry;

  /* setup context */
  p->ctxt.lr = (u32) entry;     /* starting address */
  p->ctxt.psr = MODE_USR;       /* starting status register */

  pagetable_t *l2;
  u32 pages = (loadph->p_memsz + 0xFFF) >> PAGE_SIZE_LOG2;
  u32 align = 1;                /* physical: shouldn't matter */
  l2 = &p->tables->next->elt;

  /* Create userspace region */

  region_list_t *rl = region_list_pool_alloc();
  if(rl == NULL) {
    DLOG(1, "process_new: region_list_pool_alloc failed.\n");
    /* FIXME: clean-up previous resources */
    return ENOSPACE;
  }
  if(physical_alloc_pages(pages, align, &rl->elt.pstart) != OK) {
    DLOG(1, "process_new: physical_alloc_pages for region failed.\n");
    /* FIXME: clean-up previous resources */
    return ENOSPACE;
  }

  rl->elt.vstart = (void *) loadph->p_vaddr;
  rl->elt.pt = l2;
  rl->elt.page_count = pages;
  rl->elt.page_size_log2 = PAGE_SIZE_LOG2;
  rl->elt.cache_buf = R_C | R_B; /* cached and buffered */
  rl->elt.shared_ng = R_NG;      /* not global */
  rl->elt.access = R_RW;         /* read-write user mode */
  region_append(&p->regions, rl);
  vmm_map_region(&rl->elt);

  process_switch_to(p);
  /* cheat, switch to process and use its mapping */

  u8 *dest = (u8 *) loadph->p_vaddr;
  u8 *src = &((u8 *) pstart)[loadph->p_offset];
  DLOG(1, "program_load: copying %d bytes src=%#x dest=%#x\n", loadph->p_filesz, src, dest);
  for(i=0;i<loadph->p_filesz;i++) {
    dest[i] = src[i];
  }

  *return_p = p;
  return OK;
}

status programs_init(void)
{
  extern u32 _program_map_start, _program_map_count;
  extern process_t *current;
  u32 *progs = (u32 *) &_program_map_start, cnt = (u32) &_program_map_count;
  process_t *p;
  int i;

  if(cnt < 1) {
    DLOG(1, "programs_init: requires at least 1 program to run.\n");
    return EINVALID;
  }

  current = NULL;
  for(i=0;i<cnt;i++) {
    DLOG(1, "programs_init: loading program found at %#x\n", progs[i]);
    if(program_load((void *) progs[i], &p) != OK) {
      DLOG(1, "program_load failed\n");
      return EINVALID;
    }
    if(current == NULL) current = p;
    else sched_wakeup(p);
  }

  process_switch_to(current);

  DLOG(1, "Switching to usermode.\n");
  sched_launch_first_process(p);

  /* control flow should not return to here */
  early_panic("unreachable");
  return EUNDEFINED;
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
