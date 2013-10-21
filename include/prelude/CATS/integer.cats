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
** $PATSHOME/prelude/CATS/CODEGEN/integer.atxt
** Time of generation: Mon Oct 14 16:14:09 2013
*/

/* ****** ****** */

/*
(* Author: Hongwei Xi *)
(* Authoremail: hwxi AT cs DOT bu DOT edu *)
(* Start time: January, 2013 *)
*/

/* ****** ****** */

#ifndef ATSLIB_PRELUDE_INTEGER_CATS
#define ATSLIB_PRELUDE_INTEGER_CATS

//
/* ****** ****** */
//
#define atspre_g0int2int_int_int(x) (x)
#define atspre_g1int2int_int_int atspre_g0int2int_int_int
#define atspre_g0int2int_int_lint(x) ((atstype_lint)(x))
#define atspre_g0int2int_int_llint(x) ((atstype_llint)(x))
#define atspre_g0int2int_int_ssize(x) ((atstype_ssize)(x))
#define atspre_g1int2int_int_lint atspre_g0int2int_int_lint
#define atspre_g1int2int_int_llint atspre_g0int2int_int_llint
#define atspre_g1int2int_int_ssize atspre_g0int2int_int_ssize
//
/* ****** ****** */
//
#define atspre_g0int2uint_int_uint(x) ((atstype_uint)(x))
#define atspre_g0int2uint_int_ulint(x) ((atstype_ulint)(x))
#define atspre_g0int2uint_int_ullint(x) ((atstype_ullint)(x))
#define atspre_g0int2uint_int_size(x) ((atstype_size)(x))
#define atspre_g1int2uint_int_uint atspre_g0int2uint_int_uint
#define atspre_g1int2uint_int_ulint atspre_g0int2uint_int_ulint
#define atspre_g1int2uint_int_ullint atspre_g0int2uint_int_ullint
#define atspre_g1int2uint_int_size atspre_g0int2uint_int_size
//
#define atspre_g0int2uint_lint_ulint(x) ((atstype_ulint)(x))
#define atspre_g0int2uint_lint_ullint(x) ((atstype_ullint)(x))
#define atspre_g0int2uint_lint_size(x) ((atstype_size)(x))
#define atspre_g1int2uint_lint_ulint atspre_g0int2uint_lint_ulint
#define atspre_g1int2uint_lint_ullint atspre_g0int2uint_lint_ullint
#define atspre_g1int2uint_lint_size atspre_g0int2uint_lint_size
//
#define atspre_g0int2uint_llint_ullint(x) ((atstype_ullint)(x))
#define atspre_g1int2uint_llint_ullint atspre_g0int2uint_llint_ullint
//
/* ****** ****** */
//
#define atspre_g0uint2int_uint_int(x) ((atstype_int)(x))
#define atspre_g0uint2int_uint_lint(x) ((atstype_lint)(x))
#define atspre_g0uint2int_uint_llint(x) ((atstype_llint)(x))
#define atspre_g0uint2int_uint_ssize(x) ((atstype_ssize)(x))
#define atspre_g1uint2int_uint_int atspre_g0uint2int_uint_int
#define atspre_g1uint2int_uint_lint atspre_g0uint2int_uint_lint
#define atspre_g1uint2int_uint_llint atspre_g0uint2int_uint_llint
#define atspre_g1uint2int_uint_ssize atspre_g0uint2int_uint_ssize
//
/* ****** ****** */
//
#define atspre_g0uint2int_size_int(x) ((atstype_int)(x))
#define atspre_g0uint2int_size_lint(x) ((atstype_lint)(x))
#define atspre_g0uint2int_size_llint(x) ((atstype_llint)(x))
#define atspre_g0uint2int_size_ssize(x) ((atstype_ssize)(x))
#define atspre_g1uint2int_size_int atspre_g0uint2int_size_int
#define atspre_g1uint2int_size_lint atspre_g0uint2int_size_lint
#define atspre_g1uint2int_size_llint atspre_g0uint2int_size_llint
#define atspre_g1uint2int_size_ssize atspre_g0uint2int_size_ssize
//
/* ****** ****** */
//
#define atspre_g0uint2uint_uint_uint(x) (x)
#define atspre_g1uint2uint_uint_uint atspre_g0uint2uint_uint_uint
#define atspre_g0uint2uint_uint_ulint(x) ((atstype_ulint)(x))
#define atspre_g1uint2uint_uint_ulint atspre_g0uint2uint_uint_ulint
#define atspre_g0uint2uint_uint_ullint(x) ((atstype_ullint)(x))
#define atspre_g1uint2uint_uint_ullint atspre_g0uint2uint_uint_ullint
#define atspre_g0uint2uint_uint_size(x) ((atstype_size)(x))
#define atspre_g1uint2uint_uint_size atspre_g0uint2uint_uint_size
//
/* ****** ****** */

