#ifndef _ATS_TYPES_H_
#define _ATS_TYPES_H_

#include "types.h"

typedef u32 ats_bool_type, ats_uint_type;
typedef s32 ats_int_type;
typedef void ats_void_type;
typedef void *ats_ptr_type, *ats_ref_type, *ats_clo_ptr_type, *ats_fun_ptr_type;

static inline ats_bool_type atspre_ptr_is_null(ats_ptr_type p)
{
  return p == NULL;
}

#endif
