/*
 * Testing Processes
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

#ifdef TEST_PROCESS

#include "types.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "sched/process.h"
#include "sched/sched.h"
#include "sched/elf.h"
#define MODULE "test_process"
#include "debug/log.h"

extern u32 _program_map_start, _program_map_count;

static status program_load(void *pstart, process_t **return_p)
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

static inline void sched_launch_first_process(process_t *p)
{
  ASM(/* load context */
      "LDMIA   %0!, {r0,lr}\n"          /* load status register, return address */
      "MSR     spsr_cxsf, r0\n"         /* prep saved process status register */
      "LDMIA   %0, {r0-r14}^\n"         /* load user registers (incl. r13_usr, r14_usr) */
      /* and return to userspace */
      "LDR     r13, =svc_stack_top\n"   /* r13 = &svc_stack_top */
      "MOVS    pc, lr"::"r"(p):"r0");   /* jump to userspace */
}

status test_process(void)
{
  extern process_t *current;
  void *entry;
  process_t *p, *q;
  u32 *progs = (u32 *) &_program_map_start, cnt = (u32) &_program_map_count;

  if(cnt < 2) {
    DLOG(1, "test_process: requires > 2 programs\n");
    return EINVALID;
  }

  DLOG(1, "test_process: loading program found at %#x\n", progs[0]);
  if(program_load((void *) progs[0], &p) != OK) {
    DLOG(1, "program_load failed\n");
    return EINVALID;
  }

  DLOG(1, "test_process: loading program found at %#x\n", progs[1]);
  if(program_load((void *) progs[1], &q) != OK) {
    DLOG(1, "program_load failed\n");
    return EINVALID;
  }

  sched_wakeup(q);
  process_switch_to(p);

  current = p;
  entry = p->entry;
  DLOG(1, "%#x: %#x %#x %#x %#x\n", entry,
       ((u32 *) entry)[0], ((u32 *) entry)[1], ((u32 *) entry)[2], ((u32 *) entry)[3]);

  DLOG(1, "Switching to usermode.\n");

  sched_launch_first_process(p);

  return OK;
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