#define atspre_g0uint2uint_ulint_uint(x) ((atstype_uint)(x))
#define atspre_g1uint2uint_ulint_uint atspre_g0uint2uint_ulint_uint
#define atspre_g0uint2uint_ulint_ulint(x) (x)
#define atspre_g1uint2uint_ulint_ulint atspre_g0uint2uint_ulint_ulint
#define atspre_g0uint2uint_ulint_ullint(x) ((atstype_ullint)(x))
#define atspre_g1uint2uint_ulint_ullint atspre_g0uint2uint_ulint_ullint
#define atspre_g0uint2uint_ulint_size(x) ((atstype_size)(x))
#define atspre_g1uint2uint_ulint_size atspre_g0uint2uint_ulint_size

/* ****** ****** */
//
#define atspre_g0uint2uint_size_uint(x) ((atstype_uint)(x))
#define atspre_g1uint2uint_size_uint atspre_g0uint2uint_size_uint
#define atspre_g0uint2uint_size_ulint(x) ((atstype_ulint)(x))
#define atspre_g1uint2uint_size_ulint atspre_g0uint2uint_size_ulint
#define atspre_g0uint2uint_size_ullint(x) ((atstype_ullint)(x))
#define atspre_g1uint2uint_size_ullint atspre_g0uint2uint_size_ullint
#define atspre_g0uint2uint_size_size(x) (x)
#define atspre_g1uint2uint_size_size atspre_g0uint2uint_size_size
//
/* ****** ****** */

#if 0
//
ATSinline()
atstype_string
atspre_g0int2string_int
  (atstype_int x)
{
  size_t n0 ;
  char *res ;
  size_t ntot ;
  n0 = 4 ;
  res = ATS_MALLOC(n0) ;
  ntot = snprintf(res, n0, "%i", x) ;
  if (ntot >= n0)
  {
    ATS_MFREE(res) ;
    res = (char*)ATS_MALLOC(ntot+1) ;
    ntot = snprintf(res, ntot+1, "%i", x) ;
  }
  return res ;
}
//
ATSinline()
atstype_string
atspre_g0int2string_lint
  (atstype_lint x)
{
  size_t n0 ;
  char *res ;
  size_t ntot ;
  n0 = 4 ;
  res = ATS_MALLOC(n0) ;
  ntot = snprintf(res, n0, "%li", x) ;
  if (ntot >= n0)
  {
    ATS_MFREE(res) ;
    res = (char*)ATS_MALLOC(ntot+1) ;
    ntot = snprintf(res, ntot+1, "%li", x) ;
  }
  return res ;
}
//
ATSinline()
atstype_string
atspre_g0int2string_llint
  (atstype_llint x)
{
  size_t n0 ;
  char *res ;
  size_t ntot ;
  n0 = 8 ;
  res = ATS_MALLOC(n0) ;
  ntot = snprintf(res, n0, "%lli", x) ;
  if (ntot >= n0)
  {
    ATS_MFREE(res) ;
    res = (char*)ATS_MALLOC(ntot+1) ;
    ntot = snprintf(res, ntot+1, "%lli", x) ;
  }
  return res ;
}
//
/* ****** ****** */
//
extern int atoi (const char *inp) ;
extern long int atol (const char *inp) ;
extern long long int atoll (const char *inp) ;
//
ATSinline()
atstype_int
atspre_g0string2int_int
  (atstype_string inp) { return atoi((char*)inp) ; }
