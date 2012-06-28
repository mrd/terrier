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
#include "mem/virtual.h"
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
static u32 *tempstack;
u32 num_cpus;

/* First real function for auxiliary CPUs. */
static void NO_INLINE smp_aux_cpu_init()
{
  u32 mpidr, actlr;
  stage = 1;
  while(stage==1);

  mpidr = arm_multiprocessor_affinity(), actlr = arm_aux_control(0, ~0);
  DLOG(1,"HELLO WORLD from cpu%d!\n", GETBITS(mpidr,0,2));
  DLOG(1,"MPIDR=%#x %s cpuid=%d\n", mpidr,
       GETBITS(mpidr,30,1) ?"(uniprocessor)" : "(mpcore)",
       GETBITS(mpidr,0,2));
  DLOG(1,"ACTLR=%#x smp=%d\n", actlr, GETBITS(actlr, 6, 1));
  DLOG(1, "Stage %d\n", stage);

  stage=3;
  while(stage==3);

  arm_aux_control(BIT(6), BIT(6)); /* set SMP */
  arm_ctrl(CTRL_DCACHE | CTRL_ICACHE, /* enable caches */
           CTRL_DCACHE | CTRL_ICACHE);

  stage=5;

  /* complete remaining init asynchronously */
  /* ... */
}

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
  /* set up the temporary call stack */
  ASM("mov sp, %0"::"r" (&tempstack[1024]));
  /* call into a real function frame */
  smp_aux_cpu_init();
  for(;;);
}

/* See also 5.3.4 Cortex-A9 MPCore: Multiprocessor bring-up */

status smp_init(void)
{
  u32 mpidr = arm_multiprocessor_affinity(), actlr = arm_aux_control(0, ~0), i;
  DLOG(1,"init\n");

  /* Snoop Control Unit registers come first in PERIPHBASE map. */
  SCU = (void *) arm_config_base_address();

  /* find number of CPUs */
  num_cpus = GETBITS(SCU->configuration, 0, 2) + 1;
  if(num_cpus > MAX_CPUS) {
    DLOG(1, "WTF: num_cpus=%d > MAX_CPUS=%d\n", num_cpus, MAX_CPUS);
    return EINVALID;
  }

  /* remember: even if we are using virtual addresses, the auxiliary
   * CPU is not yet configured for that. */
  tempstack = (u32 *) physical_alloc_page();
  if(tempstack == NULL) {
      DLOG(1, "Unable to allocate stack.\n");
      return ENOSPACE;
  }

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

    /* 27.4.4.1 TRM OMAP4460: Startup */
    AUX_CORE_BOOT[1] = (u32) &smp_aux_entry_point; /* physical address of starting point */
    AUX_CORE_BOOT[0] = ~0;                         /* toggle status flag */

    DLOG(1, "AUX_CORE_BOOT(%#x)=%#x, %#x\n", AUX_CORE_BOOT, AUX_CORE_BOOT[0], AUX_CORE_BOOT[1]);

    data_sync_barrier();
    ASM("SEV");                   /* set-event: wake up waiting CPUs */

    while(stage==0);

    SCU->control |= 1;            /* enable SCU */

    DLOG(1, "Stage %d\n", stage);
    stage=2;
    while(stage==2);

    arm_aux_control(BIT(6), BIT(6)); /* set SMP */

    DLOG(1, "Stage %d\n", stage);
    stage=4;
    while(stage==4);
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
