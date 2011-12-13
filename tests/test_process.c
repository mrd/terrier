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
#include "arm/asm.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "sched/process.h"
#define MODULE "test_process"
#include "debug/log.h"

status test_process(void)
{
  process_t *p;
  if(process_new(&p) != OK) {
    DLOG(1, "process_new failed\n");
    return EINVALID;
  }

  DLOG(1, "process region pstart=%#x\n", p->regions->elt.pstart);
  region_t rtmp = { p->regions->elt.pstart, NULL, &kernel_l2pt, 4, PAGE_SIZE_LOG2, R_C | R_B, 0, R_PM };
  /* map to kernel virtual memory */
  if(vmm_map_region_find_vstart(&rtmp) != OK) {
    DLOG(1, "test_process: vmm_map_region_find_vstart for user region failed.\n");
    return ENOSPACE;
  }
  ((u32 *) rtmp.vstart)[0] = 0xef00007b; /* SVC #123 */
  ((u32 *) rtmp.vstart)[1] = 0xeafffffe; /* B . */

  process_switch_to(p);
  DLOG(1, "0x1000: %#x\n", *((u32 *) 0x1000));

  DLOG(1, "Switching to usermode. Expect SWI then halt:\n");

  u32 spsr;
  ASM("MRS %0,spsr\n"
      "AND %0, %0, #0xE0\n"
      "ORR %0, %0, #0x10\n"
      "MSR spsr_c, %0":"=r"(spsr));

  ASM("STMFD sp!, {%0}\n"
      "LDMFD sp!, {pc}^"::"r"(0x1000));

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
