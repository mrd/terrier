/***********************************************************************/
/*                                                                     */
/*                         Applied Type System                         */
/*                                                                     */
/***********************************************************************/

/* (*
** ATS/Postiats - Unleashing the Potential of Types!
** Copyright (C) 2010-2013 Hongwei Xi, ATS Trustful Software, Inc.
** All rights reserved
**
** ATS is free software;  you can  redistribute it and/or modify it under
** the terms of  the GNU GENERAL PUBLIC LICENSE (GPL) as published by the
** Free Software Foundation; either version 3, or (at  your  option)  any
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
** $PATSHOME/prelude/CATS/CODEGEN/integer.atxt
** Time of generation: Tue Oct 28 01:40:31 2014
*/

/* ****** ****** */

/*
(* Author: Hongwei Xi *)
(* Authoremail: hwxi AT cs DOT bu DOT edu *)
(* Start time: January, 2013 *)
*/

/* ****** ****** */

#ifndef ATSLIB_PRELUDE_CATS_INTEGER
#define ATSLIB_PRELUDE_CATS_INTEGER

/* ****** ****** */
//
#define atspre_g0int2int_int_int(x) (x)
#define atspre_g0int2int_int_lint(x) ((atstype_lint)(x))
#define atspre_g0int2int_int_llint(x) ((atstype_llint)(x))
#define atspre_g0int2int_int_ssize(x) ((atstype_ssize)(x))
#define atspre_g1int2int_int_int atspre_g0int2int_int_int
#define atspre_g1int2int_int_lint atspre_g0int2int_int_lint
#define atspre_g1int2int_int_llint atspre_g0int2int_int_llint
#define atspre_g1int2int_int_ssize atspre_g0int2int_int_ssize
//
#define atspre_g0int2int_lint_int(x) ((atstype_int)(x))
#define atspre_g0int2int_lint_lint(x) (x)
#define atspre_g0int2int_lint_llint(x) ((atstype_llint)(x))
#define atspre_g0int2int_lint_ssize(x) ((atstype_ssize)(x))
#define atspre_g1int2int_lint_int atspre_g0int2int_lint_int
#define atspre_g1int2int_lint_lint atspre_g0int2int_lint_lint
#define atspre_g1int2int_lint_llint atspre_g0int2int_lint_llint
#define atspre_g1int2int_lint_ssize atspre_g0int2int_lint_ssize
//
#define atspre_g0int2int_ssize_int(x) ((atstype_int)(x))
#define atspre_g0int2int_ssize_lint(x) ((atstype_lint)(x))
#define atspre_g0int2int_ssize_llint(x) ((atstype_llint)(x))
#define atspre_g0int2int_ssize_ssize(x) (x)
#define atspre_g1int2int_ssize_int atspre_g0int2int_ssize_int
#define atspre_g1int2int_ssize_lint atspre_g0int2int_ssize_lint
#define atspre_g1int2int_ssize_llint atspre_g0int2int_ssize_llint
#define atspre_g1int2int_ssize_ssize atspre_g0int2int_ssize_ssize

