#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "fixedslot.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// small hack for naming convenience
sortdef bit = bool
typedef bit (x: bit) = bool x

// --------------------------------------------------

extern fun rcount {rc: int -> int} {i: nat} (!fixedslot, int i): [j: nat | rc i == j] int j = "mac#_rcount"
extern fun rtarget (!fixedslot): int = "mac#_rtarget"
extern fun rprevious (!fixedslot): int = "mac#_rprevious"
extern fun set_rprevious (!fixedslot, int): void = "mac#_set_rprevious"
extern fun wfilled (!fixedslot): int = "mac#_wfilled"
extern fun set_wfilled (!fixedslot, int): void = "mac#_set_wfilled"
// extern fun set_wfilling (!fixedslot, int): void

extern fun pick_ri (!fixedslot): int = "mac#_pick_ri"
extern fun incr_rcount (!fixedslot, int): void = "mac#_incr_rcount"
extern fun read_data {a: t@ype} (!fixedslot, int, &a? >> a, size_t (sizeof a)): void = "mac#_read_data"
extern fun decr_rcount (!fixedslot, int): void = "mac#_decr_rcount"
// extern fun r5 (!fixedslot): void

// extern fun w1 (!fixedslot): int
// extern fun w2 (!fixedslot, int): void
extern fun write_data {a: t@ype} (!fixedslot, int, !a, size_t (sizeof a)): void = "mac#_write_data"
// extern fun w4 (!fixedslot, int): void


dataprop rcount_p (int) =
  | rcount_base (0) of ()
  | {n: nat} rcount_ind (n+1) of (rcount_p (n))

// This function works because all ri are equal to p or t.
// That leaves possibly f, and another empty slot open.

// fun pick_wi {x: nat | x < 4} {rc: int -> int | rc x == 0} (fs: !fixedslot): [wi: nat | wi == x] int wi =
// let
//   fun loop {i, x: nat | i <= x && x < 4} {rc: int -> int | rc x == 0} (
//         fs: !fixedslot, f: int, i: int i
//       ): [wi: nat | rc wi == 0] int wi =
//     if f <> i
//       then if rcount {rc} {i} (fs, i) = 0
//              then i
//              else loop {i+1,x} {rc} (fs, f, i + 1)
//       else loop {i+1,x} {rc} (fs, f, i + 1)
// in
//   loop {0,x} {rc} (fs, wfilled fs, 0)
// end


fun pick_wi {rc: int -> int} (fs: !fixedslot): [wi: nat | rc wi == 0] int wi =
let
  fun loop {i: nat} (
        fs: !fixedslot, f: int, i: int i
      ): [wi: nat | rc wi == 0] int wi =
    if f <> i
      then if rcount {rc} {i} (fs, i) = 0
             then i
             else loop (fs, f, i + 1)
      else loop (fs, f, i + 1)
in
  loop (fs, wfilled fs, 0)
end
  
implement{a} fixedslot_write (fs, item) = let
  val wi = pick_wi (fs)
  // val _  = set_wfilling (fs, wi)
  val _  = write_data (fs, wi, item, sizeof<a>)
  val _  = set_wfilled (fs, wi)
in () end

////
implement{a} fixedslot_read (fs) = let
  var buf: a?
  val ri = pick_ri (fs)
  val _  = incr_rcount (fs, ri)
  val _  = read_data (fs, ri, buf, sizeof<a>)
  val _  = decr_rcount (fs, ri)
  val _  = if rcount (fs, rprevious fs) = 0
           then set_rprevious (fs, rtarget fs)
           else ()
in
  buf
end

////

f, g, p, t
data[4]
rcount[4]

R:
if(p == t) t <- f
ri <- t
incr rcount[ri]
read data[ri]
decr rcount[ri]
if(rcount[p] == 0) p <- t

W:
for(i=0;i<4;i++)  // wi <- ~(p,t,f)
  if(i != f && rcount[i] == 0) {
    wi = i; break;
  }
g <- wi
write data[wi]
f <- wi
