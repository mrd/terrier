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
extern fun pick_ri {S, p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f', rc')
  ): #[rc': int -> int]
     #[S', p', f', t': nat | ((p == t) ==> (t' == f')) && (S' == S + 1)]
      [ri: int | rc' ri >= 0 && ri == t'] int ri
  = "mac#_pick_ri"

absview read_data_v (ri: int) // view to ensure an incremented count is decremented

// thanks to S > 0 I don't have to consider that p' might become equal to t or t'
// and all the combinations from that
extern fun incr_rcount {S, p, t, f: nat | S > 0} {rc: int -> int} {ri: int | ri == t} (
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f', rc'), int ri
  ): #[S': nat | S' > 0]
     #[p', t', f': nat | (p' == p && t' == t) || (p == t && p' == p && t' == f')]
     #[rc': int -> int | rc' ri == rc ri + 1] (read_data_v ri | void)
  = "mac#_incr_rcount"

extern fun decr_S {S, p, t, f: nat | S > 0} {rc: int -> int} {ri: int | ri == p || ri == t} (
    !read_data_v ri |
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f', rc')
  ): #[S': nat | S' == S - 1]
     #[p', t', f': nat | (p' == p && t' == t) || (p == t && p' == p && t' == f')]
     #[rc': int -> int | rc' ri >= rc ri ] void
  = "mac#_decr_S"

extern fun read_data {a: t@ype} {rc: int -> int} {S, p, t, f, ri: nat | rc ri > 0 && (ri == p || ri == t)} (
    !read_data_v ri |
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot__rc rc', int ri, &a? >> a, size_t (sizeof a)
  ): #[rc': int -> int | rc' ri >= rc ri] void
  = "mac#_read_data"

extern fun read_data_ptr {a: t@ype} {l: agz} {rc: int -> int} {S, p, t, f, ri: nat | rc ri > 0 && (ri == p || ri == t)} (
    !a @ l,
    !read_data_v ri |
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot__rc rc', int ri, ptr l, size_t (sizeof a)
  ): #[rc': int -> int | rc' ri >= rc ri] void
  = "mac#_read_data"

extern fun takeout_read_data_ptr {a: t@ype} {rc: int -> int} {S, p, t, f, ri: nat | rc ri > 0 && (ri == p || ri == t)} (
    !read_data_v ri |
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot__rc rc', int ri, size_t (sizeof a)
  ): #[rc': int -> int | rc' ri >= rc ri] [l: agz] ((a @ l, (a @ l) -<lin,prf> void) | ptr l)
  = "mac#_takeout_read_data_ptr"

extern fun decr_rcount {i: int} {rc: int -> int | rc i > 0} (
    read_data_v i | !fixedslot__rc rc >> fixedslot__rc rc', int i
  ): #[rc': int -> int | rc' i == rc i - 1 ] void
  = "mac#_decr_rcount"

extern fun check_previous {S, p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f', rc')
  ): #[S', p', t', f': nat]
     #[rc': int -> int] void
  = "mac#_check_previous"

implement{a} fixedslot_read (fs) = buf where {
  var buf: a?

  val ri = pick_ri fs // atomic

  val (rd_v | ())  = incr_rcount (fs, ri) // atomic

  val () = decr_S (rd_v | fs) // atomic

  val () = read_data (rd_v | fs, ri, buf, sizeof<a>)

  val () = decr_rcount (rd_v | fs, ri) // atomic

  val () = check_previous fs // atomic
}

implement fixedslot_readptr (pf | fs, p, len) = () where {
  val ri = pick_ri fs // atomic

  val (rd_v | ())  = incr_rcount (fs, ri) // atomic

  val () = decr_S (rd_v | fs) // atomic

  val () = read_data_ptr (pf, rd_v | fs, ri, p, len)

  val () = decr_rcount (rd_v | fs, ri) // atomic

  val () = check_previous fs // atomic
}


