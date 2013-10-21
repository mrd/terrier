(*
** RMS: rate-monotonic scheduler
*)

(* ****** ****** *)
//
#define ATS_STALOADFLAG 0 // no run-time staloading
//
(* ****** ****** *)

%{#

#include "rms_sched.cats"

%}

abst@ype span = uint

castfn int_of_span(_:span):int

(* ****** ****** *)

fun sub_span_span (s1: span, s2: span): span = "mac#atspre_sub_int_int"
overload - with sub_span_span

fun lt_span_span (s1: span, s2: span): bool = "mac#atspre_lt_int_int"
overload < with lt_span_span
fun lte_span_span (s1: span, s2: span): bool = "mac#atspre_lte_int_int"
overload <= with lte_span_span

fun gt_span_span (s1: span, s2: span): bool = "mac#atspre_gt_int_int"
overload > with gt_span_span
fun gte_span_span (s1: span, s2: span): bool = "mac#atspre_gte_int_int"
overload >= with gte_span_span

(* ****** ****** *)

abst@ype tick = uint
castfn int_of_tick(_:tick):int

fun add_tick_span (t1: tick, s2: span): tick = "mac#atspre_add_int_int"
overload + with add_tick_span

fun sub_tick_tick (t1: tick, t2: tick): span = "mac#atspre_sub_int_int"
overload - with sub_tick_tick

fun lt_tick_tick (t1: tick, t2: tick): bool = "mac#is_earlier_than"
overload < with lt_tick_tick
fun lte_tick_tick (t1: tick, t2: tick): bool = "mac#is_earlier_than_or_equal"
overload <= with lte_tick_tick

(* ****** ****** *)

abst@ype pid = int // process id

(* ****** ****** *)

abstype process = ptr // boxed record

fun process_get_pid (p: process): pid = "mac#process_id"

fun process_get_period (p: process): span = "mac#get_period"
fun process_get_capacity (p: process): span = "mac#get_capacity"

fun process_get_budget (p: process): span = "mac#get_budget"
fun process_set_budget (p: process, s: span): void = "mac#set_budget"

fun process_get_replenish (p: process): tick = "mac#get_replenish_time"
fun process_set_replenish (p: process, t: tick): void = "mac#set_replenish_time"
fun process_reset_replenish (p: process, now: tick): void = "mac#reset_replenish_time"

fun process_is_idle (p: process): bool = "mac#process_is_idle"

castfn int_of_pid (_: pid): int

(* ****** ****** *)

absviewtype
processopt_vtype (bool)
stadef processopt = processopt_vtype
viewtypedef processopt = [b:bool] processopt b

(* ****** ****** *)

absviewt@ype
processlst_vtype (n:int) = int
stadef processlst = processlst_vtype
viewtypedef processlst = [n:nat] processlst (n)

prfun
processlst_free_nil (xs: processlst (0)): void

fun processlst_is_cons
  {n:int} (xs: !processlst (n)): bool (n > 0) = "mac#processlst_is_cons"
// end of [processlst_is_cons]

fun processlst_uncons
  {n:int | n > 0}
  (xs: processlst n, x: &process? >> process): processlst (n-1) = "mac#processlst_uncons"
// end of [processlst_uncons]

fun the_processlst_get (): processlst = "mac#the_processlst_get"


(* ****** ****** *)
//
// various specific functions
//
(* ****** ****** *)

fun timer_32k_value (): tick = "mac#timer_32k_value"
fun timer_32k_delay (s: span): void = "mac#timer_32k_delay"

(* ****** ****** *)

fun schedule (): void = "ext#schedule"
fun sched_wakeup (p: process): void = "ext#sched_wakeup"

(* ****** ****** *)

fun budget_is_negligible (b: span): bool

(* ****** ****** *)

fun pvttimer_set (n: span): void = "mac#pvttimer_set"
fun arm_read_cycle_counter (): int = "mac#arm_read_cycle_counter"

(* ****** ****** *)

fun get_prev_span (): span = "mac#update_prev_sched"
fun get_prev_timer_val (): span = "mac#get_prev_timer_val"
fun set_prev_timer_val (_: span): void = "mac#set_prev_timer_val"

(* ****** ****** *)

fun arm_read_cycle_counter (): int = "mac#arm_read_cycle_counter"
fun record_stats (_: span, _: span, _: int): void = "mac#record_stats"
fun dump_stats (): void = "mac#dump_stats"
fun dump_process_stats (): void = "mac#dump_process_stats"
fun update_process_stats (prev_span: span): void = "mac#update_process_stats"

(* ****** ****** *)

fun get_process_current (): process = "mac#get_current"

(* ****** ****** *)

fun do_process_switch (p: process): void = "mac#do_process_switch"
fun do_idle(): void = "mac#do_idle"

(* ****** ****** *)

// fun DLOG {ts:types} (_:int, _: printf_c ts, _: ts): void = "mac#DLOG"



(* end of [rms.sats] *)
