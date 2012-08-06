/*
 * OMAP SMP Support
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
#include "intr/interrupts.h"
#include "omap/early_uart3.h"
#include "omap/smp.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/physical.h"
#include "mem/virtual.h"
#include "arm/smp/spinlock.h"
#include "arm/smp/per-cpu.h"
#define MODULE "smp"
#include "debug/log.h"
#include "debug/cassert.h"

#ifdef OMAP4460

/* 4.4.9 TRM OMAP4460: Wake-up generator registers */
static u32 *AUX_CORE_BOOT = (void *) 0x48281800;

/* 2.2 Cortex-A9 MPCore: SCU Registers */
PACKED_STRUCT(scu) {
  PACKED_FIELD(u32, control);
  PACKED_FIELD(u32, configuration);
  PACKED_FIELD(u32, cpu_power_status);
  PACKED_FIELD(u32, invalidate_all_registers_in_secure_state);
  PACKED_FIELD(u32, _rsvd1[12]);
  PACKED_FIELD(u32, filtering_start_address);
  PACKED_FIELD(u32, filtering_end_address);
  PACKED_FIELD(u32, _rsvd2[2]);
  PACKED_FIELD(u32, access_control);
  PACKED_FIELD(u32, non_secure_access_control);
} PACKED_END;
CASSERT(offsetof(struct scu, filtering_start_address) == 0x40, scu);
CASSERT(offsetof(struct scu, non_secure_access_control) == 0x54, scu);

static volatile struct scu *SCU;
static volatile u32 stage, curboot;
static u32 *newstack[5];        /* hold stack addresses for secondary processor */
u32 num_cpus;
status smp_init_per_cpu_spaces(void);
status smp_init_invoke_percpu_constructors(void);

