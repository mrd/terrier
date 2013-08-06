// fourslot: a 1-way pool-based async communication library

#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "fourslot.sats"

(* ************************************************** *)

absview read_v (p: bool)                  (* view of 'reading' *)
absview slot_v (i:bool,p':bool,i': bool)  (* view of slot[i] where (p',i') are used by other process *)

extern fun get_reading {l:addr} {n:nat} {a:t@ype} (
    pf_fs: !fourslot_v (l,n,a,true) | fs: ptr l
  ): [reading: bool] (read_v reading | bool reading) = "mac#_get_reading_c"

(* get_write_slot asserts that we can only read from the 'not reading' slot and if wp == rp then wi == ri *)
extern fun get_write_slot {l:addr} {n:nat} {a:t@ype} {wp: bool} (
    pf_fs: !fourslot_v (l,n,a,true), pfr: read_v (~wp) |
    fs: ptr l, p: bool wp
  ): [wi',rp,ri: bool | wp <> rp || wi' == ri] (slot_v (wi',rp,ri) | bool wi')
  = "mac#_get_write_slot_c"

(* Primary assertion: wp <> rp || wi <> ri. Either the pairs are unequal or the indices are unequal. *)
(* We also need to know that the current slot has the opposite value from our index. *)
extern fun{a:t@ype} write_data {l: addr} {n: nat} {wp,wi,rp,ri: bool | wp <> rp || wi <> ri} (
    pf_fs: !fourslot_v (l, n, a, true),
    pf_s: !slot_v (~wi, rp, ri) |
    fs: ptr l, p: bool wp, i: bool wi, item: a
  ): void

extern fun write_data_c {a:t@ype} {l: addr} {n,e: nat} {wp,wi,rp,ri: bool | wp <> rp || wi <> ri} (
    pf_fs: !fourslot_v (l, n, a, true),
    pf_s: !slot_v (~wi, rp, ri) |
    fs: ptr l, p: bool wp, i: bool wi, item: a, elsz: size_t e
  ): void = "mac#_write_data_c"

implement{a} write_data (pf_fs, pf_s | fs, p, i, item) = write_data_c (pf_fs, pf_s | fs, p, i, item, sizeof<a>)

(* Write back the slot value knowing that the existing value is the opposite. *)
(* Regain the view of 'reading' so that it must be dealt with. *)
extern fun write_slot {l:addr} {n:nat} {a:t@ype} {wp, wi, rp, ri: bool} (
    pf_fs: !fourslot_v (l, n, a, true),
    pf: slot_v (~wi,rp,ri) |
    fs: ptr l, p: bool wp, i: bool wi
  ): (read_v (~wp) | void) = "mac#_write_slot_c"

(* Discharge the read_v by writing it with the opposite value to 'latest' *)
extern fun write_latest {l:addr} {n:nat} {a:t@ype} {reading: bool} (
    pf_fs: !fourslot_v (l, n, a, true), pf: read_v reading |
    fs: ptr l, p: bool (~reading)
  ): void = "mac#_write_latest_c"

implement{a} fourslot_write (pf_fs | fs, item) = () where {
  val (pfr | reading) = get_reading (pf_fs | fs)
  val wp = not reading
  val (pfs | wi) = get_write_slot (pf_fs, pfr | fs, wp)
  val _ = write_data (pf_fs, pfs | fs, wp, not wi, item)
  val (pfr | _) = write_slot (pf_fs, pfs | fs, wp, not wi)
  val _ = write_latest (pf_fs, pfr | fs, wp)
}

(* ************************************************** *)

absview latest_v (p: bool)      (* view of 'latest' *)

extern fun get_latest {l:addr} {n:nat} {a:t@ype} (
    pf_fs: !fourslot_v (l, n, a, false) |
    fs: ptr l
  ): [latest: bool] (latest_v latest | bool latest)
  = "mac#_get_latest_c"

(* The new value of 'reading' needs to be the same as 'latest' was *)
extern fun write_reading {l:addr} {n:nat} {a:t@ype} {rp: bool} (
    pf_fs: !fourslot_v (l, n, a, false), pf: latest_v rp | fs: ptr l, p: bool rp
  ): (read_v rp | void)
  = "mac#_write_reading_c"

(* Exchange the view of 'reading' for a slot view; we know that wi will be negated by other process *)
extern fun get_read_slot {l:addr} {n:nat} {a:t@ype} {rp: bool} (
    pf_fs: !fourslot_v (l, n, a, false), pf: read_v rp | fs: ptr l, p: bool rp
  ): [ri,wp,wi: bool | wp <> rp || ri == wi] (slot_v (ri, wp, ~wi) | bool ri)
  = "mac#_get_read_slot_c"

(* Read the data from the slot, requires slot view *)
extern fun{a:t@ype} read_data {l:addr} {n:nat} {rp, ri, wp, wi: bool | wp <> rp || ri <> wi} (
    pf_fs: !fourslot_v (l, n, a, false), pf: !slot_v (ri, wp, wi) | fs: ptr l, p: bool rp, i: bool ri
  ): a

extern fun read_data_c {a:t@ype} {l:addr} {n,e:nat | e == sizeof a} {rp, ri, wp, wi: bool | wp <> rp || ri <> wi} (
    pf_fs: !fourslot_v (l, n, a, false), pf: !slot_v (ri, wp, wi) | fs: ptr l, p: bool rp, i: bool ri, item: &a? >> a, elsz: size_t e
  ): void
  = "mac#_read_data_c"

implement{a} read_data (pf_fs, pf | fs, p, i) = item where {
  var item: a?
  val _ = read_data_c (pf_fs, pf | fs, p, i, item, sizeof<a>)
}

extern prfun release_read_slot {i,p',i':bool} (pf: slot_v (i,p',i')): void

implement{a} fourslot_read (pf_fs | fs) = read where {
  val [rp: bool] (pfl | rp) = get_latest (pf_fs | fs)
  val (pfr | _) = write_reading (pf_fs, pfl | fs, rp)
  val (pfs | ri) = get_read_slot (pf_fs, pfr | fs, rp)
  val read = read_data (pf_fs, pfs | fs, rp, ri)
  prval _ = release_read_slot pfs
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