ATSinline()
atstype_lint
atspre_g0string2int_lint
  (atstype_string inp) { return atol((char*)inp) ; }
ATSinline()
atstype_llint
atspre_g0string2int_llint
  (atstype_string inp) { return atoll((char*)inp) ; }
ATSinline()
atstype_ssize
atspre_g0string2int_ssize
  (atstype_string inp) { return atol((char*)inp) ; }
//
/* ****** ****** */
//
extern
unsigned long int
strtoul(const char *nptr, char **endptr, int base);
extern
unsigned long long int
strtoull(const char *nptr, char **endptr, int base);
//
ATSinline()
atstype_uint
atspre_g0string2uint_uint
  (atstype_string inp) { return strtoul((char*)inp, NULL, 10) ; }
ATSinline()
atstype_ulint
atspre_g0string2uint_ulint
  (atstype_string inp) { return strtoul((char*)inp, NULL, 10) ; }
ATSinline()
atstype_ullint
atspre_g0string2uint_ullint
  (atstype_string inp) { return strtoull((char*)inp, NULL, 10) ; }
ATSinline()
atstype_size
atspre_g0string2uint_size
  (atstype_string inp) { return strtoul((char*)inp, NULL, 10) ; }
//
/* ****** ****** */
#endif




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


#if 0
ATSinline()
atsvoid_t0ype
atspre_fprint_int
(
  atstype_ref out, atstype_int x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%i", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_int] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_int]
#define atspre_print_int(x) atspre_fprint_int(stdout, (x))
#define atspre_prerr_int(x) atspre_fprint_int(stderr, (x))

ATSinline()
atsvoid_t0ype
atspre_fprint_lint
(
  atstype_ref out, atstype_lint x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%li", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_lint] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_lint]
#define atspre_print_lint(x) atspre_fprint_lint(stdout, (x))
#define atspre_prerr_lint(x) atspre_fprint_lint(stderr, (x))

ATSinline()
atsvoid_t0ype
atspre_fprint_llint
(
  atstype_ref out, atstype_llint x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%lli", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_llint] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_llint]
#define atspre_print_llint(x) atspre_fprint_llint(stdout, (x))
#define atspre_prerr_llint(x) atspre_fprint_llint(stderr, (x))

ATSinline()
atsvoid_t0ype
atspre_fprint_ssize
(
  atstype_ref out, atstype_ssize x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%li", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_ssize] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_ssize]
#define atspre_print_ssize(x) atspre_fprint_ssize(stdout, (x))
#define atspre_prerr_ssize(x) atspre_fprint_ssize(stderr, (x))
#endif

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
#define atspre_g1int_mod_int atspre_g0int_mod_int
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
#define atspre_g1int_mod_lint atspre_g0int_mod_lint
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
#define atspre_g1int_mod_llint atspre_g0int_mod_llint
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
#define atspre_g1int_mod_ssize atspre_g0int_mod_ssize
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
atstype_uint
atspre_g0uint_succ_uint
  (atstype_uint x) { return (x + 1) ; }
// end of [atspre_g0uint_succ_uint]
ATSinline()
atstype_uint
atspre_g0uint_pred_uint
  (atstype_uint x) { return (x - 1) ; }
// end of [atspre_g0uint_pred_uint]
ATSinline()
atstype_uint
atspre_g0uint_half_uint
  (atstype_uint x) { return (x >> 1) ; }
// end of [atspre_g0uint_half_uint]
ATSinline()
atstype_uint
atspre_g0uint_add_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 + x2) ; }
// end of [atspre_g0uint_add_uint]
ATSinline()
atstype_uint
atspre_g0uint_sub_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 - x2) ; }
// end of [atspre_g0uint_sub_uint]
ATSinline()
atstype_uint
atspre_g0uint_mul_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 * x2) ; }
// end of [atspre_g0uint_mul_uint]
ATSinline()
atstype_uint
atspre_g0uint_div_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 / x2) ; }
// end of [atspre_g0uint_div_uint]
ATSinline()
atstype_uint
atspre_g0uint_mod_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 % x2) ; }
// end of [atspre_g0uint_mod_uint]
ATSinline()
atstype_uint
atspre_g0uint_lsl_uint
  (atstype_uint x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0uint_lsl_uint]
