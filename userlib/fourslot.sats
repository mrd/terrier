// fourslot: a 1-way pool-based async communication library

// See: Simpson, "Four-slot fully asynchronous communication mechanism", IEE, 1989

#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fourslot.cats"

%}

staload "ipcmem.sats"
staload "prelude/SATS/status.sats"

absview fourslot_v (l: addr, n: int, a: t@ype, w: bool)
prfun fourslot_ipc_free {l:addr} {pages:nat} {a:t@ype} {w:bool} (_: fourslot_v (l, pages, a, w)): ipcmem_v (l, pages)

(* ************************************************** *)

fun{a:t@ype} fourslot_read {l:addr} {n:nat} (pf_fs: !fourslot_v (l, n, a, false) | fs: ptr l): a
fun{a:t@ype} fourslot_write {l:addr} {n: nat} (pf_fs: !fourslot_v (l, n, a, true) | fs: ptr l, item: a): void

(* ************************************************** *)

fun{a:t@ype} fourslot_ipc_writer_init {l:addr | l > null} {pages: nat} (
    pf: ipcmem_v (l, pages) | p: ptr l, pages: int pages
  ): [s:int] (option_v (fourslot_v (l, pages, a, true), s == 0) | status s)

fun{a:t@ype} fourslot_ipc_writer_init2 {l:addr | l > null} {pages: nat} (
    pf: !ipcmem_v (l, pages) >> either_v (ipcmem_v (l, pages), fourslot_v (l, pages, a, true), s == 0) |
    p: ptr l, pages: int pages
  ): #[s:int] status s

fun fourslot_ipc_writer_init_c {a:t@ype} {l:addr | l > null} {pages: nat} (
    pf: ipcmem_v (l, pages) | p: ptr l, pages: int pages, elsz: size_t (sizeof a)
  ): [s:int] (option_v (fourslot_v (l, pages, a, true), s == 0) | status s)
  = "mac#_fourslot_ipc_writer_init_c"

fun fourslot_ipc_writer_init2_c {a:t@ype} {l:addr | l > null} {pages, e: nat | e == sizeof a} (
    pf: !ipcmem_v (l, pages) >> either_v (ipcmem_v (l, pages), fourslot_v (l, pages, a, true), s == 0) |
    p: ptr l, pages: int pages, elsz: size_t e
  ): #[s:int] status s
  = "mac#_fourslot_ipc_writer_init_c"

fun fourslot_ipc_reader_init {a:t@ype} {l:addr | l > null} {pages: nat} (
    pf: ipcmem_v (l, pages) | p: ptr l, pages: int pages
  ): [s:int] (option_v (fourslot_v (l, pages, a, false), s == 0) | status s)
  = "mac#_fourslot_ipc_reader_init_c"


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
