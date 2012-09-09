/*
 * Round-Robin Scheduler -- simple and straightforward stand-in
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

#define ATS_DYNLOADFLAG 0
#define ATS_STALOADFLAG 0

%{^
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
#define MODULE "sched_rr"
#include "debug/log.h"

void *atspre_null_ptr = 0;      //FIXME
ats_bool_type atspre_peq(ats_ptr_type a, ats_ptr_type b) { return a == b; }

static pid_t runq_head;         /* global runqueue: can only hold cleanly saved processes */
static DEFSPINLOCK(rrlock);

#define get_runq_head() ((void *) &runq_head)
#define rel_runq_head(p)
#define get_cpuq_head() ((void *) (&cpu_read(pid_t, cpu_runq_head)))
#define rel_cpuq_head(p)
#define rrlock_lock() spinlock_lock(&rrlock)
#define rrlock_unlock() spinlock_unlock(&rrlock)
static status waitqueue_append(pid_t *q, process_t *p);

%}

%{^


#define QUANTUM (1<<12)

DEF_PER_CPU(pid_t, cpu_runq_head); /* cpu runqueue: may hold unsaved processes */
INIT_PER_CPU(pid_t) { cpu_write(pid_t, cpu_runq_head, NOPID); }

DEF_PER_CPU(process_t *, current);
INIT_PER_CPU(current) { cpu_write(process_t *, current, NULL); }
DEF_PER_CPU(context_t *, _next_context);
INIT_PER_CPU(_next_context) { cpu_write(process_t *, _next_context, NULL); }
DEF_PER_CPU(context_t *, _prev_context);
INIT_PER_CPU(_prev_context) { cpu_write(process_t *, _prev_context, NULL); }

DEF_PER_CPU_EXTERN(process_t *, cpu_idle_process);

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

%}

abst@ype status = $extype "status";
abst@ype pid_t = $extype "pid_t";
abst@ype process_t = $extype "process_t";
absview rrlock_v
extern fun waitqueue_append {ql, pl: addr} (_: !pid_t @ ql, _: !process_t @ pl | q: ptr ql, p: ptr pl): status = "mac#waitqueue_append";
extern fun waitqueue_dequeue {ql: addr} (_: !pid_t @ ql | q: ptr ql): [pl: addr] (process_t @ pl | ptr pl) = "mac#waitqueue_dequeue"
extern prfun rel_process {pl: addr} (_: process_t @ pl): void

extern fun rrlock_lock(): (rrlock_v | void) = "mac#rrlock_lock";
extern fun rrlock_unlock(_: rrlock_v | ): void = "mac#rrlock_unlock";
extern fun get_runq_head (_: !rrlock_v | ): [l:addr] (pid_t @ l | ptr l) = "mac#get_runq_head"
extern fun rel_runq_head {l:addr} (pf: pid_t @ l, _: !rrlock_v | p: ptr l): void = "mac#rel_runq_head"
extern fun get_cpuq_head (): [l: addr] (pid_t @ l | ptr l) = "mac#get_cpuq_head"
extern fun rel_cpuq_head {l: addr} (_: pid_t @ l | _: ptr l): void = "mac#rel_cpuq_head"

extern fun move_cpu_runqueue_to_global (): void = "sta#move_cpu_runqueue_to_global"
implement move_cpu_runqueue_to_global () = let
  val (cpuq_pf | cpuq) = get_cpuq_head ()
  val (p_pf | p) = waitqueue_dequeue (cpuq_pf | cpuq)
  val _ = rel_cpuq_head (cpuq_pf | cpuq)
in
  if p = null then let prval _ = rel_process (p_pf) in () end
  else let
    val (lock_v | _) = rrlock_lock ()
    val (runq_pf | runq) = get_runq_head (lock_v | )
    val _ = waitqueue_append (runq_pf, p_pf | runq, p)
    prval _ = rel_process (p_pf)
  in
    rel_runq_head (runq_pf, lock_v | runq);
    rrlock_unlock (lock_v | );
    move_cpu_runqueue_to_global ()
  end
end

%{

void schedule(void)
{
  for(;;) {
    process_t *p;
    /* Processes on global runqueue have been waiting the longest. */

    spinlock_lock(&rrlock);
    /* runq_head is shared -- must hold rrlock */
    p = waitqueue_dequeue(&runq_head); /* try global runqueue first */
    spinlock_unlock(&rrlock);

    if(p == NULL)
      p = waitqueue_dequeue(&cpu_read(pid_t, cpu_runq_head)); /* try cpu runqueue */

    if(p == NULL) {
      /* go idle */
      cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
      cpu_write(process_t *, current, cpu_read(process_t *, cpu_idle_process));
      cpu_write(context_t *, _next_context, &cpu_read(process_t *, cpu_idle_process)->ctxt);
    } else {
      /* invariant: process p runnable by only one CPU at a time */
      cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
      cpu_write(process_t *, current, p);
      process_switch_to(p);
      cpu_write(context_t *, _next_context, &p->ctxt);
      DLOG(1, "switch_to: pid=%d pc=%#x\n", p->pid, p->ctxt.usr.r[15]);
      /* invariant: every process remaining on cpu_runqueue is cleanly saved */
      move_cpu_runqueue_to_global();
      return;
    }
  }
}

status sched_wakeup(process_t *p)
{
  /* p may not be saved yet, so it must go on cpu runqueue */
  return waitqueue_append(&cpu_read(pid_t, cpu_runq_head), p);

  /* Lock is not needed because cpu_runq_head is dedicated to this CPU
   * only. Also, waitqueue_append only touches "next" fields in
   * processes that are already claimed by this CPU. */
}

static void sched_timer_handler(u32 intid)
{
  if(pvttimer_is_triggered()) {
    DLOG(1, "sched_timer_handler intid=%#x\n", intid);
    DLOG(1, "control=%#x\n", PVTTIMER->control);
    pvttimer_set(QUANTUM);
    pvttimer_ack_interrupt();   /* acknowledge and unmask */

    /* Put current process on local CPU runqueue, since its context
     * has not been saved yet. */
    waitqueue_append(&cpu_read(pid_t, cpu_runq_head), cpu_read(process_t *, current));
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
  runq_head = NOPID;
}

void sched_aux_cpu_init(void)
{
  pvttimer_set_handler(sched_timer_handler);
  pvttimer_set(QUANTUM);
  pvttimer_enable_interrupt();
  pvttimer_start();
}

%}

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */

