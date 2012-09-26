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

#if SCHED==rms
      p->r = p->b = 0;
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

u32 program_memory_size(void *pstart)
{
  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);
  Elf32_Shdr *data = program_find_section(pstart, ".data", SHT_PROGBITS);
  Elf32_Shdr *bss  = program_find_section(pstart, ".bss", SHT_NOBITS);

  return text->sh_size + data->sh_size + bss->sh_size;
}

status program_relocate(void *pstart, u32 base, u32 *entry)
{
  int i;
  Elf32_Sym *sym;
  Elf32_Ehdr *pe = (Elf32_Ehdr *) pstart;
  /* Extract sections and create layout */

  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);
  Elf32_Shdr *reltext = program_find_section(pstart, ".rel.text", SHT_REL);
  /* Elf32_Shdr *relatext = program_find_section(pstart, ".rela.text", SHT_RELA); */
  Elf32_Shdr *data = program_find_section(pstart, ".data", SHT_PROGBITS);
  Elf32_Shdr *bss  = program_find_section(pstart, ".bss", SHT_NOBITS);

#define CHK_SEC(var) if(var == NULL) { DLOG(1, "Unable to find section %s.\n", #var); return EINVALID; }
  CHK_SEC(text);
  CHK_SEC(reltext);
  CHK_SEC(data);
  CHK_SEC(bss);

#define SECTION_INDEX(s) (((u32) (s) - (u32) pe - pe->e_shoff) / pe->e_shentsize)
  u32 textshnum = SECTION_INDEX(text);
  u32 datashnum = SECTION_INDEX(data);
  u32 bssshnum  = SECTION_INDEX(bss);
  DLOG(1, "section indices: text=%d data=%d bss=%d\n", textshnum, datashnum, bssshnum);

  /* Calculate addresses for .text, .data, .bss */

  u32 textbase = base;
  u32 database = textbase + text->sh_size;
  u32 bssbase  = database + data->sh_size;
  DLOG(1, "base addresses: text=%#x data=%#x bss=%#x\n", textbase, database, bssbase);
  text->sh_addr = textbase;
  data->sh_addr = database;
  bss->sh_addr  = bssbase;

  /* Rewrite symtab based on shndx, and put symbols into proper spaces */

  Elf32_Shdr *symtabsh = program_find_section(pstart, ".symtab", SHT_SYMTAB);
  CHK_SEC(symtabsh);

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
      DLOG_NO_PREFIX(1, "ABS32: A=%#x S=%#x result=%#x\n", A, S, *ptr);
      break;
    case R_ARM_CALL:            /* branch with link instruction */
      P = textbase + off;
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
    default:
      DLOG_NO_PREFIX(1, "Unknown relocation type.\n");
      break;
    }
  }

  *entry = textbase + pe->e_entry;
  return OK;
}

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

  Elf32_Shdr *text = program_find_section(pstart, ".text", SHT_PROGBITS);
  Elf32_Shdr *data = program_find_section(pstart, ".data", SHT_PROGBITS);
  Elf32_Shdr *bss  = program_find_section(pstart, ".bss", SHT_NOBITS);

  u32 memsz = program_memory_size(pstart);
  if(memsz == 0) {
    DLOG(1, "program_memory_size == 0\n");
    return EINVALID;
  }

  u32 pages = (memsz + 0xFFF) >> PAGE_SIZE_LOG2, region_phys;
  u32 align = (text->sh_addralign <= 0x1000 ? 1 : text->sh_addralign >> 12);

  /* Create region in physical memory for program */

  if(physical_alloc_pages(pages, align, &region_phys) != OK) {
    DLOG(1, "process_new: physical_alloc_pages for region failed.\n");
    /* FIXME: clean-up previous resources */
    return ENOSPACE;
  }

  /* Select base address */
#ifdef USE_VMM
  u32 base = VIRTUAL_BASE_ADDR;
#else
  u32 base = region_phys;
#endif

  if(program_relocate(pstart, base, &entry) != OK) {
    DLOG(1, "program_relocate(%#x, %#x) failed.\n", pstart, region_phys);
    /* FIXME: clean-up previous resources */
    return EINVALID;
  }

  /* Create process */

  if(process_new(&p) != OK) {
    DLOG(1, "process_new failed\n");
    /* FIXME: clean-up previous resources */
    return EINVALID;
  }

  DLOG(1, "entry=%#x _start=%#x _end_entry=%#x\n",
       entry,
       program_find_symbol(pstart, "_start")->st_value,
       program_find_symbol(pstart, "_end_entry")->st_value);

  sym = program_find_symbol(pstart, "_end_entry");
  if(sym == NULL) {
    DLOG(1, "unable to find _end_entry\n");
    return EINVALID;
  }

  p->entry = (void *) entry;
  p->end_entry = (void *) sym->st_value;

  /* setup context */
  for(i=0;i<sizeof(context_t)>>2;i++)
    ((u32 *)&p->ctxt)[i] = 0;

  p->ctxt.lr = (u32) entry;        /* starting address */
  p->ctxt.usr.r[15] = (u32) entry; /* starting address */
  p->ctxt.psr = MODE_SYS;          /* starting status register */

#ifdef USE_VMM
  pagetable_t *l2 = &p->tables->next->elt;
  region_list_t *rl = region_list_pool_alloc();
  if(rl == NULL) {
    DLOG(1, "process_new: region_list_pool_alloc failed.\n");
    /* FIXME: clean-up previous resources */
    return ENOSPACE;
  }
  rl->elt.pstart = region_phys;
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
#endif

  u8 *dest, *src;

  /* copy text */
  dest = (u8 *) text->sh_addr;
  src = pstart + text->sh_offset;
  DLOG(1, "program_load: copying %d bytes src=%#x dest=%#x\n", text->sh_size, src, dest);
  for(i=0;i<text->sh_size;i++)
    dest[i] = src[i];

  /* copy data */
  dest = (u8 *) data->sh_addr;
  src = pstart + data->sh_offset;
  DLOG(1, "program_load: copying %d bytes src=%#x dest=%#x\n", data->sh_size, src, dest);
  for(i=0;i<data->sh_size;i++)
    dest[i] = src[i];

  /* zero bss */
  dest = (u8 *) bss->sh_addr;
  DLOG(1, "program_load: zeroing %d bytes dest=%#x\n", bss->sh_size, dest);
  for(i=0;i<bss->sh_size;i++)
    dest[i] = 0;

  *return_p = p;

  /* ensure programs are written to main memory */
  data_mem_barrier();
  arm_cache_clean_invl_data();

  return OK;
}

#if SCHED==rms
#ifdef OMAP4460
DEF_PER_CPU_EXTERN(u32, prev_sched);
#else
#error "prev_sched not implemented"
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

  DLOG(1, "Switching to idle process. entry=%#x\n", cpu_read(process_t *, current)->entry);

#if SCHED==rms
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
