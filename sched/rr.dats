(*
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
 *)

#define ATS_DYNLOADFLAG 0

staload "sched/rr.sats"

absview rrlock_v
extern fun rrlock_lock(): (rrlock_v | void) = "mac#rrlock_lock";
extern fun rrlock_unlock(_: rrlock_v | ): void = "mac#rrlock_unlock";

absviewtype pqueue (n:int) = ptr
extern praxi lemma_pqueue_param_always_nat {n: int} (_: !pqueue n): [n >= 0] void
extern fun pqueue_is_empty {n: nat} (_: !pqueue n): bool (n == 0) = "mac#pqueue_is_empty"
extern fun pqueue_is_not_empty {n: nat} (_: !pqueue n): bool (n <> 0) = "mac#pqueue_is_not_empty"

absviewtype process (l:addr)
extern fun process_is_null {l:addr} (_: !process l): [l >= null] bool (l == null) = "mac#atspre_ptr_is_null"

extern fun waitqueue_dequeue {n:nat | n > 0} (q: !pqueue n >> pqueue (n-1)): [l: agz] process(l) = "mac#waitqueue_dequeue"
extern fun waitqueue_append {n:nat} {l:agz} (q: !pqueue n >> pqueue (n+1), p: process l): void = "mac#waitqueue_append"

extern fun get_runq_head (_: !rrlock_v | ): [n: nat] pqueue n = "mac#get_runq_head"
extern fun rel_runq_head {n:nat} (_: !rrlock_v | p: pqueue n): void = "mac#rel_runq_head"
extern fun get_cpuq_head (): [n:nat] pqueue n = "mac#get_cpuq_head"
extern fun rel_cpuq_head {n:nat} (_: pqueue n): void = "mac#rel_cpuq_head"

extern fun move_cpu_runqueue_to_global (): void = "sta#move_cpu_runqueue_to_global"
implement move_cpu_runqueue_to_global () = let
  val cpuq = get_cpuq_head ()
in
  if pqueue_is_empty cpuq then rel_cpuq_head cpuq
  else let
    val p = waitqueue_dequeue cpuq; val _ = rel_cpuq_head cpuq
    val (lock_v | _) = rrlock_lock ()
    val runq = get_runq_head (lock_v | )
    val _ = waitqueue_append (runq, p)
  in
    rel_runq_head (lock_v | runq);
    rrlock_unlock (lock_v | );
    move_cpu_runqueue_to_global ()
  end
end

extern fun do_process_switch {l:agz} (_: process l): void = "do_process_switch"
extern fun do_idle (): void = "do_idle"

extern fun schedule (): void = "schedule"
implement schedule () = let
  val (lock_v | _) = rrlock_lock ()
  val runq = get_runq_head (lock_v | )
in
  if pqueue_is_not_empty runq then let
    val p = waitqueue_dequeue runq
  in
    rel_runq_head (lock_v | runq);
    rrlock_unlock (lock_v | );
    (* selected p from the global runqueue *)
    do_process_switch p
  end else let
    val cpuq = get_cpuq_head ()
  in
    rel_runq_head (lock_v | runq);
    rrlock_unlock (lock_v | );
    if pqueue_is_not_empty cpuq then
      (* selected p from the local runqueue *)
      do_process_switch (waitqueue_dequeue cpuq)
    else
      (* no process ready to run *)  
      do_idle ();

    rel_cpuq_head cpuq
  end
end

%{

void do_idle(void)
{
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

%}

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 */

/* vi: set et sw=2 sts=2: */
