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
#include "omap/smp.h"
#include "omap/timer.h"
#include "arm/status.h"
#include "arm/smp/per-cpu.h"
#include "arm/smp/semaphore.h"
#include "sched/process.h"
#include "sched/elf.h"
#include "sched/sched.h"
#include "omap/early_uart3.h"
#define MODULE "process"
#include "debug/log.h"

process_t process[MAX_PROCESSES];
static process_t idle_process[MAX_CPUS];
DEF_PER_CPU(process_t *, cpu_idle_process);
INIT_PER_CPU(cpu_idle_process) {
  cpu_write(process_t *, cpu_idle_process, &idle_process[cpu_index()]);
}

void idle_loop(void)
{
  enable_interrupts();
  for(;;) ASM("WFE");
}

static context_t idle_context = {
  .psr = MODE_SVC, .lr = (u32) idle_loop
};

process_t *process_find(pid_t pid)
{
  if(0 < pid && pid <= MAX_PROCESSES)
    return process + (pid - 1);
  else {
    DLOG(1, "process_find(%d) invalid\n", pid);
    return NULL;
  }
}

/* Does the region [ptr, ptr+bytes] lie within the process? */
status process_is_valid_pointer(process_t *p, void *ptr, u32 bytes)
{
  /* FIXME: doesn't handle straddled regions */
  u32 ptrval = (u32) ptr;
  region_list_t *rl = p->regions;
  while(rl != NULL) {
    u32 start = (u32) rl->elt.vstart;
    u32 end = start + (rl->elt.page_count << rl->elt.page_size_log2);
    if(start <= ptrval && (ptrval + bytes) <= end)
      return OK;
    rl = rl->next;
  }
  return EINVALID;
}

/* Switch processes. Requires valid p (meaning, in process[] table). */
status process_switch_to(process_t *p)
{
#ifdef USE_VMM
  //Set TTBCR.PD0 = 1
  //ISB
  //Change ASID to new value
  //Change Translation Table Base Register to new value
  //ISB
  //Set TTBCR.PD0 = 0

  arm_mmu_ttbcr(MMU_TTBCR_PD0, MMU_TTBCR_PD0);

  prefetch_flush();
  arm_mmu_set_context_id(p->pid-1);
  vmm_activate_pagetable(&p->tables->elt);
  data_sync_barrier();

  prefetch_flush();
  arm_cache_invl_instr();
  arm_cache_invl_branch_pred_array();

  arm_mmu_ttbcr(0, MMU_TTBCR_PD0);
#endif
  clrex();

  return OK;
}

#define PT_ATTRS R_C | R_B, R_S

status process_new(process_t **return_p)
{
  int i;
  for(i=0; i<MAX_PROCESSES; i++) {
    if(process[i].pid == NOPID) {
      process_t *p = process + i;
#ifdef USE_VMM
      pagetable_t pt, *l1, *l2;
      /* initialize level-1 pt */

      /* get four contiguous naturally aligned physical pages */
      if(physical_alloc_pages(4, 4, &pt.ptpaddr) != OK) {
        DLOG(1, "process_new: physical_alloc_pages for pagetable failed.\n");
        return ENOSPACE;
      }
      region_t rtmp = { pt.ptpaddr, NULL, &kernel_l2pt, 4, PAGE_SIZE_LOG2, PT_ATTRS, R_PM };
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
      region_t rtmp2 = { pt.ptpaddr, NULL, &kernel_l2pt, 1, PAGE_SIZE_LOG2, PT_ATTRS, R_PM };
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
#endif
      p->regions = NULL;

      /* set pid */
      p->pid = (pid_t) (i + 1);
      *return_p = p;

#if SCHED==rms || SCHED==rms_sched
      p->totalticks = p->runticks = p->r = p->b = 0;
      /* some test values */
      p->c = 1 << 16;
      p->t = 1 << 18;
#endif
      return OK;
    }
  }
  return ENOSPACE;
}

