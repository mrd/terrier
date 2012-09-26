/*
 * Rate monotonic scheduler C code
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

#ifdef OMAP4460

#include "ats_types.h"
#include "ats_basics.h"
#include "types.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "arm/smp/spinlock.h"
#include "arm/smp/per-cpu.h"
#include "omap/timer.h"
#include "sched/process.h"
#include "intr/interrupts.h"
#define MODULE "sched_rms"
#include "debug/log.h"

void *atspre_null_ptr = 0;      //FIXME
ats_bool_type atspre_peq(ats_ptr_type a, ats_ptr_type b)
{
  return a == b;
}
ats_int_type atspre_iadd(ats_int_type a, ats_int_type b)
{
  return a + b;
}
ats_int_type atspre_add_int_int(ats_int_type a, ats_int_type b)
{
  return a + b;
}
ats_int_type atspre_isub(ats_int_type a, ats_int_type b)
{
  return a - b;
}
ats_int_type atspre_sub_int_int(ats_int_type a, ats_int_type b)
{
  return a - b;
}
ats_bool_type atspre_lt_int_int(ats_int_type a, ats_int_type b)
{
  return a < b;
}
ats_bool_type atspre_ilt(ats_int_type a, ats_int_type b)
{
  return a < b;
}
ats_bool_type atspre_ieq(ats_int_type a, ats_int_type b)
{
  return a == b;
}
ats_bool_type atspre_eq_int_int(ats_int_type a, ats_int_type b)
{
  return a == b;
}
#define ATS_MALLOC(x) ((void *) NULL)

extern process_t process[];
#define process_id(p) ((process_t *) (p))->pid
#define get_process(i) ((void *) &process[i - 1])
#define rel_process(p)

#define get_capacity(p) ((process_t *) (p))->c
#define get_period(p) ((process_t *) (p))->t

#define get_budget(p) ((process_t *) (p))->b
#define set_budget(p,v) ((process_t *) (p))->b = (v)

#define get_replenish_time(p) ((process_t *) (p))->r
#define set_replenish_time(p, v) ((process_t *) (p))->r = (v)

/* signed arithmetic to handle wraparound */
#define is_earlier_than(n,m) ((u32) !!(((s32) n) - ((s32) m) < (s32) 0))

#define pqueue_is_empty(q) (*((pid_t *) q) == NOPID)
#define pqueue_is_not_empty(q) (*((pid_t *) q) != NOPID)

DEF_PER_CPU(pid_t, cpu_runq_head); /* cpu runqueue: may hold unsaved processes */
INIT_PER_CPU(cpu_runq_head) { cpu_write(pid_t, cpu_runq_head, NOPID); }

DEF_PER_CPU(u32, prev_sched);   /* 32khz clock time when sched previously ran */
INIT_PER_CPU(prev_sched) { cpu_write(u32, prev_sched, 0); }

DEF_PER_CPU(process_t *, current);
INIT_PER_CPU(current) { cpu_write(process_t *, current, NULL); }
DEF_PER_CPU(context_t *, _next_context);
INIT_PER_CPU(_next_context) { cpu_write(process_t *, _next_context, NULL); }
DEF_PER_CPU(context_t *, _prev_context);
INIT_PER_CPU(_prev_context) { cpu_write(process_t *, _prev_context, NULL); }

DEF_PER_CPU_EXTERN(process_t *, cpu_idle_process);

#define get_current() ((void *) (cpu_read(process_t *, current)))

#define QUANTUM (1<<12)

/* precondition: either q is protected by a lock that is held, or q is
 * only usable by this CPU. */
static status waitqueue_append(pid_t *q, process_t *p)
{
  p->next = NOPID;
  for(;;) {                     /* tail-recursion elided */
    if(*q == NOPID) {
      *q = p->pid;
      return OK;
    } else {
      process_t *qp = process_find(*q);
      if(qp == NULL)
        return EINVALID;
      q = &qp->next;
    }
  }
}

/* precondition: either q is protected by a lock that is held, or q is
 * only usable by this CPU. */
static process_t *waitqueue_dequeue(pid_t *q)
{
  process_t *p;
  if(*q == NOPID)
    return NULL;
  p = process_find(*q);
  if(p == NULL)
    return NULL;
  *q = p->next;
  return p;
}

static u32 update_prev_sched(void)
{
  u32 prev = cpu_read(u32, prev_sched), now = timer_32k_value (), diff;
  cpu_write(u32, prev_sched, now);
  diff = now - prev;            /* unsigned arithmetic handles wraparound correctly */
  return diff;
}

void schedule(void);

static void sched_timer_handler(u32 intid)
{
  if(pvttimer_is_triggered()) {
    DLOG(1, "sched_timer_handler intid=%#x\n", intid);
    pvttimer_ack_interrupt();   /* acknowledge and unmask */

    schedule();
  }
}

void sched_init(void)
{
  DLOG(1, "init: QUANTUM=%#x\n", QUANTUM);
  pvttimer_set_handler(sched_timer_handler);
  pvttimer_set(QUANTUM);
  pvttimer_enable_interrupt();
  pvttimer_start();
  //runq_head = NOPID;
}

void sched_aux_cpu_init(void)
{
  pvttimer_set_handler(sched_timer_handler);
  pvttimer_set(QUANTUM);
  pvttimer_enable_interrupt();
  pvttimer_start();
}

void do_idle(void)
{
  DLOG(1, "do_idle\n");
  cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
  cpu_write(process_t *, current, cpu_read(process_t *, cpu_idle_process));
  cpu_write(context_t *, _next_context, &cpu_read(process_t *, cpu_idle_process)->ctxt);
}

void do_process_switch(void *pptr)
{
  process_t *p = (process_t *) pptr;
  /* invariant: process p runnable by only one CPU at a time */
  cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
  cpu_write(process_t *, current, p);
  process_switch_to(p);
  cpu_write(context_t *, _next_context, &p->ctxt);
  DLOG(1, "switch_to: pid=%d pc=%#x\n", p->pid, p->ctxt.usr.r[15]);
}

#else
#error "RMS not implemented for non-OMAP4460"
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