#define atspre_g1int_neg_int atspre_g0int_neg_int
#define atspre_g1int_abs_int atspre_g0int_abs_int
#define atspre_g1int_succ_int atspre_g0int_succ_int
#define atspre_g1int_pred_int atspre_g0int_pred_int
#define atspre_g1int_half_int atspre_g0int_half_int
#define atspre_g1int_add_int atspre_g0int_add_int
#define atspre_g1int_sub_int atspre_g0int_sub_int
#define atspre_g1int_mul_int atspre_g0int_mul_int
#define atspre_g1int_div_int atspre_g0int_div_int
#define atspre_g1int_nmod_int atspre_g0int_nmod_int
#define atspre_g1int_isltz_int atspre_g0int_isltz_int
#define atspre_g1int_isltez_int atspre_g0int_isltez_int
#define atspre_g1int_isgtz_int atspre_g0int_isgtz_int
#define atspre_g1int_isgtez_int atspre_g0int_isgtez_int
#define atspre_g1int_iseqz_int atspre_g0int_iseqz_int
#define atspre_g1int_isneqz_int atspre_g0int_isneqz_int
#define atspre_g1int_lt_int atspre_g0int_lt_int
#define atspre_g1int_lte_int atspre_g0int_lte_int
#define atspre_g1int_gt_int atspre_g0int_gt_int
#define atspre_g1int_gte_int atspre_g0int_gte_int
#define atspre_g1int_eq_int atspre_g0int_eq_int
#define atspre_g1int_neq_int atspre_g0int_neq_int
#define atspre_g1int_compare_int atspre_g0int_compare_int
#define atspre_g1int_max_int atspre_g0int_max_int
#define atspre_g1int_min_int atspre_g0int_min_int

/* ****** ****** */

#define atspre_g1int_neg_int atspre_g0int_neg_int
#define atspre_g1int_abs_int atspre_g0int_abs_int
#define atspre_g1int_succ_int atspre_g0int_succ_int
#define atspre_g1int_pred_int atspre_g0int_pred_int
#define atspre_g1int_half_int atspre_g0int_half_int
#define atspre_g1int_add_int atspre_g0int_add_int
#define atspre_g1int_sub_int atspre_g0int_sub_int
#define atspre_g1int_mul_int atspre_g0int_mul_int
#define atspre_g1int_div_int atspre_g0int_div_int
#define atspre_g1int_nmod_int atspre_g0int_nmod_int
#define atspre_g1int_isltz_int atspre_g0int_isltz_int
#define atspre_g1int_isltez_int atspre_g0int_isltez_int
#define atspre_g1int_isgtz_int atspre_g0int_isgtz_int
#define atspre_g1int_isgtez_int atspre_g0int_isgtez_int
#define atspre_g1int_iseqz_int atspre_g0int_iseqz_int
#define atspre_g1int_isneqz_int atspre_g0int_isneqz_int
#define atspre_g1int_lt_int atspre_g0int_lt_int
#define atspre_g1int_lte_int atspre_g0int_lte_int
#define atspre_g1int_gt_int atspre_g0int_gt_int
#define atspre_g1int_gte_int atspre_g0int_gte_int
#define atspre_g1int_eq_int atspre_g0int_eq_int
#define atspre_g1int_neq_int atspre_g0int_neq_int
#define atspre_g1int_compare_int atspre_g0int_compare_int
#define atspre_g1int_max_int atspre_g0int_max_int
#define atspre_g1int_min_int atspre_g0int_min_int

/* ****** ****** */

#define atspre_g1int_neg_lint atspre_g0int_neg_lint
#define atspre_g1int_succ_lint atspre_g0int_succ_lint
#define atspre_g1int_pred_lint atspre_g0int_pred_lint
#define atspre_g1int_half_lint atspre_g0int_half_lint
#define atspre_g1int_add_lint atspre_g0int_add_lint
#define atspre_g1int_sub_lint atspre_g0int_sub_lint
#define atspre_g1int_mul_lint atspre_g0int_mul_lint
#define atspre_g1int_div_lint atspre_g0int_div_lint
#define atspre_g1int_nmod_lint atspre_g0int_nmod_lint
#define atspre_g1int_isltz_lint atspre_g0int_isltz_lint
#define atspre_g1int_isltez_lint atspre_g0int_isltez_lint
#define atspre_g1int_isgtz_lint atspre_g0int_isgtz_lint
#define atspre_g1int_isgtez_lint atspre_g0int_isgtez_lint
#define atspre_g1int_iseqz_lint atspre_g0int_iseqz_lint
#define atspre_g1int_isneqz_lint atspre_g0int_isneqz_lint
#define atspre_g1int_lt_lint atspre_g0int_lt_lint
#define atspre_g1int_lte_lint atspre_g0int_lte_lint
#define atspre_g1int_gt_lint atspre_g0int_gt_lint
#define atspre_g1int_gte_lint atspre_g0int_gte_lint
#define atspre_g1int_eq_lint atspre_g0int_eq_lint
#define atspre_g1int_neq_lint atspre_g0int_neq_lint
#define atspre_g1int_compare_lint atspre_g0int_compare_lint
#define atspre_g1int_max_lint atspre_g0int_max_lint
#define atspre_g1int_min_lint atspre_g0int_min_lint