/* precondition: SMP already initialized */
void process_init(void)
{
  int i;
  extern u32 num_cpus;
  for(i=0; i<MAX_PROCESSES; i++)
    process[i].pid = NOPID;
  for(i=0; i<num_cpus; i++) {
    idle_process[i].pid = NOPID;
    idle_process[i].ctxt.psr = idle_context.psr;
    idle_process[i].ctxt.lr = idle_context.lr;
    idle_process[i].entry = idle_loop;
    idle_process[i].end_entry = idle_loop;
  }
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
    DLOG(1, "shnum=%d name=%d (%s) type=%d addr=%#x offs=%#x size=%#x link=%d info=%d entsz=%d\n",
         i, sh->sh_name, shstr + sh->sh_name, sh->sh_type, sh->sh_addr, sh->sh_offset, sh->sh_size,
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

Elf32_Shdr *program_find_section_index(void *pstart, int ndx)
{
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff;
  int shnum = pe->e_shnum, i;

  for(i=0; i<shnum; i++) {
    if(i == ndx)
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

char *program_find_string_in(void *pstart, Elf32_Shdr *strtabsh, int strndx)
{
  return (char *) pstart + strtabsh->sh_offset + strndx;
}

char *program_find_string(void *pstart, int strndx)
{
  Elf32_Shdr *strtabsh = program_find_section(pstart, ".strtab", SHT_STRTAB);
  if(strtabsh == NULL)
    return NULL;

  return program_find_string_in(pstart, strtabsh, strndx);
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

/* Add up section sizes */
u32 program_memory_size(void *pstart)
{
  /* Loop through sections and add up "in memory" sections */
  u32 size = 0;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff;
  int shnum = pe->e_shnum, i;

  for(i=0; i<shnum; i++) {
    if(sh->sh_flags & SHF_ALLOC) /* ALLOC means "in memory" */
      size += sh->sh_size;
    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }

  return size;
}

/* place ALLOC sections into memory and rewrite headers; return final size */
static u32 lay_out_sections(void *pstart, u32 base)
{
  u32 curaddr = base;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff;
  int shnum = pe->e_shnum, i;

  /* loop through sections */
  for(i=0; i<shnum; i++) {
    if(sh->sh_flags & SHF_ALLOC) {
      if(sh->sh_addralign > 0) {
        u32 al = sh->sh_addralign;
        /* align new address */
        if((curaddr & (al-1)) != 0)
          curaddr += al - (curaddr & (al-1));
      }

      /* relocate section to new address */
      sh->sh_addr = curaddr;

      /* next section goes after this one */
      curaddr += sh->sh_size;
    }
    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }

  return curaddr - base;        /* return memory size */
}

/* Rewrite the ELF at 'pstart' following instructions in relocation
 * section 'rel' with 'num_rel_entries' entries and apply those
 * instructions to the binary data managed by section header 'sh'. */
static void rewrite_binary_section(void *pstart, Elf32_Rel *rel, u32 num_rel_entries, Elf32_Shdr *sh)
{
  u32 i;

  DLOG(1, "rewrite_binary_section: applying %d relocation entries to section (%s).\n", num_rel_entries,
       program_find_string_in(pstart, program_find_section_index(pstart, ((Elf32_Ehdr *) pstart)->e_shstrndx), sh->sh_name));
  for(i=0; i<num_rel_entries; i++) {
    u32 A, S, P;
    u32 ndx = ELF32_R_SYM(rel[i].r_info);
    u32 typ = ELF32_R_TYPE(rel[i].r_info);
    u32 off = rel[i].r_offset;
    u32 *ptr = (void *) pstart + sh->sh_offset + off;
    char *symstr;
    Elf32_Sym *sym = program_find_symbol_index(pstart, ndx);
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
    case R_ARM_NONE:
      DLOG_NO_PREFIX(1, "NONE\n");
      break;
    case R_ARM_ABS32:           /* Absolute 32-bit relocation */
      A = *ptr;
      *ptr = S + A;
      DLOG_NO_PREFIX(1, "ABS32: A=%#x S=%#x result=%#x\n", A, S, *ptr);
      break;
    case R_ARM_CALL:            /* branch with link instruction */
      P = sh->sh_addr + off;
      A = (*ptr << 8) >> 6;     /* sign-extend 24-bit immediate and mul-by-4 */
      *ptr &= 0xFF000000;
      *ptr |= ((S + A - P) & 0x00FFFFFF) >> 2; /* re-encode */
      DLOG_NO_PREFIX(1, "CALL: A=%#x S=%#x P=%#x result=%#x\n", A, S, P, *ptr);
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
      DLOG_NO_PREFIX(1, "MOVT/MOVW: A=%#x S=%#x result=%#x\n", A, S, *ptr);
      break;
    case R_ARM_PREL31:
      // R_ARM_PREL31 formula is ((S + A) | T) – P
      A = *ptr;                 /* Data-relocation: A is sign-extended to 32-bits */
      P = sh->sh_addr + off;    /* the address of the place being relocated (derived from r_offset). */
      /* T = 0; // because we don't support THUMB */
      *ptr = ((S + A)) - P;
      DLOG_NO_PREFIX(1, "PREL31: A=%#x S=%#x P=%#x result=%#x\n", A, S, P, *ptr);
      break;
    default:
      early_panic("Unknown relocation type. See ELF-for-ARM manual to implement.\n");
      break;
    }
  }

}

/* Rewrite symbol table and binary based on actual memory lay-out */
status program_relocate(void *pstart, u32 base, u32 *entry)
{
  int i;
  Elf32_Sym *sym;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  Elf32_Shdr *sh;
  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);

#define CHK_SEC(var) if(var == NULL) { DLOG(1, "Unable to find section %s.\n", #var); return EINVALID; }
  CHK_SEC(text);

  /* Rewrite symtab based on shndx, and put symbols into proper spaces */

  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  CHK_SEC(symtabsh);

  /* Rewrite values in symbol table that have been relocated by lay-out */
  sym = (void *) pstart + symtabsh->sh_offset;
  for(i=0; i< symtabsh->sh_size / sizeof(Elf32_Sym); i++) {
    sh = program_find_section_index(pstart, sym[i].st_shndx);
    if(sh == NULL) continue;    /* skip nonsense section */

    /* if symbol section is in memory (meaning it was relocated) then
     * adjust the symbol's value */
    if(sh->sh_flags & SHF_ALLOC)
      sym[i].st_value += sh->sh_addr;

    DLOG(4, "Elf32_Sym name=\"%s\" shndx=%d value=%#x\n",
         program_find_string(pstart, sym[i].st_name),
         sym[i].st_shndx,
         sym[i].st_value);
  }


  /* FIXME: handle SHT_RELA too */
  if(program_find_section(pstart, NULL, SHT_RELA))
    early_panic("Unable to handle SHT_RELA.");

  /* Iterate through relocation sections and rewrite binary */

  sh = (void *) pe + pe->e_shoff; /* start at first section */
  /* loop through sections */
  for(i=0; i<pe->e_shnum; i++) {
    if(sh->sh_type == SHT_REL) { /* if it is a relocation section */
      Elf32_Rel *rel = (void *) pstart + sh->sh_offset;
      u32 num_rel_entries = sh->sh_size/sizeof(Elf32_Rel);
      Elf32_Shdr *target_sh;    /* the target section to be rewritten */
      /* sh_info contains the target section index; check its validity */
      if(0 < sh->sh_info && sh->sh_info < pe->e_shnum) {
        target_sh = program_find_section_index(pstart, sh->sh_info);
        rewrite_binary_section(pstart, rel, num_rel_entries, target_sh);
        /* postcondition: data in section is now rewritten */
      } else
        DLOG(1, "SHT_REL entry %d had invalid sh_info=%d\n", i, sh->sh_info);
    }

    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }

  *entry = text->sh_addr + pe->e_entry;
  return OK;
}

/* Load a (relocatable) ELF program the image of which begins at
 * 'pstart' and, if successful, will place the pointer to the
 * process_t data structure into the 'return_p' location. */
status program_load(void *pstart, process_t **return_p)
{
  process_t *p;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  u32 entry;
  Elf32_Sym *sym;
  int i;

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

  /* *** */

  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);
  u32 align = (text->sh_addralign <= PAGE_SIZE ? 1 : text->sh_addralign >> 12);
  /* choose page-level alignment based on ELF section ".text" desired alignment;
   * any alignment less than a page is rounded up to a page */

  /* obtain estimate of program memory size */
  u32 memsz = program_memory_size(pstart), memsz2;
  u32 pages, pages2, base;
  physaddr region_phys;
  DLOG(2, "program_memory_size=%d\n", memsz);

  if(memsz == 0) {
    DLOG(1, "program_memory_size == 0\n");
    return EINVALID;
  }

  /*** Allocate physical memory and rewrite section headers */

  /* Complication: alignment constraints on sections may cause the
   * program to require more space in memory than the initial estimate
   * suggested. */

  do {
    pages = (memsz + 0xFFF) >> PAGE_SIZE_LOG2;

    /* Create region in physical memory for program */
    if(physical_alloc_pages(pages, align, &region_phys) != OK) {
      DLOG(1, "process_new: physical_alloc_pages for region failed.\n");
      /* FIXME: clean-up previous resources */
      return ENOSPACE;
    }

    /* Select base address */
#ifdef USE_VMM
    base = VIRTUAL_BASE_ADDR;
#else
    base = region_phys;
#endif

    /* rewrite sections using new base address */
    memsz2 = lay_out_sections(pstart, base);
    /* postcondition: the ELF sections have been relocated to new memory addresses */

    pages2 = (memsz2 + 0xFFF) >> PAGE_SIZE_LOG2;
    if(pages2 > pages) {
      /* Problem: alignment constraints during lay-out caused memory
       * requirements to overflow the simple estimate. Must redo with
       * larger size. */
      memsz = memsz2;

      /* clean up previous allocation */
      for(i=0; i<pages; i++)
        physical_free_page(region_phys + (i << PAGE_SIZE_LOG2));
    }
  } while(pages2 != pages);

  /*** Now memsz is determined and section headers have been rewritten. */

  if(program_relocate(pstart, base, &entry) != OK) {
    DLOG(1, "program_relocate(%#x, %#x) failed.\n", pstart, region_phys);
    /* FIXME: clean-up previous resources */
    return EINVALID;
  }

  /*** Create process */

  if(process_new(&p) != OK) {
    DLOG(1, "process_new failed\n");
    /* FIXME: clean-up previous resources */
    return EINVALID;
  }

  DLOG(1, "entry=%#x _start=%#x _end_entry=%#x\n",
       entry,
       program_find_symbol(pstart, "_start")->st_value,
       program_find_symbol(pstart, "_end_entry")->st_value);

  /*** Find end of entry stub */
  sym = program_find_symbol(pstart, "_end_entry");
  if(sym == NULL) {
    DLOG(1, "unable to find _end_entry\n");
    return EINVALID;
  }

  p->entry = (void *) entry;
  p->end_entry = (void *) sym->st_value;

  Elf32_Shdr *data = program_find_section(pstart, ".data", SHT_PROGBITS);
#if SCHED==rms || SCHED==rms_sched
  /*** Obtain scheduler parameters */
  /* Capacity */
  sym = program_find_symbol(pstart, "_scheduler_capacity");
  if(sym == NULL) {
    DLOG(1, "unable to find _scheduler_capacity\n");
    return EINVALID;
  }
  p->c = *((u32 *) (pstart + data->sh_offset + (sym->st_value - data->sh_addr)));
  /* Period */
  sym = program_find_symbol(pstart, "_scheduler_period");
  if(sym == NULL) {
    DLOG(1, "unable to find _scheduler_period\n");
    return EINVALID;
  }
  p->t = *((u32 *) (pstart + data->sh_offset + (sym->st_value - data->sh_addr)));
  DLOG(1, "capacity=%#x period=%#x\n", p->c, p->t);
#endif
  /* CPU affinity */
  sym = program_find_symbol(pstart, "_scheduler_affinity");
  if(sym == NULL) {
    DLOG(1, "unable to find _scheduler_affinity -- assuming CPU0\n");
    p->affinity = 1;
  } else
    p->affinity = *((u32 *) (pstart + data->sh_offset + (sym->st_value - data->sh_addr)));
  DLOG(1, "affinity=%#x\n", p->affinity);

  /*** Setup context for scheduling */
  for(i=0;i<sizeof(context_t)>>2;i++)
    ((u32 *)&p->ctxt)[i] = 0;

  p->ctxt.lr = (u32) entry;        /* starting address */
  p->ctxt.usr.r[15] = (u32) entry; /* starting address */
  p->ctxt.psr = MODE_SYS;          /* starting status register */

  /*** Describe program region for the benefit of memory management */
  region_list_t *rl = region_list_pool_alloc();
  if(rl == NULL) {
    DLOG(1, "process_new: region_list_pool_alloc failed.\n");
    /* FIXME: clean-up previous resources */
    return ENOSPACE;
  }
  rl->elt.pstart = region_phys;
  rl->elt.vstart = (void *) base;
#ifdef USE_VMM
  pagetable_t *l2 = &p->tables->next->elt;
  rl->elt.pt = l2;
#else
  rl->elt.pt = NULL;
#endif
  rl->elt.page_count = pages;
  rl->elt.page_size_log2 = PAGE_SIZE_LOG2;

  rl->elt.cache_buf = R_C | R_B; /* cached and buffered */
  rl->elt.shared_ng = R_NG;      /* not global */

  rl->elt.access = R_RW;         /* read-write user mode */
  region_append(&p->regions, rl);

#ifdef USE_VMM
  vmm_map_region(&rl->elt);     /* Create region in process's pagetables */

  //DLOG_DUMP(1, &l2->ptvaddr[0], &l2->ptvaddr[16]);

  process_switch_to(p);
  u32 paddr;
  if(vmm_get_phys_addr((void *) 0x8000, &paddr) != OK)
    DLOG(1, "vmm_get_phys_addr failed\n");
  else
    DLOG(1, "vmm_get_phys_addr(0x8000) = %#x\n", paddr);
  /* *** cheat ***, switch to process and use its mapping */

#endif

  vmm_dump_region(&rl->elt);

  /*** Copy sections into their final memory locations */

  u8 *dest, *src;
  Elf32_Shdr *sh = (void *) pe + pe->e_shoff; /* start at first section */

  /* loop through sections */
  for(i=0; i<pe->e_shnum; i++) {
    if((sh->sh_flags & SHF_ALLOC) && sh->sh_size > 0) { /* if it is "in memory" */
      if(sh->sh_type == SHT_PROGBITS) {
        /* PROGBITS means: data that the program needs to be copied verbatim */
        dest = (u8 *) sh->sh_addr;
        src = pstart + sh->sh_offset;
        DLOG(1, "program_load: copying section=%d (%s) bytes=%d src=%#x dest=%#x\n",
             i, program_find_string_in(pstart, program_find_section_index(pstart, pe->e_shstrndx), sh->sh_name),
             sh->sh_size, src, dest);
        memcpy(dest, src, sh->sh_size);
      } else if(sh->sh_type == SHT_NOBITS) {
        /* NOBITS means: just allocate empty space and zero it out */
        dest = (u8 *) sh->sh_addr;
        DLOG(1, "program_load: zeroing section=%d (%s) bytes=%d dest=%#x\n",
             i, program_find_string_in(pstart, program_find_section_index(pstart, pe->e_shstrndx), sh->sh_name),
             sh->sh_size, dest);
        memset(dest, 0, sh->sh_size);
      } else
        DLOG(1, "program_load: not bothering with section=%d (%s) type=%#x\n",
             i, program_find_string_in(pstart, program_find_section_index(pstart, pe->e_shstrndx), sh->sh_name),
             sh->sh_type);
    }

    /* go to next section header */
    sh = (void *) sh + pe->e_shentsize;
  }

  *return_p = p;

  /*** Ensure programs are written to main memory */
  data_mem_barrier();
  arm_cache_clean_invl_data();

  return OK;
}

#if SCHED==rms || SCHED==rms_sched
#ifdef OMAP4460
DEF_PER_CPU_EXTERN(u32, prev_sched);
#else
#error "prev_sched not implemented"
#endif
#endif

#if SCHED==rms || SCHED==rms_sched
#ifdef OMAP4460
void show_program_params(void)
{
  int i;
  DLOG(1, "Program scheduling parameters:\n");
  for(i=0;i<MAX_PROCESSES;i++) {
    process_t *p = &process[i];
    if(p->pid != NOPID) {
      u32 u = 100 * p->c / p->t;
      DLOG(1, "  pid=%d C=%d T=%d U=%d%%\n", p->pid, p->c, p->t, u);
    }
  }
}


#else
#error "show_program_params not implemented"
#endif
#endif

status programs_init(void)
{
  extern u32 _program_map_start, _program_map_count;
  u32 *progs = (u32 *) &_program_map_start, cnt = (u32) &_program_map_count;
  process_t *p;
  int i;

  if(cnt < 1) {
    DLOG(1, "programs_init: requires at least 1 program to run.\n");
    return EINVALID;
  }

  for(i=0;i<cnt;i++) {
    DLOG(1, "programs_init: loading program found at %#x\n", progs[i]);
    if(program_load((void *) progs[i], &p) != OK) {
      DLOG(1, "program_load failed\n");
      return EINVALID;
    }
    sched_wakeup(p);
  }

  extern semaphore_t scheduling_enabled_sem;
  extern u32 num_cpus;
  /* tell waiting aux CPUs that it's time to wake up */
  for(i=0;i<num_cpus - 1;i++) semaphore_up(&scheduling_enabled_sem);

  /* start with idle process */
  cpu_write(process_t *, current, cpu_read(process_t *, cpu_idle_process));

#if 0

#define TEST2

  /* play with values */
#ifdef TEST1
  process[0].c = 2 << 14;
  process[0].t = 10 << 14;

  process[1].c = 3 << 14;
  process[1].t = 12 << 14;

  process[2].c = 4 << 14;
  process[2].t = 15 << 14;
#endif
#ifdef TEST2
  process[0].c = 2 << 14;
  process[0].t = 11 << 14;

  process[1].c = 3 << 14;
  process[1].t = 13 << 14;

  process[2].c = 4 << 14;
  process[2].t = 16 << 14;
#endif

#endif

#if SCHED==rms || SCHED==rms_sched
  show_program_params();
#endif

  DLOG(1, "Switching to idle process. entry=%#x\n", cpu_read(process_t *, current)->entry);

#if SCHED==rms || SCHED==rms_sched
#ifdef OMAP4460
  cpu_write(u32, prev_sched, timer_32k_value() - 1);
#else
#error "prev_sched not implemented"
#endif
#endif

  sched_launch_first_process(cpu_read(process_t *, cpu_idle_process));

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
