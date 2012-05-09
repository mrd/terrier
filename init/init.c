/*
 * Begin initialization in C.
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
#include "omap3/early_uart3.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "mem/virtual.h"

void perfmon_init(void)
{
  arm_perfmon_pmnc(PERFMON_PMNC_E, PERFMON_PMNC_E);
  arm_perfmon_cntens(PERFMON_CNTENS_C);
}

void NO_RETURN c_entry()
{
  void identify_device(void);
  void physical_init(void);
  void intr_init(void);
  void timer_init(void);
  void vmm_init(void);
  void process_init(void);
  void sched_init(void);
  status programs_init(void);

#ifdef USE_VMM
  extern pagetable_t l1pt;
  extern void *_physical_start;
  u32 phystart = (u32) &_physical_start;

  l1pt.ptvaddr[phystart >> 20] = 0; /* unmap stub */
  arm_mmu_flush_tlb();
#endif

  perfmon_init();
  reset_uart3();
  identify_device();
  physical_init();
  intr_init();
  timer_init();
#ifdef USE_VMM
  vmm_init();
#endif
  process_init();
  sched_init();

#ifdef TEST_CSWITCH
  status test_cswitch(void); test_cswitch();
#endif
#ifdef TEST_PROCESS
  status test_process(void); test_process();
#endif

  programs_init();

  print_uart3("-- HALTED --\n");

  for(;;) arm_wait_for_interrupt();
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