ATSinline()
atstype_uint
atspre_g0uint_lsr_uint
  (atstype_uint x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0uint_lsr_uint]
ATSinline()
atstype_uint
atspre_g0uint_lnot_uint
  (atstype_uint x) { return ~(x) ; }
// end of [atspre_g0uint_lnot_uint]
ATSinline()
atstype_uint
atspre_g0uint_lor_uint
  (atstype_uint x, atstype_uint y) { return (x | y) ; }
// end of [atspre_g0uint_uint_uint]
ATSinline()
atstype_uint
atspre_g0uint_land_uint
  (atstype_uint x, atstype_uint y) { return (x & y) ; }
// end of [atspre_g0uint_uint_uint]
ATSinline()
atstype_uint
atspre_g0uint_lxor_uint
  (atstype_uint x, atstype_uint y) { return (x ^ y) ; }
// end of [atspre_g0uint_uint_uint]
ATSinline()
atstype_bool
atspre_g0uint_isgtz_uint (atstype_uint x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isgtz_uint]
ATSinline()
atstype_bool
atspre_g0uint_iseqz_uint (atstype_uint x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_iseqz_uint]
ATSinline()
atstype_bool
atspre_g0uint_isneqz_uint (atstype_uint x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isneqz_uint]
ATSinline()
atstype_bool
atspre_g0uint_lt_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lt_uint]
ATSinline()
atstype_bool
atspre_g0uint_lte_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lte_uint]
ATSinline()
atstype_bool
atspre_g0uint_gt_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gt_uint]
ATSinline()
atstype_bool
atspre_g0uint_gte_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gte_uint]
ATSinline()
atstype_bool
atspre_g0uint_eq_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_eq_uint]
ATSinline()
atstype_bool
atspre_g0uint_neq_uint
(
  atstype_uint x1, atstype_uint x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_neq_uint]
ATSinline()
atstype_int
atspre_g0uint_compare_uint
(
  atstype_uint x1, atstype_uint x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0uint_compare_uint]
ATSinline()
atstype_uint
atspre_g0uint_max_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_max_uint]
ATSinline()
atstype_uint
atspre_g0uint_min_uint
  (atstype_uint x1, atstype_uint x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_min_uint]
//
ATSinline()
atstype_ulint
atspre_g0uint_succ_ulint
  (atstype_ulint x) { return (x + 1) ; }
// end of [atspre_g0uint_succ_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_pred_ulint
  (atstype_ulint x) { return (x - 1) ; }
// end of [atspre_g0uint_pred_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_half_ulint
  (atstype_ulint x) { return (x >> 1) ; }
