// fourslot2w: two fourslot instances for 2-way pool-based communications

#define ATS_DYNLOADFLAG 0

staload "ipcmem.sats"
staload "fourslot.sats"
staload "fourslot2w.sats"
staload _ = "fourslot.dats"

// Splitting and joining views of fourslot2w so that it can be used
// with the existing fourslot code.

absview fourslot2w_split_v (l1: addr, l2: addr)

// splitA
extern fun fourslot2w_splitA_c {a1,a2:t@ype} {l1: addr} {pages: nat} (
    pf: fourslot2w_v (l1, pages, a1, a2, A) | p: ptr l1, sz1: size_t (sizeof a1), sz2: size_t (sizeof a2)
  ): [l2: addr] [pg1, pg2: nat | pg1 + pg2 == pages]
     ((fourslot2w_split_v (l1, l2), fourslot_v (l1, pg1, a1, false), fourslot_v (l2, pg2, a2, true)) | ptr l2)
  = "mac#fourslot2w_splitA_c"

extern fun{a1,a2:t@ype} fourslot2w_splitA {l1: addr} {pages: nat} (
    pf: fourslot2w_v (l1, pages, a1, a2, A) | p: ptr l1
  ): [l2: addr] [pg1, pg2: nat | pg1 + pg2 == pages]
     ((fourslot2w_split_v (l1, l2), fourslot_v (l1, pg1, a1, false), fourslot_v (l2, pg2, a2, true)) | ptr l2)

implement{a1,a2} fourslot2w_splitA (pf | p) = fourslot2w_splitA_c (pf | p, sizeof<a1>, sizeof<a2>)

// splitB
extern fun fourslot2w_splitB_c {a1,a2:t@ype} {l1: addr} {pages: nat} (
    pf: fourslot2w_v (l1, pages, a1, a2, B) | p: ptr l1, sz1: size_t (sizeof a1), sz2: size_t (sizeof a2)
  ): [l2: addr] [pg1, pg2: nat | pg1 + pg2 == pages]
     ((fourslot2w_split_v (l1, l2), fourslot_v (l1, pg1, a1, true), fourslot_v (l2, pg2, a2, false)) | ptr l2)
  = "mac#fourslot2w_splitB_c"

extern fun{a1,a2:t@ype} fourslot2w_splitB {l1: addr} {pages: nat} (
    pf: fourslot2w_v (l1, pages, a1, a2, B) | p: ptr l1
  ): [l2: addr] [pg1, pg2: nat | pg1 + pg2 == pages]
     ((fourslot2w_split_v (l1, l2), fourslot_v (l1, pg1, a1, true), fourslot_v (l2, pg2, a2, false)) | ptr l2)

implement{a1,a2} fourslot2w_splitB (pf | p) = fourslot2w_splitB_c (pf | p, sizeof<a1>, sizeof<a2>)

// joins are purely proof-level
extern prfun fourslot2w_joinA {a1,a2: t@ype} {l1,l2: addr} {pg1, pg2: nat} (
    pf_s: fourslot2w_split_v (l1, l2), pf1: fourslot_v (l1, pg1, a1, false), pf2: fourslot_v (l2, pg2, a2, true)
  ): fourslot2w_v (l1, pg1 + pg2, a1, a2, A)

extern prfun fourslot2w_joinB {a1,a2: t@ype} {l1,l2: addr} {pg1, pg2: nat} (
    pf_s: fourslot2w_split_v (l1, l2), pf1: fourslot_v (l1, pg1, a1, true), pf2: fourslot_v (l2, pg2, a2, false)
  ): fourslot2w_v (l1, pg1 + pg2, a1, a2, B)


(* ************************************************** *)

implement{a1,a2} fourslot2w_init (pf | p, pages, s) = fourslot2w_init_c (pf | p, pages, s, sizeof<a1>, sizeof<a2>)

// A-side
implement{a1,a2} fourslot2w_readA (pf_fs | p1) = let
  val ((pf_s, pf1, pf2) | p2) = fourslot2w_splitA<a1,a2> (pf_fs | p1)
  val x = fourslot_read<a1> (pf1 | p1)
  prval _ = pf_fs := fourslot2w_joinA (pf_s, pf1, pf2)
in
  x
end

implement{a1,a2} fourslot2w_writeA (pf_fs | p1, item) = let
  val ((pf_s, pf1, pf2) | p2) = fourslot2w_splitA<a1,a2> (pf_fs | p1)
  val _ = fourslot_write<a2> (pf2 | p2, item)
  prval _ = pf_fs := fourslot2w_joinA (pf_s, pf1, pf2)
in
end

// B-side
implement{a1,a2} fourslot2w_readB (pf_fs | p1) = let
  val ((pf_s, pf1, pf2) | p2) = fourslot2w_splitB<a1,a2> (pf_fs | p1)
  val x = fourslot_read<a2> (pf2 | p2)
  prval _ = pf_fs := fourslot2w_joinB (pf_s, pf1, pf2)
in
  x
end

implement{a1,a2} fourslot2w_writeB (pf_fs | p1, item) = let
  val ((pf_s, pf1, pf2) | p2) = fourslot2w_splitB<a1,a2> (pf_fs | p1)
  val _ = fourslot_write<a1> (pf1 | p1, item)
  prval _ = pf_fs := fourslot2w_joinB (pf_s, pf1, pf2)
in
end


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