/* ****** ****** */

#define atspre_g1int_neg_llint atspre_g0int_neg_llint
#define atspre_g1int_succ_llint atspre_g0int_succ_llint
#define atspre_g1int_pred_llint atspre_g0int_pred_llint
#define atspre_g1int_half_llint atspre_g0int_half_llint
#define atspre_g1int_add_llint atspre_g0int_add_llint
#define atspre_g1int_sub_llint atspre_g0int_sub_llint
#define atspre_g1int_mul_llint atspre_g0int_mul_llint
#define atspre_g1int_div_llint atspre_g0int_div_llint
#define atspre_g1int_nmod_llint atspre_g0int_nmod_llint
#define atspre_g1int_isltz_llint atspre_g0int_isltz_llint
#define atspre_g1int_isltez_llint atspre_g0int_isltez_llint
#define atspre_g1int_isgtz_llint atspre_g0int_isgtz_llint
#define atspre_g1int_isgtez_llint atspre_g0int_isgtez_llint
#define atspre_g1int_iseqz_llint atspre_g0int_iseqz_llint
#define atspre_g1int_isneqz_llint atspre_g0int_isneqz_llint
#define atspre_g1int_lt_llint atspre_g0int_lt_llint
#define atspre_g1int_lte_llint atspre_g0int_lte_llint
#define atspre_g1int_gt_llint atspre_g0int_gt_llint
#define atspre_g1int_gte_llint atspre_g0int_gte_llint
#define atspre_g1int_eq_llint atspre_g0int_eq_llint
#define atspre_g1int_neq_llint atspre_g0int_neq_llint
#define atspre_g1int_compare_llint atspre_g0int_compare_llint
#define atspre_g1int_max_llint atspre_g0int_max_llint
#define atspre_g1int_min_llint atspre_g0int_min_llint

/* ****** ****** */

#define atspre_g1int_neg_ssize atspre_g0int_neg_ssize
#define atspre_g1int_succ_ssize atspre_g0int_succ_ssize
#define atspre_g1int_pred_ssize atspre_g0int_pred_ssize
#define atspre_g1int_half_ssize atspre_g0int_half_ssize
#define atspre_g1int_add_ssize atspre_g0int_add_ssize
#define atspre_g1int_sub_ssize atspre_g0int_sub_ssize
#define atspre_g1int_mul_ssize atspre_g0int_mul_ssize
#define atspre_g1int_div_ssize atspre_g0int_div_ssize
#define atspre_g1int_nmod_ssize atspre_g0int_nmod_ssize
#define atspre_g1int_isltz_ssize atspre_g0int_isltz_ssize
#define atspre_g1int_isltez_ssize atspre_g0int_isltez_ssize
#define atspre_g1int_isgtz_ssize atspre_g0int_isgtz_ssize
#define atspre_g1int_isgtez_ssize atspre_g0int_isgtez_ssize
#define atspre_g1int_iseqz_ssize atspre_g0int_iseqz_ssize
#define atspre_g1int_isneqz_ssize atspre_g0int_isneqz_ssize
#define atspre_g1int_lt_ssize atspre_g0int_lt_ssize
#define atspre_g1int_lte_ssize atspre_g0int_lte_ssize
#define atspre_g1int_gt_ssize atspre_g0int_gt_ssize
#define atspre_g1int_gte_ssize atspre_g0int_gte_ssize
#define atspre_g1int_eq_ssize atspre_g0int_eq_ssize
#define atspre_g1int_neq_ssize atspre_g0int_neq_ssize
#define atspre_g1int_compare_ssize atspre_g0int_compare_ssize
#define atspre_g1int_max_ssize atspre_g0int_max_ssize
#define atspre_g1int_min_ssize atspre_g0int_min_ssize

