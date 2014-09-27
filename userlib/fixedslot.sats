#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

absvtype fixedslot_vt (p: int, t: int, f: int, rc: int -> int)
vtypedef fixedslot = [p, t, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef rfixedslot (p: int, t: int, rc: int -> int) = [f: nat] fixedslot_vt (p, t, f, rc)
vtypedef wfixedslot (f: int) = [p, t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
fun{a:t@ype} fixedslot_read (!fixedslot >> fixedslot): a
fun{a:t@ype} fixedslot_write (!fixedslot >> fixedslot, a): void
fun fixedslot_initialize_reader (!fixedslot): void
fun fixedslot_initialize_writer (!fixedslot): void