implement{b} fixedslot_readfn {a} (fs, f, len) = let
  val ri = pick_ri fs // atomic

  val (rd_v | ())  = incr_rcount (fs, ri) // atomic

  val () = decr_S (rd_v | fs) // atomic

  val ((pf, fpf) | p) = takeout_read_data_ptr (rd_v | fs, ri, len)

  val x = f (!p)

  prval _ = fpf pf

  val () = decr_rcount (rd_v | fs, ri) // atomic

  val () = check_previous fs // atomic
in x end



// --------------------------------------------------
// writer

extern fun set_wfilled {f, f': nat} (!wfixedslot f >> wfixedslot f', int f'): void
  = "mac#_set_wfilled"
extern fun write_data {a: t@ype} {S, p, t, f: nat} {rc: int -> int}
                      {wi: nat | wi <> p && wi <> f && wi <> t} (  // can we prove rc wi == 0 ? (is it needed?)
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f, rc'),
    int wi, a, size_t (sizeof a)
  ): #[S', p', t': nat] #[rc': int -> int] void
  = "mac#_write_data"
extern fun write_data_ptr {a: t@ype} {S, p, t, f: nat} {rc: int -> int} {l: agz}
                          {wi: nat | wi <> p && wi <> f && wi <> t} (
    !a @ l |
    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f, rc'),
    int wi, ptr l, size_t (sizeof a)
  ): #[S', p', t': nat] #[rc': int -> int] void
  = "mac#_write_data_ptr"

//extern fun takeout_data_ptr {a: t@ype} {S, p, t, f: nat} {rc: int -> int} {l: agz}
//                            {wi: nat | wi <> p && wi <> f && wi <> t} (
//    !fixedslot_vt (S, p, t, f, rc) >> fixedslot_vt (S', p', t', f, rc'),
//    int wi)
//  ): #[S', p', t': nat] #[rc': int -> int] ((a @ l, (a @ l) -<lin,prf> ()) | ptr l)

// requires splitting up an atomic word into three component parts
extern fun get_triple {S, p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (S, p, t, f, rc) >> wfixedslot_post (p, t, f, rc), &int? >> int p, &int? >> int t, &int? >> int f
  ): void
  = "mac#_get_triple"

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
extern fun intset_nil ():<> intset nil = "mac#_intset_nil"
extern fun intset_cons {x: int} {s: set} (int x, intset s):<> (in_set (x, (cons (x, s))) | intset (cons (x, s))) = "mac#_intset_cons"
extern fun intset_find_first_not_in_set {s: set} (intset s):<> [x: nat] (not_in_set (x, s) | int x) = "mac#_intset_ffz"

// Theorem: If $x \in s$ and $y \not\in s$ then $x \neq y$.
prfun in_set_ne_not_in_set {x, y: int} {s: set} .<s>. (pf1: in_set (x, s), pf2: not_in_set (y, s)):<> [x <> y] void =
  case+ (pf1, pf2) of
  | (in_set_base1 x, not_in_set_cons pf2') => ()
  | (in_set_base2 pf1', not_in_set_cons pf2') => ()
  | (in_set_cons pf1', not_in_set_cons pf2') => in_set_ne_not_in_set (pf1', pf2')

// Pick value of $w_i\geq 0$ with proven property that $w_i \not\in \{p, t, f\}$.
extern fun pick_wi {S, p, t, f: nat} {rc: int -> int} (
    !fixedslot_vt (S, p, t, f, rc) >> wfixedslot_post (p, t, f, rc)
  ): #[wi: nat | wi <> p && wi <> f && wi <> t]
     int wi
implement pick_wi (fs) = wi where {
  var p:int
  var t:int
  var f:int
  val () = get_triple (fs, p, t, f)

  val (pf_p0 | s1) = intset_cons (p, intset_nil ())
  val (pf_t0 | s2) = intset_cons (t, s1)
  val (pf_f | s3)  = intset_cons (f, s2)
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

implement fixedslot_writeptr (pf | fs, p, len) = let
  val wi = pick_wi fs
  val () = write_data_ptr (pf | fs, wi, p, len)
  val () = set_wfilled (fs, wi)
in () end