/* ****** ****** */

ATSinline()
atstype_int
atspre_g0int_neg_int
  (atstype_int x) { return (-x) ; }
// end of [atspre_g0int_neg_int]
ATSinline()
atstype_int
atspre_g0int_abs_int
  (atstype_int x) { return (x >= 0 ? x : -x) ; }
// end of [atspre_g0int_abs_int]
ATSinline()
atstype_int
atspre_g0int_succ_int
  (atstype_int x) { return (x + 1) ; }
// end of [atspre_g0int_succ_int]
ATSinline()
atstype_int
atspre_g0int_pred_int
  (atstype_int x) { return (x - 1) ; }
// end of [atspre_g0int_pred_int]
ATSinline()
atstype_int
atspre_g0int_half_int
  (atstype_int x) { return (x / 2) ; }
// end of [atspre_g0int_half_int]
ATSinline()
atstype_int
atspre_g0int_add_int
  (atstype_int x1, atstype_int x2) { return (x1 + x2) ; }
// end of [atspre_g0int_add_int]
ATSinline()
atstype_int
atspre_g0int_sub_int
  (atstype_int x1, atstype_int x2) { return (x1 - x2) ; }
// end of [atspre_g0int_sub_int]
ATSinline()
atstype_int
atspre_g0int_mul_int
  (atstype_int x1, atstype_int x2) { return (x1 * x2) ; }
// end of [atspre_g0int_mul_int]
ATSinline()
atstype_int
atspre_g0int_div_int
  (atstype_int x1, atstype_int x2) { return (x1 / x2) ; }
// end of [atspre_g0int_div_int]
ATSinline()
atstype_int
atspre_g0int_mod_int
  (atstype_int x1, atstype_int x2) { return (x1 % x2) ; }
// end of [atspre_g0int_mod_int]
ATSinline()
atstype_int
atspre_g0int_nmod_int
  (atstype_int x1, atstype_int x2) { return (x1 % x2) ; }
// end of [atspre_g0int_nmod_int]
ATSinline()
atstype_int
atspre_g0int_asl_int
  (atstype_int x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0int_asl_int]
ATSinline()
atstype_int
atspre_g0int_asr_int
  (atstype_int x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0int_asr_int]
ATSinline()
atstype_bool
atspre_g0int_isltz_int (atstype_int x)
{
  return (x < 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltz_int]
ATSinline()
atstype_bool
atspre_g0int_isltez_int (atstype_int x)
{
  return (x <= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltez_int]
ATSinline()
atstype_bool
atspre_g0int_isgtz_int (atstype_int x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtz_int]
ATSinline()
atstype_bool
atspre_g0int_isgtez_int (atstype_int x)
{
  return (x >= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtez_int]
ATSinline()
atstype_bool
atspre_g0int_iseqz_int (atstype_int x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_iseqz_int]
ATSinline()
atstype_bool
atspre_g0int_isneqz_int (atstype_int x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isneqz_int]
ATSinline()
atstype_bool
atspre_g0int_lt_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lt_int]
ATSinline()
atstype_bool
atspre_g0int_lte_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lte_int]
ATSinline()
atstype_bool
atspre_g0int_gt_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gt_int]
ATSinline()
atstype_bool
atspre_g0int_gte_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gte_int]
ATSinline()
atstype_bool
atspre_g0int_eq_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_eq_int]
ATSinline()
atstype_bool
atspre_g0int_neq_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_neq_int]
ATSinline()
atstype_int
atspre_g0int_compare_int
(
  atstype_int x1, atstype_int x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0int_compare_int]
