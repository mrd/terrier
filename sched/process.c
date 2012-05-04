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

void program_dump_sections(void *pstart)
{
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff;
  int shnum = pe->e_shnum, i;
  int shstrndx = pe->e_shstrndx; /* section header string table index */
  char *shstr = (void *) pe + ((Elf32_Shdr *) ((void *) sh + (shstrndx * pe->e_shentsize)))->sh_offset;

  DLOG(1, "shstrndx=%d shstr=%#x\n", shstrndx, shstr);
  for(i=0; i<shnum; i++) {
    DLOG(1, "shnum=%d name=%d (%s) type=%d offs=%#x size=%#x link=%d info=%d entsz=%d\n",
         i, sh->sh_name, shstr + sh->sh_name, sh->sh_type, sh->sh_offset, sh->sh_size,
         sh->sh_link, sh->sh_info, sh->sh_entsize);
    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }
}

Elf32_Shdr *program_find_section(void *pstart, const char *name, u32 type)
{
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff;
  int shnum = pe->e_shnum, i;
  int shstrndx = pe->e_shstrndx; /* section header string table index */
  char *shstr = (void *) pe + ((Elf32_Shdr *) ((void *) sh + (shstrndx * pe->e_shentsize)))->sh_offset;

  for(i=0; i<shnum; i++) {
    if((type == 0 || type == sh->sh_type) &&
       (name == NULL || strcmp(shstr + sh->sh_name, name) == 0))
      return sh;
    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }
  return NULL;
}

Elf32_Sym *program_find_symbol(void *pstart, const char *name)
{
  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  Elf32_Shdr *strtabsh = program_find_section(pstart, ".strtab", SHT_STRTAB);
  Elf32_Sym *sym;
  char *str;
  int i;

  if(symtabsh == NULL || strtabsh == NULL)
    return NULL;

  sym = (void *) pstart + symtabsh->sh_offset;
  str = (void *) pstart + strtabsh->sh_offset;

  for(i=0; i< symtabsh->sh_size / sizeof(Elf32_Sym); i++) {
    if(strcmp(str + sym[i].st_name, name) == 0)
      return &sym[i];
  }
  return NULL;
}

Elf32_Sym *program_find_symbol_index(void *pstart, int ndx)
{
  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  Elf32_Sym *sym;

  if(symtabsh == NULL)
    return NULL;

  sym = (void *) pstart + symtabsh->sh_offset;

  return &sym[ndx];
}

char *program_find_string(void *pstart, int strndx)
{
  Elf32_Shdr *strtabsh = program_find_section(pstart, ".strtab", SHT_STRTAB);
  if(strtabsh == NULL)
    return NULL;

  return (char *) pstart + strtabsh->sh_offset + strndx;
}

void program_dump_symbols(void *pstart)
{
  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  Elf32_Shdr *strtabsh = program_find_section(pstart, ".strtab", SHT_STRTAB);
  Elf32_Sym *sym;
  char *str;
  int i;

  if(symtabsh == NULL || strtabsh == NULL)
    return;

  sym = (void *) pstart + symtabsh->sh_offset;
  str = (void *) pstart + strtabsh->sh_offset;

  for(i=0; i< symtabsh->sh_size / sizeof(Elf32_Sym); i++) {
    DLOG(1, "\"%s\" value=%#x size=%#x info=%#x other=%#x shndx=%d\n",
         str + sym[i].st_name,
         sym[i].st_value,
         sym[i].st_size,
         sym[i].st_info,
         sym[i].st_other,
         sym[i].st_shndx);
  }
}

