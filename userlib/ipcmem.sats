// ipcmem: module to manage IPC connections created by kernel

#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "ipcmem.cats"

%}

// for now
dataview either_view_bool_view (a:view+, b:view+, bool) = Left_v (a, b, false) of a | Right_v (a, b, true) of b
stadef either_v = either_view_bool_view

typedef status (s:int) = int s  (* helper *)

absview ipcmem_v (l: addr, pages: int)

fun ipcmem_get_view (id: int, pages: &int? >> int pages): #[pages:nat] [l:addr] (option_v (ipcmem_v (l, pages), l > null) | ptr l)
  = "mac#_ipcmem_get_view_c"
prfun ipcmem_put_view {l:addr} {pages:int} (_: ipcmem_v (l, pages)): void


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