ATSinline()
atstype_int
atspre_g0int_max_int
  (atstype_int x1, atstype_int x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0int_max_int]
ATSinline()
atstype_int
atspre_g0int_min_int
  (atstype_int x1, atstype_int x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0int_min_int]
//
ATSinline()
atstype_lint
atspre_g0int_neg_lint
  (atstype_lint x) { return (-x) ; }
// end of [atspre_g0int_neg_lint]
ATSinline()
atstype_lint
atspre_g0int_abs_lint
  (atstype_lint x) { return (x >= 0 ? x : -x) ; }
// end of [atspre_g0int_abs_lint]
ATSinline()
atstype_lint
atspre_g0int_succ_lint
  (atstype_lint x) { return (x + 1) ; }
// end of [atspre_g0int_succ_lint]
ATSinline()
atstype_lint
atspre_g0int_pred_lint
  (atstype_lint x) { return (x - 1) ; }
// end of [atspre_g0int_pred_lint]
ATSinline()
atstype_lint
atspre_g0int_half_lint
  (atstype_lint x) { return (x / 2) ; }
// end of [atspre_g0int_half_lint]
ATSinline()
atstype_lint
atspre_g0int_add_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 + x2) ; }
// end of [atspre_g0int_add_lint]
ATSinline()
atstype_lint
atspre_g0int_sub_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 - x2) ; }
// end of [atspre_g0int_sub_lint]
ATSinline()
atstype_lint
atspre_g0int_mul_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 * x2) ; }
// end of [atspre_g0int_mul_lint]
ATSinline()
atstype_lint
atspre_g0int_div_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 / x2) ; }
// end of [atspre_g0int_div_lint]
ATSinline()
atstype_lint
atspre_g0int_mod_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 % x2) ; }
// end of [atspre_g0int_mod_lint]
ATSinline()
atstype_lint
atspre_g0int_nmod_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 % x2) ; }
// end of [atspre_g0int_nmod_lint]
ATSinline()
atstype_lint
atspre_g0int_asl_lint
  (atstype_lint x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0int_asl_lint]
ATSinline()
atstype_lint
atspre_g0int_asr_lint
  (atstype_lint x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0int_asr_lint]
ATSinline()
atstype_bool
atspre_g0int_isltz_lint (atstype_lint x)
{
  return (x < 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltz_lint]
ATSinline()
atstype_bool
atspre_g0int_isltez_lint (atstype_lint x)
{
  return (x <= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltez_lint]
ATSinline()
atstype_bool
atspre_g0int_isgtz_lint (atstype_lint x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtz_lint]
ATSinline()
atstype_bool
atspre_g0int_isgtez_lint (atstype_lint x)
{
  return (x >= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtez_lint]
ATSinline()
atstype_bool
atspre_g0int_iseqz_lint (atstype_lint x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_iseqz_lint]
ATSinline()
atstype_bool
atspre_g0int_isneqz_lint (atstype_lint x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isneqz_lint]
ATSinline()
atstype_bool
atspre_g0int_lt_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lt_lint]
ATSinline()
atstype_bool
atspre_g0int_lte_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lte_lint]
ATSinline()
atstype_bool
atspre_g0int_gt_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gt_lint]
ATSinline()
atstype_bool
atspre_g0int_gte_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gte_lint]
ATSinline()
atstype_bool
atspre_g0int_eq_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_eq_lint]
ATSinline()
atstype_bool
atspre_g0int_neq_lint
(
  atstype_lint x1, atstype_lint x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_neq_lint]
ATSinline()
atstype_int
atspre_g0int_compare_lint
(
  atstype_lint x1, atstype_lint x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0int_compare_lint]
