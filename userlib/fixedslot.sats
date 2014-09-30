#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

// Primary abstract viewtype definition
absvtype fixedslot_vt (p: int, t: int, f: int, rc: int -> int)

// Common shortcuts
vtypedef fixedslot = [p, t, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef rfixedslot (p: int, t: int, rc: int -> int) = [f: nat] fixedslot_vt (p, t, f, rc)
vtypedef wfixedslot (f: int) = [p, t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)

// Possibilities:
// (p1 == p0 && t1 == t0) ||
// (p1 == t0 && t1 == t0) ||
// (p1 == t0 && t1 == f)  ||
// (p1 == f  && t1 == f)

// Frequently used post-condition for writer steps:
// ((p1 == p0 && t1 == t0) || (p1 == t0 && t1 == t0) || (p1 == t0 && t1 == f) || (p1 == f && t1 == f))

vtypedef wfixedslot_post (p: int, t: int, f: int, rc: int -> int) =
  [p', t': nat | ((p' == p && t' == t) || (p' == t && t' == t) || (p' == t && t' == f) || (p' == f && t' == f))]
  [rc': int -> int] fixedslot_vt (p', t', f, rc')

// Helper viewtypedefs
vtypedef fixedslot__p (p: int) = [t, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__t (t: int) = [p, f: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__f (f: int) = [p, t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__t_f (t: int, f: int) = [p: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__p_f (p: int, f: int) = [t: nat] [rc: int -> int] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__rc (rc: int -> int) = [p, t, f: nat] fixedslot_vt (p, t, f, rc)
vtypedef fixedslot__f_rc (f: int, rc: int -> int) = [p, t: nat] fixedslot_vt (p, t, f, rc)

// API function prototypes
fun{a:t@ype} fixedslot_read (!fixedslot >> fixedslot): a
fun{a:t@ype} fixedslot_write (!fixedslot >> fixedslot, a): void
fun fixedslot_initialize_reader (!fixedslot): void
fun fixedslot_initialize_writer (!fixedslot): void
