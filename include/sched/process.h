/*
 * Scheduler processes
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

#ifndef _SCHED_PROCESS_H_
#define _SCHED_PROCESS_H_

#include "types.h"
#include "mem/virtual.h"
#include "arm/smp/per-cpu.h"

typedef u32 pid_t;
#define MAX_PROCESSES 16
#define NOPID ((pid_t) 0)

#define VIRTUAL_BASE_ADDR 0x8000 /* if using VMM */

typedef struct {
  u32 psr, lr;
  struct {
    u32 r[16];
  } usr;
} context_t;

typedef struct {
  context_t ctxt;
  void *entry;
  void *end_entry;
  pid_t pid;
  pagetable_list_t *tables;
  region_list_t *regions;
#if SCHED == rms
  u32 r, b, c, t;
#endif
  pid_t next;
} process_t;

process_t *process_find(u32 pid);
status process_switch_to(process_t *p);
status process_new(process_t **return_p);
void process_init(void);
status program_load(void *pstart, process_t **return_p);

DEF_PER_CPU_EXTERN(process_t *, current);

/* must-inline */
#define sched_launch_first_process(p) do {                              \
    ASM(/* load context */                                              \
        "LDMIA   %0!, {r0,lr}\n"          /* load status register, return address */ \
        "MSR     spsr_cxsf, r0\n"         /* prep saved process status register */ \
        "LDMIA   %0, {r0-r14}^\n"         /* load user registers (incl. r13_usr, r14_usr) */ \
        /* and return to userspace */                                   \
        "LDR     r13, =svc_stack_top\n"   /* r13 = &svc_stack_top */    \
        "MOVS    pc, lr"::"r"(&p->ctxt):"r0");   /* jump to userspace */ \
  } while(0)

#define sched_launch_first_process_no_stack_patchup(p) do {             \
    ASM(/* load context */                                              \
        "LDMIA   %0!, {r0,lr}\n"          /* load status register, return address */ \
        "MSR     spsr_cxsf, r0\n"         /* prep saved process status register */ \
        "LDMIA   %0, {r0-r14}^\n"         /* load user registers (incl. r13_usr, r14_usr) */ \
        /* and return to userspace */                                   \
        "MOVS    pc, lr"::"r"(&p->ctxt):"r0");   /* jump to userspace */ \
  } while(0)

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
