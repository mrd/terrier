#ifndef _ATS_TYPES_H_
#define _ATS_TYPES_H_

#include "types.h"
#include "pats_ccomp_typedefs.h"
#include "pats_ccomp_instrset.h"
#include "pats_ccomp_basics.h"

typedef u32 ats_bool_type, ats_uint_type;
typedef s32 ats_int_type;
typedef void ats_void_type;
typedef void *ats_ptr_type, *ats_ref_type, *ats_clo_ptr_type, *ats_fun_ptr_type;
struct ats_struct_type; /* of indefinite size */
typedef struct ats_struct_type ats_abs_type;
typedef struct { int tag ; } ats_sum_type ;
typedef ats_sum_type *ats_sum_ptr_type ;
typedef u8 atstype_uint8;


#define ats_false_bool 0
#define ats_true_bool 1

typedef void atstype_exncon;
typedef void atsvoid_t0ype;

#define atstyvar_type(a) atstype_var

//#define ATSdyncst_extfun(d2c, targs, tres) ATSglobaldec() tres d2c targs
#define ATSglobaldec() extern
#define ATSdyncst_mac(d2c)

#endif
