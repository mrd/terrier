// multireader: a 1-way pool-based async communication library with options for multiple readers/writers

#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "multireader.sats"
infix ==>

(* ************************************************** *)
// READER

absview rs1_v (WS3occ: int, OP: bit, IP: bit)
extern fun rs1 {l: addr} {a: t@ype} {n, i: nat} (
    pf_ms: !mslot_v (l, n, a, false, i) | ms: ptr l, i: int i
  ): [OP, IP: bit] [WS3occ: nat] (rs1_v (WS3occ, OP, IP) | void)
  = "mac#_rs1"

absview rs2_v (WS3occ: int, OP: bit, IP: bit, Rip: bit, Wip: bit, overlap: bool)
extern fun rs2 {l: addr} {a: t@ype} {OP, IP: bit} {n, i, WS3occ: nat} (
    pf_ms: !mslot_v (l, n, a, false, i),
    pf_rs1: rs1_v (WS3occ, OP, IP) |
    ms: ptr l, i: int i
  ): [overlap: bool]
     [OP', IP': bit | (WS3occ == 0 ==> (OP' != IP' || overlap == false))]
     [Rip, Wip: bit | ((WS3occ > 0 && OP' == IP') ==> (Rip != Wip || overlap == false))]
     (rs2_v (WS3occ, OP', IP', Rip, Wip, overlap) | void)
  = "mac#_rs2"

extern fun _rs3 {a:t@ype}
                {l1, l2: addr}
                {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} {i,WS3occ: nat} (
    pf_rs2: rs2_v (WS3occ, OP, IP, Rip, Wip, overlap) |
    ms: ptr l1, i: int i, dest: &a? >> a, size: size_t (sizeof a)
  ): void = "mac#_rs3"

fun{a:t@ype} rs3 {l: addr} {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} {n, i, WS3occ: nat} (
    pf_ms: !mslot_v (l, n, a, false, i),
    pf_rs2: rs2_v (WS3occ, OP, IP, Rip, Wip, overlap) |
    ms: ptr l, i: int i
  ): a = item where {
  var item: a?
  val _ = _rs3 (pf_rs2 | ms, i, item, sizeof<a>)
}

implement{a} multireader_read (pf_ms | ms, i) = let
  val (pf_rs1 | _) = rs1 (pf_ms | ms, i)
  val (pf_rs2 | _) = rs2 (pf_ms, pf_rs1 | ms, i)
in
  rs3<a> (pf_ms, pf_rs2 | ms, i)
end

extern fun multireader_initialize_reader_c {a: t@ype} {l:addr | l > null} {n: nat} (
    pf: ipcmem_v (l, n) | p: ptr l, pages: int n, elemsize: size_t (sizeof a), index: &int? >> int i
  ): #[i:nat] [s:int] (either_v (ipcmem_v (l, n), mslot_v (l, n, a, false, i), s == 0) | status s)
  = "mac#_multireader_initialize_reader_c"

implement{a} multireader_initialize_reader (pf | p, pages, iref) =
  multireader_initialize_reader_c (pf | p, pages, sizeof<a>, iref)


(* ************************************************** *)
// WRITER

extern fun _ws1 {l: addr} {a: t@ype} {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} (
    pf_ws3: ws3_v (OP, IP, Rip, Wip, overlap) |
    ms: ptr l,
    item: a,
    size: size_t (sizeof a)
  ): void = "mac#_ws1"

fun{a: t@ype} ws1 {l: addr} {overlap: bool} {n: nat} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} (
    pf_ms: !mslot_v (l, n, a, true, 0),
    pf_ws3: ws3_v (OP, IP, Rip, Wip, overlap) |
    ms: ptr l,
    item: a
  ): void = _ws1 (pf_ws3 | ms, item, sizeof<a>)

absview ws2_v (RS1occ: int, OP: bit, IP: bit, overlap: bool)
extern fun ws2 {l: addr} {a: t@ype} {n: nat} (
    pf_ms: !mslot_v (l, n, a, true, 0) | ms: ptr l
  ): [OP, IP: bit] [overlap: bool] [RS1occ: nat | RS1occ > 0 ==> (OP != IP || (overlap == false))]
     (ws2_v (RS1occ, OP, IP, overlap) | void)
  = "mac#_ws2"

extern fun ws3 {l: addr} {a: t@ype} {n, RS1occ: nat} {overlap: bool} {OP, IP: bit} (
    pf_ms: !mslot_v (l, n, a, true, 0),
    pf_ws2: ws2_v (RS1occ, OP, IP, overlap) |
    ms: ptr l
  ): [Rip, Wip: bit | (RS1occ == 0 && OP == IP) ==> (Rip != Wip)] (ws3_v (OP, IP, Rip, Wip, overlap) | void)
  = "mac#_ws3"

implement{a} multireader_write (pf_ms, pf_ws3 | ms, item) = () where {
  val _ = ws1<a> (pf_ms, pf_ws3 | ms, item)
  val (pf_ws2  | _) = ws2 (pf_ms | ms)
  val (pf_ws3' | _) = ws3 (pf_ms, pf_ws2 | ms)
  prval _ = pf_ws3 := pf_ws3'
}

extern fun multireader_initialize_writer_c {a: t@ype} {l:addr | l > null} {n: nat} (
    pf: ipcmem_v (l, n) | p: ptr l, pages: int n, elemsize: size_t (sizeof a)
  ): [s:int]
     [overlap': bool]
     [OP', IP', Rip', Wip': bit | (OP' != IP') || (Rip' != Wip') || (overlap' == false)]
     (either_v (ipcmem_v (l, n), (mslot_v (l, n, a, true, 0), ws3_v (OP', IP', Rip', Wip', overlap')), s == 0) | status s)
  = "mac#_multireader_initialize_writer_c"

implement{a} multireader_initialize_writer (pf | p, pages) =
  multireader_initialize_writer_c (pf | p, pages, sizeof<a>)


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
