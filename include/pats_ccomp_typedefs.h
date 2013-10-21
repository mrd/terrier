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
(* Author: Hongwei Xi *)
(* Authoremail: hwxi AT cs DOT bu DOT edu *)
(* Start time: October, 2012 *)
*/

/* ****** ****** */

#ifndef PATS_CCOMP_TYPEDEFS_H
#define PATS_CCOMP_TYPEDEFS_H

/* ****** ****** */

struct atstype_struct ; /* of indefinite size */

/* ****** ****** */

typedef void atstype_void ;
typedef void atsvoid_t0ype ;

/* ****** ****** */

typedef int atstype_int ;
typedef unsigned int atstype_uint ;

typedef long int atstype_lint ;
typedef unsigned long int atstype_ulint ;

typedef long long int atstype_llint ;
typedef unsigned long long int atstype_ullint ;

typedef short int atstype_sint ;
typedef unsigned short int atstype_usint ;

/* ****** ****** */

typedef atstype_lint atstype_ssize ;
typedef atstype_ulint atstype_size ;

/* ****** ****** */
//
// HX: true/false: 1/0
//
typedef int atstype_bool ;
//
/* ****** ****** */

typedef unsigned char atstype_byte ;

/* ****** ****** */

typedef char atstype_char ;
typedef signed char atstype_schar ;
typedef unsigned char atstype_uchar ;

/* ****** ****** */

typedef char *atstype_string ;
typedef char *atstype_stropt ;
typedef char *atstype_strptr ;

/* ****** ****** */

typedef float atstype_float ;
typedef double atstype_double ;
typedef long double atstype_ldouble ;

/* ****** ****** */
/*
** HX: for pointers
*/
typedef void *atstype_ptr ;
typedef void *atstype_ptrk ;
/*
** HX: for references
*/
typedef void *atstype_ref ;
/*
** HX: for boxed values
*/
typedef void* atstype_boxed ;
/*
** HX: for [datconptr]
*/
typedef void* atstype_datconptr ;
/*
** HX: for [datcontyp]
*/
typedef void* atstype_datcontyp ;

/* ****** ****** */

#ifndef _ATS_EXCEPTION_NONE
//
typedef
struct
{
  int exntag ; char *exnmsg ;
} atstype_exncon ;
typedef atstype_exncon *atstype_exnconptr ;
//
#endif // end of [_ATS_EXCEPTION_NONE]

/* ****** ****** */
/*
** HX: for pointers to arrays
*/
typedef void* atstype_arrptr ;
/*
** HX: for arrays plus size info
*/
typedef
struct {
  atstype_arrptr ptr ; atstype_size size ;
} atstype_arrpsz ;

/* ****** ****** */

typedef void* atstype_funptr ;
typedef void* atstype_cloptr ;

/* ****** ****** */

#define atstkind_type(tk) tk
#define atstkind_t0ype(tk) tk

/* ****** ****** */

typedef struct{} atstype_var ;
#define atstyvar_type(a) atstype_var

/* ****** ****** */

#define atstybox_type(hit) atstype_boxed

/* ****** ****** */

#define atsrefarg0_type(hit) hit
#define atsrefarg1_type(hit) atstype_ref

/* ****** ****** */

#endif /* PATS_CCOMP_TYPEDEFS_H */

/* ****** ****** */

/* end of [pats_ccomp_typedefs.h] */
