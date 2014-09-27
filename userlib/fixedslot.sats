#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

absvtype fixedslot_vt (p: int, t: int, f: int, rc: int -> int)
vtypedef fixedslot = [p, t, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef rfixedslot (p: int, t: int, rc: int -> int) = [f: nat] fixedslot_vt (p, t, f, rc)
vtypedef wfixedslot (f: int) = [p, t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef wfixedslot_post (p: int, t: int, f: int, rc: int -> int) =
  // [p', t': nat | (p' == p || p' == t') && (t' == t || t' == f)]
  [p', t': nat | ((p' == p && t' == t) || (p' == t && t' == t) || (p' == t && t' == f) || (p' == f && t' == f))]
  [rc': int -> int] fixedslot_vt (p', t', f, rc')
vtypedef fixedslot__p (p: int) = [t, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__t (t: int) = [p, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__f (f: int) = [p, t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__t_f (t: int, f: int) = [p: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__p_f (p: int, f: int) = [t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__rc (rc: int -> int) = [p, t, f: nat] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__f_rc (f: int, rc: int -> int) = [p, t: nat] fixedslot_vt (p, t, f, rc)
fun{a:t@ype} fixedslot_read (!fixedslot >> fixedslot): a
fun{a:t@ype} fixedslot_write (!fixedslot >> fixedslot, a): void
fun fixedslot_initialize_reader (!fixedslot): void
fun fixedslot_initialize_writer (!fixedslot): void
