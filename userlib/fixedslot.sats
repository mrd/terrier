#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

// Primary abstract viewtype definition
// (datatype, writer?, S-value, previous, target, freshest, rcount)
absvtype fixedslot_vt (a: t@ype, wr: bool, S: int, p: int, t: int, f: int, rc: int -> int) = ptr

// Common shortcuts
vtypedef fixedslot (a: t@ype, wr: bool) = [S, p, t, f: nat] [rc: int -> int] fixedslot_vt (a, wr, S, p, t, f, rc)
vtypedef fixedslotw (a: t@ype) = fixedslot (a, true)
vtypedef fixedslotr (a: t@ype) = fixedslot (a, false)

// API function prototypes
fun{a:t@ype} fixedslot_read (!fixedslotr a >> fixedslotr a): a
fun{a:t@ype} fixedslot_write (!fixedslotw a >> fixedslotw a, a): void
fun fixedslot_readptr {a:t@ype} {l: agz} (!a @ l | !fixedslotr a >> fixedslotr a, ptr l, size_t (sizeof a)): void
fun fixedslot_writeptr {a:t@ype} {l: agz} (!a @ l | !fixedslotw a >> fixedslotw a, ptr l, size_t (sizeof a)): void
fun fixedslot_initialize_reader {a: t@ype} {l: addr} {n: nat} (ipcmem_v (l, n) | ptr l, int n): fixedslotr a = "mac#_fixedslot_initialize_reader"
fun fixedslot_initialize_writer {a: t@ype} {l: addr} {n: nat} (ipcmem_v (l, n) | ptr l, int n): fixedslotw a = "mac#_fixedslot_initialize_writer"
fun fixedslot_free {a: t@ype} {wr: bool} (fixedslot (a, wr)): [l: addr] [n: nat] (ipcmem_v (l, n) | ptr l) = "mac#_fixedslot_free"
