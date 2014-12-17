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

ATSinline()
atstype_bool
atspre_g0int_eq_int
(
  atstype_int x1, atstype_int x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0int_eq_int]

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

#endif // ifndef ATSLIB_PRELUDE_CATS_INTEGER

/* ****** ****** */

/* end of [integer.cats] */
