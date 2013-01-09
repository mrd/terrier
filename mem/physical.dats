(*
 * Physical memory manager
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2011-2013 Matthew Danish.
 *
 * All rights reserved. Please see enclosed LICENSE file for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *)

#define ATS_DYNLOADFLAG 0

staload "physical.sats"

// prototypes for various helper functions, types and values

abst@ype physaddr = int
abst@ype status = int
extern val nullphysaddr: physaddr = "mac#nullphysaddr"
extern val OK: status = "mac#OK"
extern val ENOSPACE: status = "mac#ENOSPACE"

extern val num_bitmaps: int = "ext#num_bitmaps"
extern fun bitmap_num_bytes (_: int): int = "mac#bitmap_num_bytes"
extern fun bitmap_tst (_: int, _: int, _: int): bool = "mac#bitmap_tst"
extern fun bitmap_set (_: int, _: int, _: int): void = "mac#bitmap_set"
extern fun bitmap_compute_start_physaddr (_: int, _: int, _: int): physaddr = "mac#bitmap_compute_start_physaddr"
extern fun is_aligned (_: int, _: int): bool = "mac#is_aligned"

(* ************************************************** *)

typedef searchstate = @(int, int, int) (* (m, byt, bit) *)

// return next searchstate
fun inc_searchstate((m, byt, bit): searchstate): searchstate =
  if bit + 1 < 8 then (m, byt, bit + 1)
  else if byt + 1 < bitmap_num_bytes m then (m, byt + 1, 0)
  else (m + 1, 0, 0)

(* ************************************************** *)

// search for contiguous physical pages
fun search ( n: int, align: int, count: int
           , curr: searchstate, saved: searchstate ): @(int, searchstate) = let
  val (m, byt, bit) = curr
  val curr' = inc_searchstate curr
in
  if m = num_bitmaps then
    // reached end of bitmap sequence
    (count, saved)
  else if count = 0 then
    // found n unallocated pages - terminate search
    (0, saved)
  else if bitmap_tst (m, byt, bit) then
    // found an allocated page - reset count
    search (n, align, n, curr', @(~1, 0, 0))
  else if count = n && is_aligned (bit, align) then
    // found an unallocated, aligned page - start span
    search (n, align, count - 1, curr', @(m, byt, bit))
  else if count = n then
    // found an unallocated, unaligned page - skip it
    search (n, align, count, curr', @(~1, 0, 0))
  else
    // found an unallocated page mid-span - keep counting down
    search (n, align, count - 1, curr', saved)
end // end [fun search]

(* ************************************************** *)

// mark pages that were found
fun mark (count: int, curr: searchstate): void = let
  val (m: int, byt: int, bit: int) = curr
  val curr' = inc_searchstate curr
in
  if count > 0 then begin
    bitmap_set (m, byt, bit);
    mark (count - 1, curr')
  end else ()
end // end [fun mark]

(* ************************************************** *)

// allocate 'n' contiguous physical pages aligned to 'align' pages
extern fun physical_alloc_pages (n: int, align: int, addr: &physaddr? >> physaddr): status = "ext#_ats_physical_alloc_pages"

implement physical_alloc_pages(n, align, addr) = let
  val (n', saved) = search (n, align, n, @(0, 0, 0), @(~1, 0, 0))
  val (saved_m, saved_byt, saved_bit) = saved
in
  if saved_m = ~1 || n' <> 0 then begin
    addr := nullphysaddr; ENOSPACE
  end else begin
    mark (n, saved);
    addr := bitmap_compute_start_physaddr (saved_m, saved_byt, saved_bit); OK
  end
end // end [fun physical_alloc_pages]
