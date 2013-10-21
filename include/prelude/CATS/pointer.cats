/***********************************************************************/
/*                                                                     */
/*                         Applied Type System                         */
/*                                                                     */
/***********************************************************************/

/* (*
** ATS/Postiats - Unleashing the Potential of Types!
** Copyright (C) 2011-2012 Hongwei Xi, ATS Trustful Software, Inc.
** All rights reserved
**
** ATS is free software;  you can  redistribute it and/or modify it under
** the terms of the GNU LESSER GENERAL PUBLIC LICENSE as published by the
** Free Software Foundation; either version 2.1, or (at your option)  any
** later version.
**
** ATS is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without  even  the  implied  warranty  of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License
** for more details.
**
** You  should  have  received  a  copy of the GNU General Public License
** along  with  ATS;  see the  file COPYING.  If not, please write to the
** Free Software Foundation,  51 Franklin Street, Fifth Floor, Boston, MA
** 02110-1301, USA.
*) */

/* ****** ****** */

/*
** Source:
** $PATSHOME/prelude/CATS/CODEGEN/pointer.atxt
** Time of generation: Mon Oct 14 16:14:11 2013
*/

/* ****** ****** */

/*
(* Author: Hongwei Xi *)
(* Authoremail: hwxi AT cs DOT bu DOT edu *)
(* Start time: February, 2012 *)
*/

/* ****** ****** */

#ifndef ATSLIB_PRELUDE_POINTER_CATS
#define ATSLIB_PRELUDE_POINTER_CATS

/* ****** ****** */

ATSinline()
atstype_bool
atspre_ptr_is_null
  (atstype_ptr p) {
  return (p==(void*)0) ? atsbool_true : atsbool_false ;
} // end of [atspre_ptr_is_null]
#define atspre_ptr0_is_null atspre_ptr_is_null
#define atspre_ptr1_is_null atspre_ptr_is_null

ATSinline()
atstype_bool
atspre_ptr_isnot_null
  (atstype_ptr p) {
  return (p > (void*)0) ? atsbool_true : atsbool_false ;
} // end of [atspre_ptr_isnot_null]
#define atspre_ptr0_isnot_null atspre_ptr_isnot_null
#define atspre_ptr1_isnot_null atspre_ptr_isnot_null

/* ****** ****** */

ATSinline()
atstype_ptr
atspre_add_ptr_bsz
  (atstype_ptr p, atstype_size ofs) { return ((char*)p + ofs) ; }
// end of [atspre_add_ptr_bsz]
#define atspre_add_ptr0_bsz atspre_add_ptr_bsz
#define atspre_add_ptr1_bsz atspre_add_ptr_bsz

ATSinline()
atstype_ptr
atspre_sub_ptr_bsz
  (atstype_ptr p, atstype_size ofs) { return ((char*)p - ofs) ; }
// end of [atspre_sub_ptr_bsz]
#define atspre_sub_ptr0_bsz atspre_sub_ptr_bsz
#define atspre_sub_ptr1_bsz atspre_sub_ptr_bsz

/* ****** ****** */

ATSinline()
atstype_ssize
atspre_sub_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) { return (p1 - p2) ; }
// end of [atspre_sub_ptr_ptr]
#define atspre_sub_ptr0_ptr0 atspre_sub_ptr_ptr
#define atspre_sub_ptr1_ptr1 atspre_sub_ptr_ptr

/* ****** ****** */

ATSinline()
atstype_bool
atspre_lt_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 < p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_lt_ptr_ptr]
#define atspre_lt_ptr0_ptr0 atspre_lt_ptr_ptr
#define atspre_lt_ptr1_ptr1 atspre_lt_ptr_ptr

ATSinline()
atstype_bool
atspre_lte_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 <= p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_lte_ptr_ptr]
#define atspre_lte_ptr0_ptr0 atspre_lte_ptr_ptr
#define atspre_lte_ptr1_ptr1 atspre_lte_ptr_ptr

ATSinline()
atstype_bool
atspre_gt_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 > p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_gt_ptr_ptr]
#define atspre_gt_ptr0_ptr0 atspre_gt_ptr_ptr
#define atspre_gt_ptr1_ptr1 atspre_gt_ptr_ptr

ATSinline()
atstype_bool
atspre_gte_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 >= p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_gte_ptr_ptr]
#define atspre_gte_ptr0_ptr0 atspre_gte_ptr_ptr
#define atspre_gte_ptr1_ptr1 atspre_gte_ptr_ptr

ATSinline()
atstype_bool
atspre_eq_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 == p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_eq_ptr_ptr]
#define atspre_eq_ptr0_ptr0 atspre_eq_ptr_ptr
#define atspre_eq_ptr1_ptr1 atspre_eq_ptr_ptr

ATSinline()
atstype_bool
atspre_neq_ptr_ptr
  (atstype_ptr p1, atstype_ptr p2) {
  return (p1 != p2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_neq_ptr_ptr]
#define atspre_neq_ptr0_ptr0 atspre_neq_ptr_ptr
#define atspre_neq_ptr1_ptr1 atspre_neq_ptr_ptr


ATSinline()
atstype_int
atspre_compare_ptr_ptr
(
  atstype_ptr p1, atstype_ptr p2
) {
  if (p1 >= p2) {
    if (p1 > p2) return 1 ; else return 0 ;
  } else return (-1) ;
} // end of [atspre_compare_ptr_ptr]
#define atspre_compare_ptr0_ptr0 atspre_compare_ptr_ptr
#define atspre_compare_ptr1_ptr1 atspre_compare_ptr_ptr

/* ****** ****** */

ATSinline()
atstype_bool
atspre_gt_ptr_intz
  (atstype_ptr p, atstype_int _)
{
  return (p > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_gt_ptr_intz]
#define atspre_gt_ptr0_intz atspre_gt_ptr_intz
#define atspre_gt_ptr1_intz atspre_gt_ptr_intz

ATSinline()
atstype_bool
atspre_eq_ptr_intz
  (atstype_ptr p, atstype_int _)
{
  return (p == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_eq_ptr_intz]
#define atspre_eq_ptr0_intz atspre_eq_ptr_intz
#define atspre_eq_ptr1_intz atspre_eq_ptr_intz

ATSinline()
atstype_bool
atspre_neq_ptr_intz
  (atstype_ptr p, atstype_int _)
{
  return (p != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_neq_ptr_intz]
#define atspre_neq_ptr0_intz atspre_neq_ptr_intz
#define atspre_neq_ptr1_intz atspre_neq_ptr_intz

/* ****** ****** */

#define atspre_cptr_null() atsptr_null

/* ****** ****** */

#define atspre_cptr_is_null atspre_ptr_is_null
#define atspre_cptr_isnot_null atspre_ptr_isnot_null

/* ****** ****** */

#define atspre_ptr_alloc_tsz atspre_malloc_gc

/* ****** ****** */

#define atspre_ptr_free(p) atspre_mfree_gc(p)

/* ****** ****** */

ATSinline()
atsvoid_t0ype
atspre_fprint_ptr (
  atstype_ref out, atstype_ptr x
) {
  //int err ;
  //err = fprintf((FILE*)out, "%p", x) ;
  return ;
} // end [atspre_fprint_ptr]

#define atspre_print_ptr(x) atspre_fprint_ptr(stdout, (x))
#define atspre_prerr_ptr(x) atspre_fprint_ptr(stderr, (x))

/* ****** ****** */

#endif // ifndef ATSLIB_PRELUDE_POINTER_CATS

/* ****** ****** */

/* end of [pointer.cats] */

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
