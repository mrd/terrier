#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "fixedslot.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// --------------------------------------------------
// reader

extern fun rrcount {rc: int -> int} {i: nat} (
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
  val () = if rrcount (fs, rprevious fs) = 0
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

extern fun wrtarget {f: nat} (!fixedslot__f f >> fixedslot__t_f (t', f)): #[t': nat | t' == f] int t'
  = "mac#_rtarget"

extern fun wfilled {f: nat} (!wfixedslot f >> wfixedslot f): int f
  = "mac#_wfilled"
extern fun set_wfilled {f, f': nat} (!wfixedslot f >> wfixedslot f', int f'): void
  = "mac#_set_wfilled"
extern fun write_data {a: t@ype} {p, t, f: nat} {rc: int -> int}
                      {wi: nat | rc wi == 0 && wi <> t} (
    !fixedslot_vt (p, t, f, rc) >> fixedslot_vt (p', t', f, rc'),
    int wi, !a, size_t (sizeof a)
  ): #[p', t': nat] #[rc': int -> int] void
  = "mac#_write_data"

extern fun pick_wi {f: nat} (
    !fixedslot__f f >> fixedslot_vt (p', t', f, rc')
  ): #[p', t': nat]
     #[rc': int -> int]
     #[wi: nat | rc' wi == 0 && wi <> f && wi <> t']
     int wi
// implement pick_wi (fs) = ...

implement{a} fixedslot_write (fs, item) = let
  val wi = pick_wi (fs)
  // val _  = set_wfilling (fs, wi) // unnecessary to code
  val () = write_data (fs, wi, item, sizeof<a>)
  val () = set_wfilled (fs, wi)
in () end

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
