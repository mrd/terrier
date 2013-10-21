// "Type A" vs "Type B" is a somewhat arbitrary distinction but
// necessary to indicate that there are two sides to every fourslot2w
// connection, and A is the mirror-image of B.

#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fourslot2w.cats"

%}

staload "ipcmem.sats"

// define "A" and "B" 'sides' to every fourslot2w
sortdef fourslot2w_side = int
abstype fourslot2w_side (s: fourslot2w_side) = $extype"fourslot2w_side_t"
stadef A: int = 0
stadef B: int = 1
macdef A = $extval (fourslot2w_side(A), "fourslot2w_A")
macdef B = $extval (fourslot2w_side(B), "fourslot2w_B")

// this will work in ATS2:
//datasort fourslot2w_side = A | B
//datatype fourslot2w_side (s: fourslot2w_side) = A (A) | B (B)

absview fourslot2w_v (l: addr, n: int, a1: t@ype, a2: t@ype, s: fourslot2w_side)

prfun fourslot2w_ipc_free {a1,a2:t@ype} {l:addr} {pages:nat} {s:fourslot2w_side}
    (_: fourslot2w_v (l, pages, a1, a2, s)): ipcmem_v (l, pages)

(* ************************************************** *)

// A-side
fun{a1,a2:t@ype} fourslot2w_readA {l:addr} {n:nat} (pf_fs: !fourslot2w_v (l, n, a1, a2, A) | fs: ptr l): a1
fun{a1,a2:t@ype} fourslot2w_writeA {l:addr} {n:nat} (pf_fs: !fourslot2w_v (l, n, a1, a2, A) | fs: ptr l, item: a2): void

// B-side
fun{a1,a2:t@ype} fourslot2w_readB {l:addr} {n:nat} (pf_fs: !fourslot2w_v (l, n, a1, a2, B) | fs: ptr l): a2
fun{a1,a2:t@ype} fourslot2w_writeB {l:addr} {n:nat} (pf_fs: !fourslot2w_v (l, n, a1, a2, B) | fs: ptr l, item: a1): void

(* ************************************************** *)

fun{a1,a2:t@ype} fourslot2w_init {l:addr | l > null} {pages: nat} {s: fourslot2w_side} (
    pf: !ipcmem_v (l, pages) >> either_v (ipcmem_v (l, pages), fourslot2w_v (l, pages, a1, a2, s), st == 0) |
    p: ptr l, pages: int pages, s: fourslot2w_side s
  ): #[st:int] status st

fun fourslot2w_init_c {a1,a2:t@ype} {l:addr | l > null}
                      {pages: nat}
                      {s: fourslot2w_side} (
    pf: !ipcmem_v (l, pages) >> either_v (ipcmem_v (l, pages), fourslot2w_v (l, pages, a1, a2, s), st == 0) |
    p: ptr l, pages: int pages, s: fourslot2w_side s, elsz1: size_t (sizeof a1), elsz2: size_t (sizeof a2)
  ): #[st:int] status st
  = "mac#_fourslot2w_init_c"


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