// end of [atspre_g0uint_half_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_add_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 + x2) ; }
// end of [atspre_g0uint_add_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_sub_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 - x2) ; }
// end of [atspre_g0uint_sub_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_mul_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 * x2) ; }
// end of [atspre_g0uint_mul_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_div_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 / x2) ; }
// end of [atspre_g0uint_div_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_mod_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 % x2) ; }
// end of [atspre_g0uint_mod_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_lsl_ulint
  (atstype_ulint x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0uint_lsl_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_lsr_ulint
  (atstype_ulint x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0uint_lsr_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_lnot_ulint
  (atstype_ulint x) { return ~(x) ; }
// end of [atspre_g0uint_lnot_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_lor_ulint
  (atstype_ulint x, atstype_ulint y) { return (x | y) ; }
// end of [atspre_g0uint_ulint_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_land_ulint
  (atstype_ulint x, atstype_ulint y) { return (x & y) ; }
// end of [atspre_g0uint_ulint_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_lxor_ulint
  (atstype_ulint x, atstype_ulint y) { return (x ^ y) ; }
// end of [atspre_g0uint_ulint_ulint]
ATSinline()
atstype_bool
atspre_g0uint_isgtz_ulint (atstype_ulint x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isgtz_ulint]
ATSinline()
atstype_bool
atspre_g0uint_iseqz_ulint (atstype_ulint x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_iseqz_ulint]
ATSinline()
atstype_bool
atspre_g0uint_isneqz_ulint (atstype_ulint x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isneqz_ulint]
ATSinline()
atstype_bool
atspre_g0uint_lt_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lt_ulint]
ATSinline()
atstype_bool
atspre_g0uint_lte_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lte_ulint]
ATSinline()
atstype_bool
atspre_g0uint_gt_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gt_ulint]
ATSinline()
atstype_bool
atspre_g0uint_gte_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gte_ulint]
ATSinline()
atstype_bool
atspre_g0uint_eq_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_eq_ulint]
ATSinline()
atstype_bool
atspre_g0uint_neq_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_neq_ulint]
ATSinline()
atstype_int
atspre_g0uint_compare_ulint
(
  atstype_ulint x1, atstype_ulint x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0uint_compare_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_max_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_max_ulint]
ATSinline()
atstype_ulint
atspre_g0uint_min_ulint
  (atstype_ulint x1, atstype_ulint x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_min_ulint]
//
ATSinline()
atstype_ullint
atspre_g0uint_succ_ullint
  (atstype_ullint x) { return (x + 1) ; }
// end of [atspre_g0uint_succ_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_pred_ullint
  (atstype_ullint x) { return (x - 1) ; }
// end of [atspre_g0uint_pred_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_half_ullint
  (atstype_ullint x) { return (x >> 1) ; }
// end of [atspre_g0uint_half_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_add_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 + x2) ; }
// end of [atspre_g0uint_add_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_sub_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 - x2) ; }
// end of [atspre_g0uint_sub_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_mul_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 * x2) ; }
// end of [atspre_g0uint_mul_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_div_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 / x2) ; }
// end of [atspre_g0uint_div_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_mod_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 % x2) ; }
// end of [atspre_g0uint_mod_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_lsl_ullint
  (atstype_ullint x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0uint_lsl_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_lsr_ullint
  (atstype_ullint x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0uint_lsr_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_lnot_ullint
  (atstype_ullint x) { return ~(x) ; }
// end of [atspre_g0uint_lnot_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_lor_ullint
  (atstype_ullint x, atstype_ullint y) { return (x | y) ; }
// end of [atspre_g0uint_ullint_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_land_ullint
  (atstype_ullint x, atstype_ullint y) { return (x & y) ; }
// end of [atspre_g0uint_ullint_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_lxor_ullint
  (atstype_ullint x, atstype_ullint y) { return (x ^ y) ; }
// end of [atspre_g0uint_ullint_ullint]
ATSinline()
atstype_bool
atspre_g0uint_isgtz_ullint (atstype_ullint x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isgtz_ullint]
ATSinline()
atstype_bool
atspre_g0uint_iseqz_ullint (atstype_ullint x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_iseqz_ullint]
ATSinline()
atstype_bool
atspre_g0uint_isneqz_ullint (atstype_ullint x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isneqz_ullint]
ATSinline()
atstype_bool
atspre_g0uint_lt_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lt_ullint]
ATSinline()
atstype_bool
atspre_g0uint_lte_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lte_ullint]
ATSinline()
atstype_bool
atspre_g0uint_gt_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gt_ullint]
ATSinline()
atstype_bool
atspre_g0uint_gte_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gte_ullint]
ATSinline()
atstype_bool
atspre_g0uint_eq_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_eq_ullint]
ATSinline()
atstype_bool
atspre_g0uint_neq_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_neq_ullint]
ATSinline()
atstype_int
atspre_g0uint_compare_ullint
(
  atstype_ullint x1, atstype_ullint x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0uint_compare_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_max_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_max_ullint]
