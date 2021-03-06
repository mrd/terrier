// fourslot: a 1-way pool-based async communication library

#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "prelude/SATS/status.sats"
staload "fourslot.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// small hack for naming convenience
sortdef bit = bool
typedef bit (x: bit) = bool x


(* ************************************************** *)

// when we view the state of 'reading' we know either it is equal to rp,
// or if it is not equal to rp, then WS1 precedes RS2
absview ws1_read_v (R: bit, rstep: int, rp: bit)

extern fun get_reading_state {l:addr} {n:nat} {a:t@ype} (
      pf_fs: !fourslot_v (l, n, a, true) | fs: ptr l
    ): [rstep: nat] [R, rp: bit | R == rp || (R <> rp ==> rstep < 2)]
       (ws1_read_v (R, rstep, rp) | bit R) = "mac#_get_reading_c"

// if WS1 preceeded RS2 then it also preceeded RS3
// we know slot[wp] == ri until WS4 consumes this proof
absview ws2_slot_v (s: bit, rp: bit, ri: bit)

extern fun get_write_slot_index {l:addr} {n:nat} {a:t@ype} {R, wp, rp: bit} {rstep: nat} (
    pf_fs: !fourslot_v (l, n, a, true), pfr: !ws1_read_v (R, rstep, rp) |
    fs: ptr l, wp: bit wp
  ): [s, ri: bit | (rstep < 3 && wp == rp) ==> s == ri]
     (ws2_slot_v (s, rp, ri) | bit s) = "mac#_get_write_slot_c"

// central assertion: wp <> rp || wi <> ri
extern fun{a:t@ype}
write_data {l: addr} {n: nat} {R, s, wp, wi, rp, ri: bit | wp <> rp || wi <> ri} {rstep: nat} (
    pf_fs: !fourslot_v (l, n, a, true),
    pfr: !ws1_read_v (R, rstep, rp),
    pfs: !ws2_slot_v (s, rp, ri) |
    fs: ptr l, wp: bit wp, wi: bit wi, item: a
  ): void

extern fun
write_data_c {l: addr} {n: nat} {a:t@ype} {R, s, wp, wi, rp, ri: bit | wp <> rp || wi <> ri} {rstep: nat} (
    pf_fs: !fourslot_v (l, n, a, true),
    pfr: !ws1_read_v (R, rstep, rp),
    pfs: !ws2_slot_v (s, rp, ri) |
    fs: ptr l, wp: bit wp, wi: bit wi, item: a, elsz: size_t (sizeof a)
  ): void = "mac#_write_data_c"

implement{a} write_data (pf_fs, pfr, pfs | fs, p, i, item) = write_data_c (pf_fs, pfr, pfs | fs, p, i, item, sizeof<a>)

// consume slot view since it is changed, and now this 'pair' is considered to be "fresh"
absview ws4_fresh_v (p: bit)

extern fun
save_write_slot_index {l:addr} {n:nat} {a:t@ype} {s, wp, wi, rp, ri: bit | wi <> s} (
    pf_fs: !fourslot_v (l, n, a, true),
    pfs: ws2_slot_v (s, rp, ri) |
    fs: ptr l, wp: bit wp, wi: bit wi
  ): (ws4_fresh_v wp | void) = "mac#_write_slot_c"

// updating 'latest' means reader can modify 'reading' state, so view is consumed
// the refreshed 'pair' is used to update 'latest' state
extern fun
save_latest_state {l:addr} {n:nat} {a:t@ype} {R, rp, wp: bit | wp <> R} {rstep: nat} (
    pf_fs: !fourslot_v (l, n, a, true),
    pfr: ws1_read_v (R, rstep, rp),
    pff: ws4_fresh_v wp |
    fs: ptr l, wp: bit wp
  ): void = "mac#_write_latest_c"

implement{a} fourslot_write (pf_fs | fs, item) = () where {
  val (pfr | R) = get_reading_state (pf_fs | fs)
  val wp = not R

  val (pfs | s) = get_write_slot_index (pf_fs, pfr | fs, wp)
  val wi = not s

  val _ = write_data<a> (pf_fs, pfr, pfs | fs, wp, wi, item)

  val (pff | _) = save_write_slot_index (pf_fs, pfs | fs, wp, wi)

  val _ = save_latest_state (pf_fs, pfr, pff | fs, wp)
}

(* ************************************************** *)

absview rs1_latest_v (L: bit)
extern fun get_latest_state {l:addr} {n:nat} {a:t@ype} (
    pf_fs: !fourslot_v (l, n, a, false) |
    fs: ptr l
  ): [L: bool] (rs1_latest_v L | bit L)
  = "mac#_get_latest_c"

(* The new value of 'reading' needs to be the same as 'latest' was *)
absview rs2_read_v (R: bit)
extern fun save_reading_state {l:addr} {n:nat} {a:t@ype} {L, rp: bit | L == rp} (
    pf_fs: !fourslot_v (l, n, a, false), pf: rs1_latest_v L | fs: ptr l, p: bit rp
  ): [R: bit | R == rp] (rs2_read_v R | void)
  = "mac#_write_reading_c"

(* Exchange the view of 'reading' for a slot view; we know that wi will be negated by other process *)
absview rs3_slot_v (s: bit, wp: bit, wi: bit)
extern fun get_read_slot_index {l:addr} {n:nat} {a:t@ype} {R, rp: bit} (
    pf_fs: !fourslot_v (l, n, a, false), pf: rs2_read_v R | fs: ptr l, p: bit rp
  ): [s, wp, wi: bool | wp == rp ==> s == ~wi] (rs3_slot_v (s, wp, wi) | bit s)
  = "mac#_get_read_slot_c"

(* Read the data from the slot, requires slot view *)
extern fun{a:t@ype} read_data {l:addr} {n:nat} {rp, ri, wp, wi: bool | wp <> rp || ri <> wi} (
    pf_fs: !fourslot_v (l, n, a, false), pf: rs3_slot_v (ri, wp, wi) | fs: ptr l, p: bit rp, i: bit ri
  ): a

extern fun read_data_c {a:t@ype} {l:addr} {n: nat} {rp, ri, wp, wi: bool | wp <> rp || ri <> wi} (
    pf_fs: !fourslot_v (l, n, a, false), pf: rs3_slot_v (ri, wp, wi) |
    fs: ptr l, p: bit rp, i: bit ri, item: &a? >> a, elsz: size_t (sizeof a)
  ): void
  = "mac#_read_data_c"

implement{a} read_data (pf_fs, pf | fs, p, i) = item where {
  var item: a?
  val _ = read_data_c (pf_fs, pf | fs, p, i, item, sizeof<a>)
}

implement{a} fourslot_read (pf_fs | fs) = item where {
  val (pfl | rp) = get_latest_state (pf_fs | fs)
  val (pfr | _) = save_reading_state (pf_fs, pfl | fs, rp)
  val (pfs | ri) = get_read_slot_index (pf_fs, pfr | fs, rp)
  val item = read_data<a> (pf_fs, pfs | fs, rp, ri)
}

(* ************************************************** *)

// pages >= 1 + ((1 + ((3 * 64 + 4 * 64 * (1 + ((sizeof a - 1) / 64))) - 1)) / 4096)

implement{a} fourslot_ipc_writer_init (pf | p, pages) = fourslot_ipc_writer_init_c (pf | p, pages, sizeof<a>)


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