ATSinline()
atstype_lint
atspre_g0int_max_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0int_max_lint]
ATSinline()
atstype_lint
atspre_g0int_min_lint
  (atstype_lint x1, atstype_lint x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0int_min_lint]
//
ATSinline()
atstype_llint
atspre_g0int_neg_llint
  (atstype_llint x) { return (-x) ; }
// end of [atspre_g0int_neg_llint]
ATSinline()
atstype_llint
atspre_g0int_abs_llint
  (atstype_llint x) { return (x >= 0 ? x : -x) ; }
// end of [atspre_g0int_abs_llint]
ATSinline()
atstype_llint
atspre_g0int_succ_llint
  (atstype_llint x) { return (x + 1) ; }
// end of [atspre_g0int_succ_llint]
ATSinline()
atstype_llint
atspre_g0int_pred_llint
  (atstype_llint x) { return (x - 1) ; }
// end of [atspre_g0int_pred_llint]
ATSinline()
atstype_llint
atspre_g0int_half_llint
  (atstype_llint x) { return (x / 2) ; }
// end of [atspre_g0int_half_llint]
ATSinline()
atstype_llint
atspre_g0int_add_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 + x2) ; }
// end of [atspre_g0int_add_llint]
ATSinline()
atstype_llint
atspre_g0int_sub_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 - x2) ; }
// end of [atspre_g0int_sub_llint]
ATSinline()
atstype_llint
atspre_g0int_mul_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 * x2) ; }
// end of [atspre_g0int_mul_llint]
ATSinline()
atstype_llint
atspre_g0int_div_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 / x2) ; }
// end of [atspre_g0int_div_llint]
ATSinline()
atstype_llint
atspre_g0int_mod_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 % x2) ; }
// end of [atspre_g0int_mod_llint]
ATSinline()
atstype_llint
atspre_g0int_nmod_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 % x2) ; }
// end of [atspre_g0int_nmod_llint]
ATSinline()
atstype_llint
atspre_g0int_asl_llint
  (atstype_llint x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0int_asl_llint]
ATSinline()
atstype_llint
atspre_g0int_asr_llint
  (atstype_llint x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0int_asr_llint]
ATSinline()
atstype_bool
atspre_g0int_isltz_llint (atstype_llint x)
{
  return (x < 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltz_llint]
ATSinline()
atstype_bool
atspre_g0int_isltez_llint (atstype_llint x)
{
  return (x <= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltez_llint]
ATSinline()
atstype_bool
atspre_g0int_isgtz_llint (atstype_llint x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtz_llint]
ATSinline()
atstype_bool
atspre_g0int_isgtez_llint (atstype_llint x)
{
  return (x >= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtez_llint]
ATSinline()
atstype_bool
atspre_g0int_iseqz_llint (atstype_llint x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_iseqz_llint]
ATSinline()
atstype_bool
atspre_g0int_isneqz_llint (atstype_llint x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isneqz_llint]
ATSinline()
atstype_bool
atspre_g0int_lt_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lt_llint]
ATSinline()
atstype_bool
atspre_g0int_lte_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lte_llint]
ATSinline()
atstype_bool
atspre_g0int_gt_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gt_llint]
ATSinline()
atstype_bool
atspre_g0int_gte_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gte_llint]
ATSinline()
atstype_bool
atspre_g0int_eq_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_eq_llint]
ATSinline()
atstype_bool
atspre_g0int_neq_llint
(
  atstype_llint x1, atstype_llint x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_neq_llint]
ATSinline()
atstype_int
atspre_g0int_compare_llint
(
  atstype_llint x1, atstype_llint x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0int_compare_llint]
ATSinline()
atstype_llint
atspre_g0int_max_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0int_max_llint]
ATSinline()
atstype_llint
atspre_g0int_min_llint
  (atstype_llint x1, atstype_llint x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0int_min_llint]
//
ATSinline()
atstype_ssize
atspre_g0int_neg_ssize
  (atstype_ssize x) { return (-x) ; }
