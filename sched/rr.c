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

#include "types.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "omap3/timer.h"
#include "sched/process.h"
#include "intr/interrupts.h"
#define MODULE "sched_rr"
#include "debug/log.h"

#define QUANTUM (1<<12)

static pid_t runq_head;
process_t *current;
context_t *_next_context, *_prev_context;

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

void schedule(void)
{
  process_t *p = waitqueue_dequeue(&runq_head);
  if(p == NULL) {
    /* go idle */
  } else {
    _prev_context = &current->ctxt;
    current = p;
    process_switch_to(p);
    _next_context = &p->ctxt;
    DLOG(1, "switch_to: pid=%d pc=%#x\n", p->pid, p->ctxt.usr.r[15]);
  }
}

status sched_wakeup(process_t *p)
{
  return waitqueue_append(&runq_head, p);
}

static void sched_timer_handler(u32 activeirq)
{
  extern u32 irq_stack_top;
  u32 *sp = &irq_stack_top;
  sp -= 2;
  DLOG(1, "sched_timer_handler %#x %#x\n", sp[0], sp[1]);
  timer_gp_set(1, -QUANTUM);
  timer_gp_ack_overflow_interrupt(1);
  intc_unmask_irq(activeirq);

  waitqueue_append(&runq_head, current);
  schedule();
}

void sched_init(void)
{
  timer_gp_set_handler(1, sched_timer_handler);
  timer_gp_set(1, -QUANTUM);
  timer_gp_enable_overflow_interrupt(1);
  timer_gp_start(1);
  runq_head = NOPID;
  current = NULL;
  _next_context = _prev_context = NULL;
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