ATSinline()
atstype_ullint
atspre_g0uint_min_ullint
  (atstype_ullint x1, atstype_ullint x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_min_ullint]
//
ATSinline()
atstype_size
atspre_g0uint_succ_size
  (atstype_size x) { return (x + 1) ; }
// end of [atspre_g0uint_succ_size]
ATSinline()
atstype_size
atspre_g0uint_pred_size
  (atstype_size x) { return (x - 1) ; }
// end of [atspre_g0uint_pred_size]
ATSinline()
atstype_size
atspre_g0uint_half_size
  (atstype_size x) { return (x >> 1) ; }
// end of [atspre_g0uint_half_size]
ATSinline()
atstype_size
atspre_g0uint_add_size
  (atstype_size x1, atstype_size x2) { return (x1 + x2) ; }
// end of [atspre_g0uint_add_size]
ATSinline()
atstype_size
atspre_g0uint_sub_size
  (atstype_size x1, atstype_size x2) { return (x1 - x2) ; }
// end of [atspre_g0uint_sub_size]
ATSinline()
atstype_size
atspre_g0uint_mul_size
  (atstype_size x1, atstype_size x2) { return (x1 * x2) ; }
// end of [atspre_g0uint_mul_size]
ATSinline()
atstype_size
atspre_g0uint_div_size
  (atstype_size x1, atstype_size x2) { return (x1 / x2) ; }
// end of [atspre_g0uint_div_size]
ATSinline()
atstype_size
atspre_g0uint_mod_size
  (atstype_size x1, atstype_size x2) { return (x1 % x2) ; }
// end of [atspre_g0uint_mod_size]
ATSinline()
atstype_size
atspre_g0uint_lsl_size
  (atstype_size x, atstype_int n) { return (x << n) ; }
// end of [atspre_g0uint_lsl_size]
ATSinline()
atstype_size
atspre_g0uint_lsr_size
  (atstype_size x, atstype_int n) { return (x >> n) ; }
// end of [atspre_g0uint_lsr_size]
ATSinline()
atstype_size
atspre_g0uint_lnot_size
  (atstype_size x) { return ~(x) ; }
// end of [atspre_g0uint_lnot_size]
ATSinline()
atstype_size
atspre_g0uint_lor_size
  (atstype_size x, atstype_size y) { return (x | y) ; }
// end of [atspre_g0uint_size_size]
ATSinline()
atstype_size
atspre_g0uint_land_size
  (atstype_size x, atstype_size y) { return (x & y) ; }
// end of [atspre_g0uint_size_size]
ATSinline()
atstype_size
atspre_g0uint_lxor_size
  (atstype_size x, atstype_size y) { return (x ^ y) ; }
