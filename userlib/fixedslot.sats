#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

// Primary abstract viewtype definition
// FIXME: index fixedslot_vt by type. and by read or write-version.
absvtype fixedslot_vt (S: int, p: int, t: int, f: int, rc: int -> int) = ptr

// Common shortcuts
vtypedef fixedslot = [S, p, t, f: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)


(* vtypedef rfixedslot_post (p: int, t: int, f: int, rc: int -> int) =
 *   [p', t', f': nat | ((p' == p && t' == t) || (p' == t && t' == t) || (p' == t && t' == f') || (p' == f && t' == f'))]
 *   [rc': int -> int]
 *   fixedslot_vt (p', t', f', rc') *)

vtypedef rfixedslot (S: int, p: int, t: int, rc: int -> int) = [f: nat] fixedslot_vt (S, p, t, f, rc)


vtypedef wfixedslot (f: int) = [S, p, t: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)

// Possibilities:
// (p1 == p0 && t1 == t0) ||
// (p1 == t0 && t1 == t0) ||
// (p1 == t0 && t1 == f)  ||
// (p1 == f  && t1 == f)

// Frequently used post-condition for writer steps:
// ((p1 == p0 && t1 == t0) || (p1 == t0 && t1 == t0) || (p1 == t0 && t1 == f) || (p1 == f && t1 == f))

vtypedef wfixedslot_post (p: int, t: int, f: int, rc: int -> int) =
  [S', p', t': nat | ((p' == p && t' == t) || (p' == t && t' == t) || (p' == t && t' == f) || (p' == f && t' == f))]
  [rc': int -> int] fixedslot_vt (S', p', t', f, rc')

// Helper viewtypedefs
vtypedef fixedslot__p (p: int) = [S, t, f: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__t (t: int) = [S, p, f: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__f (f: int) = [S, p, t: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__t_f (t: int, f: int) = [S, p: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__p_f (p: int, f: int) = [S, t: nat] [rc: int -> int] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__rc (rc: int -> int) = [S, p, t, f: nat] fixedslot_vt (S, p, t, f, rc)
vtypedef fixedslot__f_rc (f: int, rc: int -> int) = [S, p, t: nat] fixedslot_vt (S, p, t, f, rc)

// API function prototypes
fun{a:t@ype} fixedslot_read (!fixedslot >> fixedslot): a
fun{a:t@ype} fixedslot_write (!fixedslot >> fixedslot, a): void
fun fixedslot_initialize_reader {l: addr} {n: nat} (ipcmem_v (l, n) | ptr l, int n): fixedslot = "mac#_fixedslot_initialize_reader"
fun fixedslot_initialize_writer {l: addr} {n: nat} (ipcmem_v (l, n) | ptr l, int n): fixedslot = "mac#_fixedslot_initialize_writer"
fun fixedslot_free (fixedslot): [l: addr] [n: nat] (ipcmem_v (l, n) | ptr l) = "mac#_fixedslot_free"