status program_load(void *pstart, process_t **return_p)
{
  process_t *p;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  void *entry = (void *) pe->e_entry;
  Elf32_Sym *sym;
  int i;
  u32 memsz;

  /* Parse ELF */
  DLOG(1, "Magic: %X%X%X%X\n", pe->e_ident[0], pe->e_ident[1], pe->e_ident[2], pe->e_ident[3]);
  if(!(pe->e_ident[EI_MAG0] == ELFMAG0 &&
       pe->e_ident[EI_MAG1] == ELFMAG1 &&
       pe->e_ident[EI_MAG2] == ELFMAG2 &&
       pe->e_ident[EI_MAG3] == ELFMAG3)) {
    DLOG(1, "Bad Magic\n");
    return EINVALID;
  }

  DLOG(1, "ELF type=%#x arch=%#x version=%#x entry=%#x flags=%#x\n",
       pe->e_type, pe->e_machine, pe->e_version, pe->e_entry, pe->e_flags);

  if(pe->e_type != ET_REL) {
    DLOG(1, "Not a relocatable file.\n");
    return EINVALID;
  }

#define EF_ARM_EABI_VER5 0x05000000

  if(pe->e_machine != EM_ARM || EF_ARM_EABI_VERSION(pe->e_flags) != EF_ARM_EABI_VER5) {
    DLOG(1, "Not a valid ARM architecture file. (machine=%#x, flags=%#x) should be (%#x, %#x)\n",
         pe->e_machine, pe->e_flags, EM_ARM, EF_ARM_EABI_VER5);
    return EINVALID;
  }

  program_dump_sections(pstart);

  /* Select base address */

  u32 base = 0x8000;            /* testing */

  /* Extract sections and create layout */

  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);
  Elf32_Shdr *reltext = program_find_section(pstart, ".rel.text", SHT_REL);
  /* Elf32_Shdr *relatext = program_find_section(pstart, ".rela.text", SHT_RELA); */
  Elf32_Shdr *data = program_find_section(pstart, ".data", SHT_PROGBITS);
  Elf32_Shdr *bss  = program_find_section(pstart, ".bss", SHT_NOBITS);

  u32 textshnum = 1;            /* temp */
  u32 datashnum = 6;            /* temp */
  u32 bssshnum  = 7;            /* temp */

  /* Calculate addresses for .text, .data, .bss */

  u32 textbase = base;
  u32 database = textbase + text->sh_size;
  u32 bssbase  = database + data->sh_size;
  DLOG(1, "base addresses: text=%#x data=%#x bss=%#x\n", textbase, database, bssbase);

  memsz = text->sh_size + data->sh_size + bss->sh_size;

  /* Rewrite symtab based on shndx, and put symbols into proper spaces */

  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  sym = (void *) pstart + symtabsh->sh_offset;
  for(i=0; i< symtabsh->sh_size / sizeof(Elf32_Sym); i++) {
    if(sym[i].st_shndx == textshnum)
      sym[i].st_value += textbase;
    if(sym[i].st_shndx == datashnum)
      sym[i].st_value += database;
    if(sym[i].st_shndx == bssshnum)
      sym[i].st_value += bssbase;
  }

  //program_dump_symbols(pstart);

  /* Iterate through relocation sections and rewrite binary */

  Elf32_Rel *rel = (void *) pstart + reltext->sh_offset;
  for(i=0; i<reltext->sh_size/sizeof(Elf32_Rel); i++) {
    u32 A, S, P;
    u32 ndx = ELF32_R_SYM(rel[i].r_info);
    u32 typ = ELF32_R_TYPE(rel[i].r_info);
    u32 off = rel[i].r_offset;
    u32 *ptr = (void *) pstart + text->sh_offset + off;
    char *symstr;
    sym = program_find_symbol_index(pstart, ndx);
    if(sym == NULL) {
      DLOG(1, "Unable to find sym=%d\n", ndx);
      break;
    }
    symstr = program_find_string(pstart, sym->st_name);
    if(symstr == NULL) {
      DLOG(1, "Unable to find symbol %d name string index=%d\n", ndx, sym->st_name);
      break;
    }

    DLOG(1, "Rewriting offset=%#x type=%#x ndx=%d (%s) ", off, typ, ndx, symstr);

    S = sym->st_value;

    u32 ins;
    switch(typ) {
    case R_ARM_ABS32:           /* Absolute 32-bit relocation */
      A = *ptr;
      *ptr = S + A;
      DLOG(1, "A=%#x S=%#x result=%#x\n", A, S, *ptr);
      break;
    case R_ARM_CALL:            /* branch with link instruction */
      P = textbase + off;
      A = (*ptr << 8) >> 6;     /* sign-extend 24-bit immediate and mul-by-4 */
      *ptr &= 0xFF000000;
      *ptr |= ((S + A - P) & 0x00FFFFFF) >> 2; /* re-encode */
      DLOG(1, "A=%#x S=%#x P=%#x result=%#x\n", A, S, P, *ptr);
      break;
    case R_ARM_MOVT_ABS:        /* mov TOP */
      S >>= 16;
    case R_ARM_MOVW_ABS_NC:     /* mov WIDE */
      /* *ptr = "...:imm4:Rd:imm12" */
      /* addend = imm4:imm12 */
      ins = *ptr;
      A = ((ins & 0xF0000) >> 4) | (ins & 0xFFF);
      u32 t = S + A;
      /* re-encode instruction */
      *ptr = (ins & 0xFFF00000) | ((t & 0xF000) << 4) | (ins & 0x0000F000) | (t & 0xFFF);
      DLOG(1, "A=%#x S=%#x result=%#x\n", A, S, *ptr);
      break;
    default:
      DLOG(1, "Unknown relocation type.\n");
      break;
    }
  }

  /* Create process */
  if(process_new(&p) != OK) {
    DLOG(1, "process_new failed\n");
    return EINVALID;
  }

  DLOG(1, "_start=%#x _end_entry=%#x\n",
       program_find_symbol(pstart, "_start")->st_value,
       program_find_symbol(pstart, "_end_entry")->st_value);

  sym = program_find_symbol(pstart, "_end_entry");
  if(sym == NULL) {
    DLOG(1, "unable to find _end_entry\n");
    return EINVALID;
  }

  entry += textbase;
  p->entry = entry;
  p->end_entry = (void *) sym->st_value;

  /* setup context */
  for(i=0;i<sizeof(context_t)>>2;i++)
    ((u32 *)&p->ctxt)[i] = 0;

  p->ctxt.lr = (u32) entry;     /* starting address */
  p->ctxt.usr.r[15] = (u32) entry;     /* starting address */
  p->ctxt.psr = MODE_SYS;       /* starting status register */

  pagetable_t *l2;
  u32 pages = (memsz + 0xFFF) >> PAGE_SIZE_LOG2;
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

  rl->elt.vstart = (void *) base;
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

  u8 *dest, *src;

  /* copy text */
  dest = (u8 *) textbase;
  src = pstart + text->sh_offset;
  DLOG(1, "program_load: copying %d bytes src=%#x dest=%#x\n", text->sh_size, src, dest);
  for(i=0;i<text->sh_size;i++)
    dest[i] = src[i];

  /* copy data */
  dest = (u8 *) database;
  src = pstart + data->sh_offset;
  DLOG(1, "program_load: copying %d bytes src=%#x dest=%#x\n", data->sh_size, src, dest);
  for(i=0;i<data->sh_size;i++)
    dest[i] = src[i];

  /* zero bss */
  dest = (u8 *) bssbase;
  DLOG(1, "program_load: zeroing %d bytes dest=%#x\n", bss->sh_size, dest);
  for(i=0;i<bss->sh_size;i++)
    dest[i] = 0;

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