static void *alloc_stack(void)
{
  physaddr pstart;
  if(physical_alloc_pages(1, 1, &pstart) != OK) {
    DLOG(1,"alloc_stack: unable to allocate physical page\n");
    return NULL;
  }

#ifdef USE_VMM
  region_t rtmp = { pstart, NULL, &kernel_l2pt, 1, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
  if(vmm_map_region_find_vstart(&rtmp) != OK) {
    DLOG(1, "smp_init_per_cpu_spaces: vmm_map_region_find_vstart failed.\n");
    /* FIXME: clean-up previous resources */
    return NULL;
  }
  return rtmp.vstart;
#else
  return (void *) pstart;
#endif
}

/* First real function for auxiliary CPUs. */
static void NO_INLINE smp_aux_cpu_init()
{
  u32 mpidr, actlr;
  mpidr = arm_multiprocessor_affinity(), actlr = arm_aux_control(0, ~0);

  arm_cache_clean_invl_data();
  stage = 1;
  while(stage==1) arm_cache_clean_invl_data();

  mpidr = arm_multiprocessor_affinity(), actlr = arm_aux_control(0, ~0);
  DLOG(1,"HELLO WORLD from cpu%d!\n", GETBITS(mpidr,0,2));
  DLOG(1,"MPIDR=%#x %s cpuid=%d\n", mpidr,
       GETBITS(mpidr,30,1) ?"(uniprocessor)" : "(mpcore)",
       GETBITS(mpidr,0,2));
  DLOG(1,"ACTLR=%#x smp=%d\n", actlr, GETBITS(actlr, 6, 1));
  DLOG(1, "Stage %d\n", stage);

  stage=3;
  while(stage==3) arm_cache_clean_invl_data();

  arm_aux_control(BIT(6), BIT(6)); /* set SMP */
  arm_ctrl(CTRL_DCACHE | CTRL_ICACHE, /* enable caches */
           CTRL_DCACHE | CTRL_ICACHE);

  stage=5;

  /* complete remaining init asynchronously */

#ifdef USE_VMM
  arm_mmu_flush_tlb();
#endif

  void perfmon_init(void);
  perfmon_init();
  extern void *_vector_table;
  arm_set_vector_base_address((u32) &_vector_table);
  void intc_secondary_cpu_init(void);
  intc_secondary_cpu_init();
  smp_init_invoke_percpu_constructors();
  arm_mmu_ttbcr(SETBITS(2,0,3), MMU_TTBCR_N | MMU_TTBCR_PD0 | MMU_TTBCR_PD1);

  /* ... */
}

extern void *_l1table_phys;

/* Beginning of the world for auxiliary CPUs. */
static void NAKED NO_RETURN smp_aux_entry_point(void)
{
  /* Quickly check if this is the current CPU being booted; if not,
   * then go back to sleep. */
  ASM("1:\n"
      "MRC p15,0,r0,c0,c0,5\n"
      "AND r0,r0,#3\n"
      "CMP r0,%0\n"
      "WFENE\n"
      "BNE 1b"::"r"(curboot));

  data_sync_barrier();

  /* Be strict about register usage and asm output here; the C
   * compiler doesn't know about register banking */
  register u32 fiqstk asm("r0") = (u32) &newstack[0][1024];
  register u32 irqstk asm("r1") = (u32) &newstack[1][1024];
  register u32 undstk asm("r2") = (u32) &newstack[2][1024];
  register u32 abtstk asm("r3") = (u32) &newstack[3][1024];
  register u32 svcstk asm("r4") = (u32) &newstack[4][1024];

  /* setup mode stacks */
  ASM("CPS %0; MOV sp, %1\n"
      "CPS %2; MOV sp, %3\n"
      "CPS %4; MOV sp, %5\n"
      "CPS %6; MOV sp, %7\n"
      "CPS %8; MOV sp, %9"::
      "i"(MODE_FIQ),"r"(fiqstk),
      "i"(MODE_IRQ),"r"(irqstk),
      "i"(MODE_ABT),"r"(abtstk),
      "i"(MODE_UND),"r"(undstk),
      "i"(MODE_SVC),"r"(svcstk));

  /* Mode: SVC */

  /* call into a real function frame */
  smp_aux_cpu_init();

  ASM("CPS %0"::"i"(MODE_SYS));

  for(;;);
}

#ifdef USE_VMM
static void NAKED NO_RETURN smp_aux_vmm_entry_point(void)
{
  arm_mmu_flush_tlb();
  arm_mmu_domain_access_ctrl(~0, ~0); /* set all domains = MANAGER */
  arm_mmu_set_ttb0((physaddr) &_l1table_phys);
  arm_mmu_set_ttb1((physaddr) &_l1table_phys);

  /* Enable MMU */
  arm_ctrl(CTRL_MMU // | CTRL_DCACHE | CTRL_ICACHE
          ,CTRL_MMU | CTRL_DCACHE | CTRL_ICACHE);

  /* jump to high memory */
  ASM("MOV pc, %0"::"r"(smp_aux_entry_point));
}
#endif

/* See also 5.3.4 Cortex-A9 MPCore: Multiprocessor bring-up */

status smp_init(void)
{
  u32 mpidr = arm_multiprocessor_affinity(), actlr = arm_aux_control(0, ~0), i, j;
  DLOG(1,"init\n");

  /* Snoop Control Unit registers come first in PERIPHBASE map. */
  SCU = (void *) arm_config_base_address();

  /* find number of CPUs */
  num_cpus = GETBITS(SCU->configuration, 0, 2) + 1;
  if(num_cpus > MAX_CPUS) {
    DLOG(1, "WTF: num_cpus=%d > MAX_CPUS=%d\n", num_cpus, MAX_CPUS);
    return EINVALID;
  }

  if(smp_init_per_cpu_spaces() != OK) return ENOSPACE;
  smp_init_invoke_percpu_constructors();

  SCU->invalidate_all_registers_in_secure_state = ~0; /* invalidate tag RAM */

  DLOG(1, "MPIDR=%#x %s cpuid=%d\n", mpidr,
       GETBITS(mpidr,30,1) ?"(uniprocessor)" : "(mpcore)",
       GETBITS(mpidr,0,2));
  DLOG(1, "ACTLR=%#x smp=%d\n", actlr, GETBITS(actlr, 6, 1));

  /* cycle through auxiliary CPUs and boot them */
  for(i=1;i<num_cpus;i++) {
    curboot = i;
    stage = 0;
    DLOG(1, "Booting cpu%d\n", i);

    /* create stacks for CPU modes */
    for(j=0;j<sizeof(newstack)/sizeof(newstack[0]);j++) {
      newstack[j] = alloc_stack();
      if(newstack[j] == NULL)
        return ENOSPACE;
      DLOG(1, "&newstack[%d][1024]=%#x\n", j, &newstack[j][1024]);
    }

    /* 27.4.4.1 TRM OMAP4460: Startup */
#ifdef USE_VMM
    /* physical address of starting point */
    if(vmm_get_phys_addr(&smp_aux_vmm_entry_point, &AUX_CORE_BOOT[1]) != OK) {
      DLOG(1, "Unable to get physical address of smp_aux_vmm_entry_point.\n");
      return EINVALID;
    }
#else
    AUX_CORE_BOOT[1] = (u32) &smp_aux_entry_point; /* physical address of starting point */
#endif
    AUX_CORE_BOOT[0] = ~0;                         /* toggle status flag */

    arm_cache_clean_data(); /* stage system won't work without this flush */
    data_sync_barrier();

    ASM("SEV");                   /* set-event: wake up waiting CPUs */

    while(stage==0) arm_cache_clean_invl_data();

    SCU->control |= 1;            /* enable SCU */

    DLOG(1, "Stage %d\n", stage);
    stage=2;
    while(stage==2) arm_cache_clean_invl_data();

    arm_aux_control(BIT(6), BIT(6)); /* set SMP */

    DLOG(1, "Stage %d\n", stage);
    stage=4;
    while(stage==4) arm_cache_clean_invl_data();
  }

  u32 scucfg = SCU->configuration;
  DLOG(1, "Booted all %d processors\n", num_cpus);
  DLOG(1, "SCU control=%#x config=%#x num_cpus=%d\n", SCU->control, scucfg, num_cpus);
  DLOG(1, "Taking part in SMP and coherency: %s%s%s%s\n",
       GETBITS(scucfg, 4, 1) ? "cpu0 " : "",
       GETBITS(scucfg, 5, 1) ? "cpu1 " : "",
       GETBITS(scucfg, 6, 1) ? "cpu2 " : "",
       GETBITS(scucfg, 7, 1) ? "cpu3 " : "");

  DLOG(1, "SCU power status=%#x\n", SCU->cpu_power_status);

  /* ensure caches enabled */
  arm_ctrl(CTRL_DCACHE | CTRL_ICACHE, CTRL_DCACHE | CTRL_ICACHE);

  return OK;
}

/* Per-CPU initialization */

u32 *_per_cpu_spaces[MAX_CPUS];

status smp_init_invoke_percpu_constructors(void)
{
  extern void (*_percpu_ctor_list)();
  void (**ctor) ();
  for (ctor = &_percpu_ctor_list; *ctor; ctor++)
    (*ctor) ();
  return OK;
}

status smp_init_per_cpu_spaces(void)
{
  u32 i,j;
  extern u32 _percpu_pages_plus_one;
  physaddr pstart;

  /* gcc's optimizer doesn't believe a symbol's address can be zero;
   * an if-statement trying to test (num_pages == 0) will end up being
   * elided by the compiler. */
  if((u32) &_percpu_pages_plus_one == 1) return OK;

  u32 num_pages = ((u32) &_percpu_pages_plus_one) - 1;

  for(i=0;i<num_cpus;i++) {
    if(physical_alloc_pages(num_pages, 1, &pstart) != OK) {
      DLOG(1,"smp_init_per_cpu_spaces: unable to allocate physical pages, count=%d\n", num_pages);
      return ENOSPACE;
    }

#ifdef VMM
    region_t rtmp = { pstart, NULL, &kernel_l2pt, 1, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
    if(vmm_map_region_find_vstart(&rtmp) != OK) {
      DLOG(1, "smp_init_per_cpu_spaces: vmm_map_region_find_vstart failed.\n");
      /* FIXME: clean-up previous resources */
      return ENOSPACE;
    }
    _per_cpu_spaces[i] = rtmp.vstart;
#else
    _per_cpu_spaces[i] = (u32 *) pstart;
#endif
    DLOG(1, "smp_init_per_cpu_spaces: cpu=%d space=%#x\n", i, _per_cpu_spaces[i]);

    for(j=0;j<(num_pages << PAGE_SIZE_LOG2);j++)
      _per_cpu_spaces[i][j] = 0;
  }

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
