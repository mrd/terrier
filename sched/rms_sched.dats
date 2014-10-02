(*
** RMS: rate-monotonic scheduler
*)

(* ****** ****** *)

(*
** It is primarily based on an implementation by Matthew Danish
*)

(* ****** ****** *)

#define ATS_DYNLOADFLAG 0
(* #define MAX_PROCESSES 16 *)

staload _ = "prelude/DATS/integer.dats"

staload "./rms_sched.sats"

(* ****** ****** *)

extern castfn span_of_int(_: int): span
extern fun{} ZERO_SPAN (): span
implement{} ZERO_SPAN() = span_of_int 0
extern fun{} BUDGET_THRESHOLD(): span // 32
implement{} BUDGET_THRESHOLD() = span_of_int 32
extern fun{} MAX_T (): span // 2147483647
implement{} MAX_T() = span_of_int 2147483647
extern fun{} STATS_DUMP_THRESHOLD(): span // considered enough idle time to print stats
implement{} STATS_DUMP_THRESHOLD() = span_of_int 8192 // somewhat arbitrary choice

extern fun idle_process(): process = "mac#get_idle_process"

(* ****** ****** *)

implement budget_is_negligible (b) = b < BUDGET_THRESHOLD()

(* ****** ****** *)

local

fun process_adjust_budget
  (p: process, s0: span): void = let
  val b = process_get_budget (p)
  val b = (
    if b >= s0 then b - s0 else ZERO_SPAN()
  ) : span // end of [val]
  val b = (
    if budget_is_negligible (b) then ZERO_SPAN() else b
  ) : span // end of [val]
in
  process_set_budget (p, b)
  //;DLOG(1, "process_adjust_budget %d %d %d %d\n", @(int_of_pid (process_get_pid p), int_of_span s0, int_of_span b, int_of_span (process_get_budget p)))
end // end of [process_adjust_budget]

fun process_adjust_replenish
  (p: process, t0: tick): void = let
  val r = process_get_replenish (p)
in
  if r <= t0 then let
    //val () = DLOG(1, "replenishing %d %d %d\n", @(int_of_pid (process_get_pid p), int_of_tick r, int_of_tick t0))
    val () = process_reset_replenish (p, t0)
    val () = process_set_budget (p, process_get_capacity (p))
  in
  end else () // end of [if]
end // end of [process_adjust_replenish]

fun processlst_adjust_replenish
  {n:nat} (
  ps: processlst (n), t0: tick
) : void = let
in
  if processlst_is_cons (ps) then let
    var p: process
    val ps = processlst_uncons (ps, p)
    val () = process_adjust_replenish (p, t0)
  in
    processlst_adjust_replenish (ps, t0)
  end else let
    prval () = processlst_free_nil (ps)
  in
    // nothing
  end // end of [if]
end // end of [processlst_adjust_replenish]

fun processlst_select_process
  {n:nat} (ps: processlst (n)): process = let
//
fun loop {n:nat} (
  ps: processlst (n), T0: span, res: process
) : process = let
in
  if processlst_is_cons (ps) then let
    var p: process
    val ps = processlst_uncons (ps, p)
    val T = process_get_period (p)
    val b = process_get_budget (p)
  in
    //DLOG(1, "processlst_select_process %d %d %d\n", @(int_of_pid (process_get_pid p), int_of_span (process_get_period p), int_of_span (process_get_budget p)));

    if T < T0 && ZERO_SPAN() < b then let
    in
      loop (ps, T, p)
    end else
      loop (ps, T0, res)
    // end of [if]
  end else let
    prval () = processlst_free_nil (ps)
  in
    res
  end // end of [if]
end // end of [loop]
//
val T0 = MAX_T()
val res = idle_process()
//
in
  loop (ps, T0, res)
end // end of [processlst_select_process]

fun processlst_find_pvttimer_val
  {n:nat} (
  ps: processlst (n), p_next: process, t_now: tick
) : tick = let
//
fun loop {n:nat} (
  ps: processlst (n), T0: span, res: tick
) : tick = let
in
  if processlst_is_cons (ps) then let
    var p: process
    val ps = processlst_uncons (ps, p)
    val T = process_get_period (p)
  in
    if T < T0 then let
      val R = process_get_replenish (p)
      val res = (if R < res then R else res): tick
    in
      loop (ps, T0, res)
    end else
      loop (ps, T0, res)
    // end of [if]
  end else let
    prval () = processlst_free_nil (ps)
  in
    res
  end // end of [if]
end // end of [loop]
//
val T0 = (if process_is_idle (p_next) then MAX_T()
         else process_get_period (p_next)): span
val res = (if process_is_idle (p_next) then t_now + MAX_T()
          else t_now + process_get_budget (p_next)): tick
//
in
  loop (ps, T0, res)
end // end of [processlst_find_pvttimer_val]

in // in of [local]

implement
schedule () = let
//
  val prev_span = get_prev_span ()
  val prev_timer_val = get_prev_timer_val ()
  //val _ = DLOG(1, "prev_span=%d prev_timer_val=%d\n", @(int_of_span prev_span, int_of_span prev_timer_val))
  val p_current = get_process_current ()
  val () = process_adjust_budget (p_current, prev_span)
//
  val t_now = timer_32k_value ()
  val cyc_now = arm_read_cycle_counter ()
//
  val () = update_process_stats (prev_span)
//
  val ps = the_processlst_get ()
  val () = processlst_adjust_replenish (ps, t_now)
//
  val ps = the_processlst_get ()
  val p_next = processlst_select_process (ps)
  val ps = the_processlst_get ()
  //val _ = DLOG(1, "p_next=%d T=%d b=%d\n", @(int_of_pid (process_get_pid p_next), int_of_span (process_get_period p_next), int_of_span (process_get_budget p_next)))
  val t_val = processlst_find_pvttimer_val (ps, p_next, t_now)
  val ticks = t_val - t_now
  //val _ = DLOG(1, "ticks=%d\n", @(int_of_span ticks))
  val () = set_prev_timer_val (ticks) // instrumentation
//
in if process_is_idle (p_next) then
     // no next process, go idle
     if ticks < BUDGET_THRESHOLD() then begin
       // Just spin-wait out short idle periods
       timer_32k_delay (ticks);
       schedule ()
     end else begin
       // For longer idle periods, use idle process
       pvttimer_set (ticks);
       do_idle ();
       record_stats (if prev_span < prev_timer_val then prev_timer_val - prev_span else prev_span - prev_timer_val,
                     timer_32k_value () - t_now,
                     arm_read_cycle_counter () - cyc_now);
       // eat some of idle time, if enough, to dump out useful statistics
       if STATS_DUMP_THRESHOLD() < ticks then begin
         // FIXME: DLOG is a macro and something in ATS2 0.1.3 made this stop working
         // $extfcall(void, "DLOG", 1, "idling for ticks=%d\n", int_of_span ticks);
         // replacing with function call for now:
         $extfcall(void, "debuglog", "rms_sched", 1, "idling for ticks=%d\n", int_of_span ticks);
         dump_stats ();
         dump_process_stats ()
      end
     end // end of [else] branch for longer idle periods
  else begin
    pvttimer_set (ticks);
    do_process_switch (p_next);
    record_stats (if prev_span < prev_timer_val then prev_timer_val - prev_span else prev_span - prev_timer_val,
                  timer_32k_value () - t_now,
                  arm_read_cycle_counter () - cyc_now);
  end
end // end of [schedule]

implement sched_wakeup (p) = begin
  process_set_budget(p, process_get_capacity(p))
end

end // end of [local]

(* ****** ****** *)

(* end of [rms_sched.dats] *)
