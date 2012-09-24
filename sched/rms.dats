(*
 * Rate Monotonic Scheduler -- basic real time scheduler
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
#define MAX_PROCESSES 16
#define MAX_T (span_of_int 2147483647)

staload "rms.sats"

absviewtype pqueue (n:int) = ptr
extern praxi lemma_pqueue_param_always_nat {n: int} (_: !pqueue n): [n >= 0] void
extern fun pqueue_is_empty {n: nat} (_: !pqueue n): bool (n == 0) = "mac#pqueue_is_empty"
extern fun pqueue_is_not_empty {n: nat} (_: !pqueue n): bool (n <> 0) = "mac#pqueue_is_not_empty"

absviewtype process(l:addr)
extern fun get_process (_: int): [l:agz] process l = "mac#get_process"

extern fun sched_wakeup {l:agz} (p: process l): void = "ext#sched_wakeup"
extern fun waitqueue_append {n:nat} {l:agz} (q: !pqueue n >> pqueue (n+1), p: process l): void = "mac#waitqueue_append"

abst@ype span (n:int) = uint
typedef span = [n:nat] span n
extern castfn int_of_span {n: int} (_: span n):<> int n
extern castfn span_of_int {n: int} (_: int n):<> span n
extern fun span_sub_span {n, m: nat} (_: span n, _: span m): span (n - m)
overload - with span_sub_span
extern fun update_prev_sched (): span = "sta#update_prev_sched"
extern fun span_lt_span {n, m: nat} (_: span n, _: span m): bool (n < m)
overload < with span_lt_span

extern castfn int1_of_int (x: int):<> [n:int] int n
extern castfn int_of_int1 {n: int} (x: int n):<> int

abst@ype tick (n:int) = uint
typedef tick = [n: nat] tick n
extern castfn int_of_tick {n: nat} (_: tick n):<> int n
extern castfn tick_of_int {n: nat} (_: int n):<> tick n
extern fun timer_32k_value (): [n: nat] tick n = "mac#timer_32k_value"
extern fun tick_sub_tick {n, m: nat} (_: tick n, _: tick m): span (n - m)
overload - with tick_sub_tick
extern fun tick_add_span {n, m: nat} (_: tick n, _: span m): tick (n + m)
overload + with tick_add_span
extern fun is_earlier_than {n, m: nat} (_: tick n, _: tick m): bool (n < m) = "mac#is_earlier_than"

extern fun process_id {l:agz} (_: !process l): int = "mac#process_id"
extern fun get_capacity {l:agz} (_: !process l): [n:nat] span n = "mac#get_capacity"
extern fun get_period {l:agz} (_: !process l): [n:pos] span n = "mac#get_period"

(* set_budget consumes process *)
extern fun set_budget {l:agz; n:nat} (_: process l, _: span n): void = "mac#set_budget"
extern fun get_budget {l:agz} (_: !process l): [n:nat] span n = "mac#get_budget"

extern fun get_replenish_time {l:agz} (_: !process l): [n:nat] tick n = "mac#get_replenish_time"
extern fun set_replenish_time {l:agz; n:nat} (_: !process l, _: tick n): void = "mac#set_replenish_time"

extern fun get_current (): [l:agz] process l = "mac#get_current"

absview must_set_timer_v
extern fun schedule (_: must_set_timer_v | ): void = "ext#schedule"

implement span_sub_span (n, m) = span_of_int (int_of_span n - int_of_span m)
implement span_lt_span (n, m) = int_of_span n < int_of_span m
implement tick_sub_tick (n, m) = span_of_int (int_of_tick n - int_of_tick m)
implement tick_add_span (n, m) = tick_of_int (int_of_tick n + int_of_span m)

extern fun rel_process {l:agz} (_: process l): void = "mac#rel_process"
extern fun pvttimer_set {t: nat} (_: must_set_timer_v | _: span t): void = "mac#pvttimer_set"
extern fun do_process_switch {l:agz} (_: process l): void = "do_process_switch"
extern fun do_idle (): void = "do_idle"

implement sched_wakeup (p) = begin
  set_budget(p, get_capacity(p))
end

extern fun foo {n, m: nat | n <= m} (_: tick n, _: tick m): void

typedef tickGT (x: int) = [y: int | x < y] tick y

//dataprop all_processes x = all_processes x
//fun{a:t@ype} map_processes(f: (prop | process, a) -> void, env: a): (all_processes prop | void)

absviewt@ype process_iter (n:int) = int
viewtypedef process_iter = [n: nat] process_iter n

extern fun make_process_iter (): process_iter = "mac#make_process_iter"
extern fun process_iter {n:pos} (_: &process_iter n >> process_iter (n-1)): [l:agz] process l = "mac#process_iter"
extern fun process_iter_has_next {n:nat} (_: !process_iter n): bool (n > 0) = "mac#process_iter_has_next"
extern fun process_iter_done {n: nat} (_: process_iter n): void = "mac#process_iter_done"

extern fun logspan(x: int, y: int): void = "mac#logspan"
extern fun logovhd(x: int): void = "mac#logovhd"

%{^

#define make_process_iter() 1
#define process_iter(x) get_process((*((u32 *) (x)))++)
#define process_iter_has_next(x) (x <= MAX_PROCESSES && process[x-1].pid != NOPID)
#define process_iter_done(x)
#define logspan(x,y) { DLOG(1,"prev_span=%#x now=%#x\n", x, y); }
#define logovhd(x) { DLOG(1,"schedule took %#x ticks\n", x); }

%}

implement schedule (must_set_timer_v | ) = let
  val prev_span = update_prev_sched ()
  val [now: int] now = timer_32k_value ()
  val _ = logspan(int_of_span prev_span, int_of_tick now)
  val current = get_current ()
  
  (* subtract time passed from current process *)
  val bcurrent = get_budget current
  val bcurrent' = (if bcurrent < prev_span
                   then span_of_int 0
                   else bcurrent - prev_span): span

  val _ = set_budget (current, bcurrent')

  (* Refill budgets if any process has reached its replenish time. Also
   * find soonest replenishment time in the future. *)

  (* Find next process to run by examining which processes have
   * remaining budget and then picking the highest priority one to
   * run. *)

  (* <cloref1> = this function makes use of variables from the outside *)
  fun loop {i, t: nat}
           (mstv: must_set_timer_v |
            iter: process_iter i,
            next_r: tickGT now,
            next_t: span t,
            next_i: int)
           :<cloref1> void =
    if process_iter_has_next iter then let
      var iter2 : process_iter i = iter
      val p = process_iter iter2
      val iter3 = iter2
      val b = get_budget p
      val t = get_period p
      val r = get_replenish_time p
      (* ensure new replenishment time is in the future *)

      val r' =
        (if is_earlier_than (now, r) then r
         else (if is_earlier_than (now, r + t) then r + t
               else now + t)): tickGT now
      val new_b =
        (if is_earlier_than (now, r) then b else get_capacity p): span

      val (t', i') = (if span_of_int 0 < new_b then
                        if t < next_t then (t, process_id p)
                                      else (next_t, next_i)
                      else (next_t, next_i)): (span, int)
    in
      set_replenish_time (p, r');
      set_budget (p, new_b);
      loop (mstv | iter3, if is_earlier_than (r', next_r) then r' else next_r, t', i')
    end (* end [let var i2] *)
    else if next_i = 0 then begin
      (* no next process, go idle *)
      process_iter_done iter;
      pvttimer_set (mstv | next_r - now);
      do_idle ()
    end else let
      (* prepare next process *)
      val pnext = get_process next_i
      val bnext = get_budget pnext
      val ticks = (if is_earlier_than (now + bnext, next_r) then bnext else next_r - now): span
    in
      process_iter_done iter;
      pvttimer_set (mstv | ticks);
      do_process_switch pnext
    end
  (* end [fun loop] *)

  var pi: process_iter = make_process_iter ()
in
  loop (must_set_timer_v | pi, now + MAX_T, MAX_T, 0);
  logovhd (int_of_span (timer_32k_value () - now))
end (* end [let fun loop] *)

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
