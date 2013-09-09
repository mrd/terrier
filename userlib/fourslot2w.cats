#ifndef _FOURSLOT2W_CATS_
#define _FOURSLOT2W_CATS_

#include "ats_types.h"
#include "ats_basics.h"
#include "types.h"
#include <user.h>

#define PAGE_SIZE_LOG2 12
#define CACHE_LINE_SIZE_LOG2 6
#define CACHE_LINE_SIZE (1<<CACHE_LINE_SIZE_LOG2)

// needed for ATS1:
typedef enum { fourslot2w_A = 0, fourslot2w_B } fourslot2w_side_t;

#define fourslot2w_splitA_c(p,elemsz1,elemsz2) \
  (((void *) p) + ((3 << CACHE_LINE_SIZE_LOG2) + 4*((((elemsz1 - 1) >> CACHE_LINE_SIZE_LOG2) + 1) << CACHE_LINE_SIZE_LOG2)))
#define fourslot2w_splitB_c(p,elemsz1,elemsz2) fourslot2w_splitA_c(p,elemsz1,elemsz2)

// memory layout in cache lines:
// [latest, reading] [slot0] [slot1] [data0] [data1] [data2] [data3] [latest, reading] [slot0] [slot1] [data0] [data1] [data2] [data3]

#define _fourslot2w_init_c(p,pages,side,elemsz1,elemsz2) fourslot2w_init_c((void *) p, (int) pages, side, (int) elemsz1, (int) elemsz2)
static inline int fourslot2w_init_c(void *p, int pages, int side, int elemsz1, int elemsz2)
{
  if(elemsz1 < 1 || elemsz2 < 1) return EINVALID;
  int elemlines1 = ((elemsz1 - 1) >> CACHE_LINE_SIZE_LOG2) + 1;
  int elemlines2 = ((elemsz2 - 1) >> CACHE_LINE_SIZE_LOG2) + 1;

  int reqbytes1 = ((3 << CACHE_LINE_SIZE_LOG2) + 4*(elemlines1 << CACHE_LINE_SIZE_LOG2));
  int reqbytes2 = ((3 << CACHE_LINE_SIZE_LOG2) + 4*(elemlines2 << CACHE_LINE_SIZE_LOG2));
  int reqbytes = reqbytes1 + reqbytes2;

  if((pages << PAGE_SIZE_LOG2) < reqbytes) return ENOSPACE;

  // clear "write" area of given "side"
  if(side == fourslot2w_B) memset(p, 0, reqbytes1);
  else memset(p + reqbytes1, 0, reqbytes2);

  return OK;
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
