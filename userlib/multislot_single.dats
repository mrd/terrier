// multislot: a 1-way pool-based async communication library with options for multiple readers/writers

// single-reader/writer reimplementation for reference, using new notation/mechanism

#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "multislot.sats"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// small hack for naming convenience
sortdef bit = bool
typedef bit (x: bit) = bool x

(* ************************************************** *)
// READER

absview mslot_v (l: addr)

absview rs1_v (WS3occ: int, OP: bit, IP: bit)
extern fun rs1 {l: addr} (pf_ms: !mslot_v (l) | ms: ptr l): [OP, IP: bit] [WS3occ: nat] (rs1_v (WS3occ, OP, IP) | void)

absview rs2_v (WS3occ: int, OP: bit, IP: bit, Rip: bit, Wip: bit, overlap: bool)
extern fun rs2 {l: addr} {OP, IP: bit} {WS3occ: nat} (
    pf_ms: !mslot_v (l),
    pf_rs1: rs1_v (WS3occ, OP, IP) |
    ms: ptr l
  ): [overlap: bool]
     [OP', IP': bit | (WS3occ == 0 ==> (OP' != IP' || overlap == false))]
     [Rip, Wip: bit | ((WS3occ > 0 && OP' == IP') ==> (Rip != Wip || overlap == false))]
     (rs2_v (WS3occ, OP', IP', Rip, Wip, overlap) | void)

extern fun rs3 {l: addr} {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} {WS3occ: nat} (
    pf_ms: !mslot_v (l),
    pf_rs2: rs2_v (WS3occ, OP, IP, Rip, Wip, overlap) |
    ms: ptr l
  ): void

extern fun read {l: addr} (pf_ms: !mslot_v (l) | ms: ptr l): void
implement read (pf_ms | ms) = let
  val (pf_rs1 | _) = rs1 (pf_ms | ms)
  val (pf_rs2 | _) = rs2 (pf_ms, pf_rs1 | ms)
in
  rs3 (pf_ms, pf_rs2 | ms)
end

(* ************************************************** *)
// WRITER

absview ws3_v (OP: bit, IP: bit, Rip: bit, Wip: bit, overlap: bool)

extern fun ws1 {l: addr} {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} (
    pf_ms: !mslot_v (l),
    pf_ws3: ws3_v (OP, IP, Rip, Wip, overlap) |
    ms: ptr l
  ): void

absview ws2_v (RS1occ: int, OP: bit, IP: bit, overlap: bool)
extern fun ws2 {l: addr} (
    pf_ms: !mslot_v (l) | ms: ptr l
  ): [OP, IP: bit] [overlap: bool] [RS1occ: nat | RS1occ > 0 ==> (OP != IP || (overlap == false))]
     (ws2_v (RS1occ, OP, IP, overlap) | void)

extern fun ws3 {l: addr} {RS1occ: nat} {overlap: bool} {OP, IP: bit} (
    pf_ms: !mslot_v (l),
    pf_ws2: ws2_v (RS1occ, OP, IP, overlap) |
    ms: ptr l
  ): [Rip, Wip: bit | (RS1occ == 0 && OP == IP) ==> (Rip != Wip)] (ws3_v (OP, IP, Rip, Wip, overlap) | void)


extern fun initial_ws3_v (): [overlap': bool]
                             [OP', IP', Rip', Wip': bit | (OP' != IP') || (Rip' != Wip') || (overlap' == false)]
                             (ws3_v (OP', IP', Rip', Wip', overlap') | void)

extern fun write {l: addr} {overlap: bool} {OP, IP, Rip, Wip: bit | (OP != IP) || (Rip != Wip) || (overlap == false)} (
    pf_ms: !mslot_v (l), pf_ws3: ws3_v (OP, IP, Rip, Wip, overlap) | ms: ptr l
  ): [overlap': bool]
     [OP', IP', Rip', Wip': bit | (OP' != IP') || (Rip' != Wip') || (overlap' == false)]
     (ws3_v (OP', IP', Rip', Wip', overlap') | void)
implement write (pf_ms, pf_ws3 | ms) = let
  val _ = ws1 (pf_ms, pf_ws3 | ms)
  val (pf_ws2  | _) = ws2 (pf_ms | ms)
in
  ws3 (pf_ms, pf_ws2 | ms)
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
