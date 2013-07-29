#ifndef _IPCMEM_CATS_
#define _IPCMEM_CATS_

#include "ats_types.h"
#include "ats_basics.h"
#include "types.h"
#include <user.h>

extern ipcmapping_t _ipcmappings[];
static inline ats_ptr_type _ipcmem_get_view_c(ats_int_type id, ats_int_type *pagesref) {
  *pagesref = _ipcmappings[id].pages;
  return _ipcmappings[id].address;
}

#endif

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