// end of [atspre_g0int_neg_ssize]
ATSinline()
atstype_ssize
atspre_g0int_abs_ssize
  (atstype_ssize x) { return (x >= 0 ? x : -x) ; }
// end of [atspre_g0int_abs_ssize]
ATSinline()
atstype_ssize
atspre_g0int_succ_ssize
  (atstype_ssize x) { return (x + 1) ; }
// end of [atspre_g0int_succ_ssize]
ATSinline()
atstype_ssize
atspre_g0int_pred_ssize
  (atstype_ssize x) { return (x - 1) ; }
// end of [atspre_g0int_pred_ssize]
ATSinline()
atstype_ssize
atspre_g0int_half_ssize
  (atstype_ssize x) { return (x / 2) ; }
// end of [atspre_g0int_half_ssize]
ATSinline()
atstype_ssize
atspre_g0int_add_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 + x2) ; }
// end of [atspre_g0int_add_ssize]
ATSinline()
atstype_ssize
atspre_g0int_sub_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 - x2) ; }
// end of [atspre_g0int_sub_ssize]
ATSinline()
atstype_ssize
atspre_g0int_mul_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 * x2) ; }
// end of [atspre_g0int_mul_ssize]
ATSinline()
atstype_ssize
atspre_g0int_div_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 / x2) ; }
// end of [atspre_g0int_div_ssize]
ATSinline()
atstype_ssize
atspre_g0int_mod_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 % x2) ; }
// end of [atspre_g0int_mod_ssize]
ATSinline()
atstype_ssize
atspre_g0int_nmod_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 % x2) ; }
// end of [atspre_g0int_nmod_ssize]
ATSinline()
atstype_ssize
atspre_g0int_asl_ssize
  (atstype_ssize x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0int_asl_ssize]
ATSinline()
atstype_ssize
atspre_g0int_asr_ssize
  (atstype_ssize x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0int_asr_ssize]
ATSinline()
atstype_bool
atspre_g0int_isltz_ssize (atstype_ssize x)
{
  return (x < 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltz_ssize]
ATSinline()
atstype_bool
atspre_g0int_isltez_ssize (atstype_ssize x)
{
  return (x <= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isltez_ssize]
ATSinline()
atstype_bool
atspre_g0int_isgtz_ssize (atstype_ssize x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtz_ssize]
ATSinline()
atstype_bool
atspre_g0int_isgtez_ssize (atstype_ssize x)
{
  return (x >= 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isgtez_ssize]
ATSinline()
atstype_bool
atspre_g0int_iseqz_ssize (atstype_ssize x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_iseqz_ssize]
ATSinline()
atstype_bool
atspre_g0int_isneqz_ssize (atstype_ssize x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_isneqz_ssize]
ATSinline()
atstype_bool
atspre_g0int_lt_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lt_ssize]
ATSinline()
atstype_bool
atspre_g0int_lte_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_lte_ssize]
ATSinline()
atstype_bool
atspre_g0int_gt_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gt_ssize]
ATSinline()
atstype_bool
atspre_g0int_gte_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_gte_ssize]
ATSinline()
atstype_bool
atspre_g0int_eq_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_eq_ssize]
ATSinline()
atstype_bool
atspre_g0int_neq_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_neq_ssize]
ATSinline()
atstype_int
atspre_g0int_compare_ssize
(
  atstype_ssize x1, atstype_ssize x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0int_compare_ssize]
ATSinline()
atstype_ssize
atspre_g0int_max_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0int_max_ssize]
ATSinline()
atstype_ssize
atspre_g0int_min_ssize
  (atstype_ssize x1, atstype_ssize x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0int_min_ssize]
//
/* ****** ****** */


#endif // ifndef ATSLIB_PRELUDE_CATS_INTEGER

/* ****** ****** */

/* end of [integer.cats] */