// end of [atspre_g0uint_size_size]
ATSinline()
atstype_bool
atspre_g0uint_isgtz_size (atstype_size x)
{
  return (x > 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isgtz_size]
ATSinline()
atstype_bool
atspre_g0uint_iseqz_size (atstype_size x)
{
  return (x == 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_iseqz_size]
ATSinline()
atstype_bool
atspre_g0uint_isneqz_size (atstype_size x)
{
  return (x != 0 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_isneqz_size]
ATSinline()
atstype_bool
atspre_g0uint_lt_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 < x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lt_size]
ATSinline()
atstype_bool
atspre_g0uint_lte_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 <= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_lte_size]
ATSinline()
atstype_bool
atspre_g0uint_gt_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 > x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gt_size]
ATSinline()
atstype_bool
atspre_g0uint_gte_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 >= x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_gte_size]
ATSinline()
atstype_bool
atspre_g0uint_eq_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 == x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_eq_size]
ATSinline()
atstype_bool
atspre_g0uint_neq_size
(
  atstype_size x1, atstype_size x2
) {
  return (x1 != x2 ? atsbool_true : atsbool_false) ;
} // end of [atspre_g0uint_neq_size]
ATSinline()
atstype_int
atspre_g0uint_compare_size
(
  atstype_size x1, atstype_size x2
) {
  if (x1 < x2) return -1 ; else if (x1 > x2) return 1 ; else return 0 ;
} // end of [atspre_g0uint_compare_size]
ATSinline()
atstype_size
atspre_g0uint_max_size
  (atstype_size x1, atstype_size x2) { return (x1 >= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_max_size]
ATSinline()
atstype_size
atspre_g0uint_min_size
  (atstype_size x1, atstype_size x2) { return (x1 <= x2 ? x1 : x2) ; }
// end of [atspre_g0uint_min_size]
//
/* ****** ****** */

#if 0

ATSinline()
atsvoid_t0ype
atspre_fprint_uint
(
  atstype_ref out, atstype_uint x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%u", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_uint] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_uint]

#define atspre_print_uint(x) atspre_fprint_uint(stdout, (x))
#define atspre_prerr_uint(x) atspre_fprint_uint(stderr, (x))

ATSinline()
atsvoid_t0ype
atspre_fprint_ulint
(
  atstype_ref out, atstype_ulint x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%lu", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_ulint] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_ulint]

#define atspre_print_ulint(x) atspre_fprint_ulint(stdout, (x))
#define atspre_prerr_ulint(x) atspre_fprint_ulint(stderr, (x))

ATSinline()
atsvoid_t0ype
atspre_fprint_ullint
(
  atstype_ref out, atstype_ullint x
) {
  int err = 0 ;
  err += fprintf((FILE*)out, "%llu", x) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_ullint] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_ullint]

#define atspre_print_ullint(x) atspre_fprint_ullint(stdout, (x))
#define atspre_prerr_ullint(x) atspre_fprint_ullint(stderr, (x))

/* ****** ****** */

ATSinline()
atsvoid_t0ype
atspre_fprint_size
(
  atstype_ref out, atstype_size x
) {
  int err = 0 ;
  atstype_ulint x2 = x ;
  err += fprintf((FILE*)out, "%lu", x2) ;
/*
  if (err < 0) {
    fprintf(stderr, "exit(ATS): [fprint_size] failed.") ; exit(1) ;
  } // end of [if]
*/
  return ;
} // end [atspre_fprint_size]

#define atspre_print_size(x) atspre_fprint_size(stdout, (x))
#define atspre_prerr_size(x) atspre_fprint_size(stderr, (x))

#endif

/* ****** ****** */

#define atspre_g1uint_succ_uint atspre_g0uint_succ_uint
#define atspre_g1uint_pred_uint atspre_g0uint_pred_uint
#define atspre_g1uint_half_uint atspre_g0uint_half_uint
#define atspre_g1uint_add_uint atspre_g0uint_add_uint
#define atspre_g1uint_sub_uint atspre_g0uint_sub_uint
#define atspre_g1uint_mul_uint atspre_g0uint_mul_uint
#define atspre_g1uint_div_uint atspre_g0uint_div_uint
#define atspre_g1uint_mod_uint atspre_g0uint_mod_uint
#define atspre_g1uint_isgtz_uint atspre_g0uint_isgtz_uint
#define atspre_g1uint_iseqz_uint atspre_g0uint_iseqz_uint
#define atspre_g1uint_isneqz_uint atspre_g0uint_isneqz_uint
#define atspre_g1uint_lt_uint atspre_g0uint_lt_uint
#define atspre_g1uint_lte_uint atspre_g0uint_lte_uint
#define atspre_g1uint_gt_uint atspre_g0uint_gt_uint
#define atspre_g1uint_gte_uint atspre_g0uint_gte_uint
#define atspre_g1uint_eq_uint atspre_g0uint_eq_uint
#define atspre_g1uint_neq_uint atspre_g0uint_neq_uint
#define atspre_g1uint_compare_uint atspre_g0uint_compare_uint
#define atspre_g1uint_max_uint atspre_g0uint_max_uint
#define atspre_g1uint_min_uint atspre_g0uint_min_uint

/* ****** ****** */

#define atspre_g1uint_succ_ulint atspre_g0uint_succ_ulint
#define atspre_g1uint_pred_ulint atspre_g0uint_pred_ulint
#define atspre_g1uint_half_ulint atspre_g0uint_half_ulint
#define atspre_g1uint_add_ulint atspre_g0uint_add_ulint
#define atspre_g1uint_sub_ulint atspre_g0uint_sub_ulint
#define atspre_g1uint_mul_ulint atspre_g0uint_mul_ulint
#define atspre_g1uint_div_ulint atspre_g0uint_div_ulint
#define atspre_g1uint_mod_ulint atspre_g0uint_mod_ulint
#define atspre_g1uint_isgtz_ulint atspre_g0uint_isgtz_ulint
#define atspre_g1uint_iseqz_ulint atspre_g0uint_iseqz_ulint
#define atspre_g1uint_isneqz_ulint atspre_g0uint_isneqz_ulint
#define atspre_g1uint_lt_ulint atspre_g0uint_lt_ulint
#define atspre_g1uint_lte_ulint atspre_g0uint_lte_ulint
#define atspre_g1uint_gt_ulint atspre_g0uint_gt_ulint
#define atspre_g1uint_gte_ulint atspre_g0uint_gte_ulint
#define atspre_g1uint_eq_ulint atspre_g0uint_eq_ulint
#define atspre_g1uint_neq_ulint atspre_g0uint_neq_ulint
#define atspre_g1uint_compare_ulint atspre_g0uint_compare_ulint
#define atspre_g1uint_max_ulint atspre_g0uint_max_ulint
#define atspre_g1uint_min_ulint atspre_g0uint_min_ulint

/* ****** ****** */

#define atspre_g1uint_succ_size atspre_g0uint_succ_size
#define atspre_g1uint_pred_size atspre_g0uint_pred_size
#define atspre_g1uint_half_size atspre_g0uint_half_size
#define atspre_g1uint_add_size atspre_g0uint_add_size
#define atspre_g1uint_sub_size atspre_g0uint_sub_size
#define atspre_g1uint_mul_size atspre_g0uint_mul_size
#define atspre_g1uint_div_size atspre_g0uint_div_size
#define atspre_g1uint_mod_size atspre_g0uint_mod_size
#define atspre_g1uint_isgtz_size atspre_g0uint_isgtz_size
#define atspre_g1uint_iseqz_size atspre_g0uint_iseqz_size
#define atspre_g1uint_isneqz_size atspre_g0uint_isneqz_size
#define atspre_g1uint_lt_size atspre_g0uint_lt_size
#define atspre_g1uint_lte_size atspre_g0uint_lte_size
#define atspre_g1uint_gt_size atspre_g0uint_gt_size
#define atspre_g1uint_gte_size atspre_g0uint_gte_size
#define atspre_g1uint_eq_size atspre_g0uint_eq_size
#define atspre_g1uint_neq_size atspre_g0uint_neq_size
#define atspre_g1uint_compare_size atspre_g0uint_compare_size
#define atspre_g1uint_max_size atspre_g0uint_max_size
#define atspre_g1uint_min_size atspre_g0uint_min_size

/* ****** ****** */

#endif // ifndef ATSLIB_PRELUDE_INTEGER_CATS

/* ****** ****** */

/* end of [integer.cats] */

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
