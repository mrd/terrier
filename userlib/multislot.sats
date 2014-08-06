// multislot: a 1-way pool-based async communication library with options for multiple readers/writers

// See: Simpson, "Four-slot fully asynchronous communication mechanism", IEE, 1989
//      Simpson, "Multireader and multiwriter asynchronous communication mechanisms", IEE, 1997

#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "multislot.cats"

%}


staload "ipcmem.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// small hack for naming convenience
sortdef bit = bool
typedef bit (x: bit) = bool x

// (address, num pages, elem type, is writer?, index)
absview mslot_v (l: addr, n: int, a: t@ype, w: bool, i: int)
absview ws3_v (OP: bit, IP: bit, Rip: bit, Wip: bit, overlap: bool)

// fun{a:t@ype} multislot_initialize_reader {l:addr | l > null} {n: nat} (
//     pf: ipcmem_v (l, n) | p: ptr l, pages: int n
//   ): [s:int] (either_v (ipcmem_v (l, n), mslot_v (l, n, a, false), s == 0) | status s)

fun{a:t@ype} multislot_initialize_reader {l:addr | l > null} {n: nat} (
    pf: ipcmem_v (l, n) | p: ptr l, pages: int n, index: &int? >> int i
  ): #[i:nat] [s:int] (either_v (ipcmem_v (l, n), mslot_v (l, n, a, false, i), s == 0) | status s)

fun{a:t@ype} multislot_initialize_writer {l:addr | l > null} {n: nat} (
    pf: ipcmem_v (l, n) | p: ptr l, pages: int n
  ): [s: int]
     [overlap': bool]
     [OP', IP', Rip', Wip': bit | (OP' != IP') || (Rip' != Wip') || (overlap' == false)]
     (either_v (ipcmem_v (l, n), (mslot_v (l, n, a, true, 0), ws3_v (OP', IP', Rip', Wip', overlap')), s == 0) | status s)

prfun multislot_release {l:addr} {n,i:nat} {a:t@ype} {w:bool} (_: mslot_v (l, n, a, w, i)): ipcmem_v (l, n)
prfun multislot_release_ws3_v {OP, IP, Rip, Wip: bit} {overlap: bool} (_: ws3_v (OP, IP, Rip, Wip, overlap)): void

// fun{a:t@ype} multislot_read {l: addr} {n, i: nat} (pf_ms: !mslot_v (l, n, a, false) | ms: ptr l): a
fun{a:t@ype} multislot_read {l: addr} {n, i: nat} (pf_ms: !mslot_v (l, n, a, false, i) | ms: ptr l, i: int i): a

fun{a:t@ype} multislot_write {l: addr} {n: nat}
                             {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} (
    pf_ms: !mslot_v (l, n, a, true, 0), pf_ws3: !ws3_v (OP, IP, Rip, Wip, overlap) >> ws3_v (OP', IP', Rip', Wip', overlap') |
    ms: ptr l, item: a
  ): #[overlap': bool]
     #[OP', IP', Rip', Wip': bit | (OP' != IP') || (Rip' != Wip') || (overlap' == false)] void



(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
