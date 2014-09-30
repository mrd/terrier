#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "fixedslot.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// --------------------------------------------------
// reader

extern fun rcount {rc: int -> int} {i: nat} (
    !fixedslot__rc rc >> fixedslot__rc rc', int i
  ): #[rc': int -> int]
      [j: nat | rc' i == j ] int j
  = "mac#_rcount"

// atomic
extern fun pick_ri {p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> fixedslot_vt (p', t', f', rc')
  ): #[rc': int -> int]
     #[p', f', t': nat | (p == t) ==> (t' == f')]
      [ri: int | rc' ri >= 0 && ri == t'] int ri
  = "mac#_pick_ri"

extern fun rtarget (!fixedslot >> fixedslot__t t'): #[t': nat] int t'
  = "mac#_rtarget"

extern fun rprevious (!fixedslot >> fixedslot__p p'): #[p': nat] int p'
  = "mac#_rprevious"

extern fun set_rprevious {p': nat} (!fixedslot >> fixedslot__p p', int p'): void
  = "mac#_set_rprevious"

absview read_data_v (ri: int) // ensure an incremented count is decremented
extern fun incr_rcount {i: int} {rc: int -> int} (
    !fixedslot__rc rc >> fixedslot__rc rc', int i
  ): #[rc': int -> int | rc' i == rc i + 1 ] (read_data_v i | void)
  = "mac#_incr_rcount"

extern fun read_data {a: t@ype} {rc: int -> int} {ri: nat | rc ri > 0} (
    !read_data_v ri |
    !fixedslot__rc rc >> fixedslot__rc rc', int ri, &a? >> a, size_t (sizeof a)
  ): #[rc': int -> int | rc' ri > 0] void
  = "mac#_read_data"

extern fun decr_rcount {i: int} {rc: int -> int | rc i > 0} (
    read_data_v i | !fixedslot__rc rc >> fixedslot__rc rc', int i
  ): #[rc': int -> int | rc' i == rc i - 1 ] void
  = "mac#_decr_rcount"

implement{a} fixedslot_read (fs) = let
  var buf: a?
  val ri = pick_ri (fs)
  val (rd_v | ())  = incr_rcount (fs, ri)
  val () = read_data (rd_v | fs, ri, buf, sizeof<a>)
  val () = decr_rcount (rd_v | fs, ri)
  val () = if rcount (fs, rprevious fs) = 0
           then set_rprevious (fs, rtarget fs)
           else ()
in
  buf
end

// --------------------------------------------------
// writer

// the writer version knows it can preserve 'f'
extern fun wrcount {f: nat} {rc: int -> int} {i: nat} (
    !fixedslot__f_rc (f, rc) >> fixedslot__f_rc (f, rc'), int i
  ): #[rc': int -> int]
      [j: nat | rc' i == j ] int j
  = "mac#_rcount"

extern fun wrprevious {p,t,f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): int p
  = "mac#_rprevious"

extern fun wrtarget {p,t,f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): int t
  = "mac#_rtarget"

extern fun wfilled {p,t,f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): int f
  = "mac#_wfilled"

extern fun set_wfilled {f, f': nat} (!wfixedslot f >> wfixedslot f', int f'): void
  = "mac#_set_wfilled"
extern fun write_data {a: t@ype} {p, t, f: nat} {rc: int -> int}
                      {wi: nat | wi <> p && wi <> f && wi <> t} (  // can we prove rc wi == 0 ? (is it needed?)
    !fixedslot_vt (p, t, f, rc) >> fixedslot_vt (p', t', f, rc'),
    int wi, !a, size_t (sizeof a)
  ): #[p', t': nat] #[rc': int -> int] void
  = "mac#_write_data"


// why can't this be done in multiple steps?
extern fun get_triple {p,t,f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): @(int p, int t, int f)

datasort set = nil | cons of (int, set)

// Property: $x \not\in xs$
dataprop not_in_set (x: int, xs: set) =
  | not_in_set_nil (x, nil) of ()
  | {y: int | y <> x} {xs': set} not_in_set_cons (x, cons (y, xs')) of not_in_set (x, xs')

// Property: $x \in s$
dataprop in_set (x: int, xs: set) =
  | in_set_base1 (x, cons (x, nil)) of int x
  | {xs': set} in_set_base2 (x, cons (x, xs')) of not_in_set (x, xs')
  | {y: int} {xs': set} in_set_cons (x, cons (y, xs')) of in_set (x, xs')

abst@ype intset (s: set) = int // intset is abstract type with machine word size

// implement in ASM:
extern fun intset_nil ():<> intset nil
extern fun intset_cons {x: int} {s: set} (int x, intset s):<> (in_set (x, (cons (x, s))) | intset (cons (x, s)))
// ffs(x) = w − clz(x & (−x)).
extern fun intset_find_first_not_in_set {s: set} (intset s):<> [x: nat] (not_in_set (x, s) | int x)

// Theorem: If $x \in s$ and $y \not\in s$ then $x \neq y$.
prfun in_set_ne_not_in_set {x, y: int} {s: set} .<s>. (pf1: in_set (x, s), pf2: not_in_set (y, s)):<> [x <> y] void =
  case+ (pf1, pf2) of
  | (in_set_base1 x, not_in_set_cons pf2') => ()
  | (in_set_base2 pf1', not_in_set_cons pf2') => ()
  | (in_set_cons pf1', not_in_set_cons pf2') => in_set_ne_not_in_set (pf1', pf2')

// Pick value of $w_i\geq 0$ with proven property that $w_i \not\in \{p, t, f\}$.
extern fun pick_wi {p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): #[wi: nat | wi <> p && wi <> f && wi <> t]
     int wi
implement pick_wi (fs) = wi where {
  val (p, t, f) = get_triple fs

  val (pf_p0 | s1) = intset_cons (p, intset_nil ())
  val (pf_t0 | s2) = intset_cons (t, s1)
  val (pf_f | s3) = intset_cons (f, s2)
  val (pf_wi | wi) = intset_find_first_not_in_set s3

  prval pf_p = in_set_cons (in_set_cons pf_p0)
  prval pf_t = in_set_cons pf_t0
  prval () = in_set_ne_not_in_set (pf_p, pf_wi)
  prval () = in_set_ne_not_in_set (pf_t, pf_wi)
  prval () = in_set_ne_not_in_set (pf_f, pf_wi)
}

implement{a} fixedslot_write (fs, item) = let
  val wi = pick_wi fs
  val () = write_data (fs, wi, item, sizeof<a>)
  val () = set_wfilled (fs, wi)
in () end

////
extern fun pick_wi {p0, t0, f: nat} {rc0: int -> int} (
    !fixedslot_vt (p0, t0, f, rc0) >> wfixedslot_post (p0, t0, f, rc0)
  ): #[wi: nat | wi <> p0 && wi <> f && wi <> t0]
     int wi
implement pick_wi (fs) = loop (fs, p1, t1, f, 0)
where {

// val f = wfilled fs
// val p1 = wrprevious fs
// val t1 = wrtarget fs
val (p1, t1, f) = get_triple fs

// alternatively, do this with bits

fun loop {p1, t1, p0, t0, f, i: nat | ((p1 == p0 && t1 == t0) || (p1 == t0 && t1 == t0) || (p1 == t0 && t1 == f) || (p1 == f && t1 == f))}
         {rc: int -> int} (
      fs: !fixedslot_vt (p1, t1, f, rc) >> wfixedslot_post (p1, t1, f, rc),
      p: int p0, t: int t0, f: int f, i: int i
    ): [wi: nat | wi <> p0 && wi <> f && wi <> t0] int wi =
  if f <> i then if t <> i then if p <> i then i
                                else loop (fs, p, t, f, i + 1)
                 else loop (fs, p, t, f, i + 1)
  else loop (fs, p, t, f, i + 1)

}


////


fun pick_wi {p, t, f: nat} {rc: int -> int}
    (fs: !fixedslot_vt (p, t, f, rc) >> fixedslot_vt (p', t', f, rc')
  ): #[p', t': nat] #[rc': int -> int] [wi: nat | rc' wi == 0 && wi <> f && wi <> t'] int wi =

  loop (fs, wfilled fs, 0) where {

val t = wrtarget fs
fun loop {p, t, f: nat} {rc: int -> int} {i: nat} (
      fs: !fixedslot_vt (p, t, f, rc) >> fixedslot_vt (p', t', f, rc'),
      f: int f, i: int i
    ): #[p', t': nat] #[rc': int -> int] [wi: nat | rc' wi == 0 && wi <> f && wi <> t] int wi =
  if f <> i then if t <> i then if wrcount (fs, i) = 0 then i
                                else loop (fs, f, i + 1)
                 else loop (fs, f, i + 1)
  else loop (fs, f, i + 1)

}

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
// g <- wi // unnecessary in code
write data[wi]
f <- wi
