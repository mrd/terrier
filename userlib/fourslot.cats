#ifndef _FOURSLOT_CATS_
#define _FOURSLOT_CATS_

#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "types.h"
#include <user.h>

#define PAGE_SIZE_LOG2 12
#define CACHE_LINE_SIZE_LOG2 6
#define CACHE_LINE_SIZE (1<<CACHE_LINE_SIZE_LOG2)

#define atspre_neg_bool1 atspre_neg_bool
static inline ats_bool_type atspre_neg_bool(ats_bool_type b) {
  return (b ? ats_false_bool : ats_true_bool) ;
} // end of [atspre_neg_bool]

#define _fourslot_ipc_reader_init_c(p,pages) OK

#define _fourslot_ipc_writer_init_c(p,pages,elemsz) fourslot_ipc_writer_init_c((void *) p, (int) pages, (int) elemsz)
static inline int fourslot_ipc_writer_init_c(void *p, int pages, int elemsz)
{
  if(elemsz < 1) return EINVALID;
  int elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1;
  // lines: [latest, reading] [slot0] [slot1] [data0] [data1] [data2] [data3]
  int reqbytes = ((3 << CACHE_LINE_SIZE_LOG2) + 4*(elemlines << CACHE_LINE_SIZE_LOG2));
  if((pages << PAGE_SIZE_LOG2) < reqbytes) return ENOSPACE;
  memset(p, 0, reqbytes);
  return OK;
}

#define _write_data_c(fs,p,i,item,elemsz) do { \
    int elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1; \
    void *ptr = ((void *) fs) + ((3 << CACHE_LINE_SIZE_LOG2) + (2 * p + i)*(elemlines << CACHE_LINE_SIZE_LOG2)); \
    memcpy(ptr,&item,elemsz); \
  } while (0)

#define __ATOMIC_MODEL __ATOMIC_SEQ_CST

#define _get_reading_c(fs) !!__atomic_load_n(((int *) fs) + 1, __ATOMIC_MODEL)
#define _write_latest_c(fs,l) __atomic_store_n(((int *) fs), l, __ATOMIC_MODEL)

#define _get_write_slot_c(fs,wp) !!__atomic_load_n(((char *) fs) + ((1 + wp) << CACHE_LINE_SIZE_LOG2), __ATOMIC_MODEL)
#define _write_slot_c(fs,wp,wi) __atomic_store_n(((char *) fs) + ((1 + wp) << CACHE_LINE_SIZE_LOG2), (char) wi, __ATOMIC_MODEL)


#define _write_reading_c(fs,r) __atomic_store_n(((int *) fs) + 1, r, __ATOMIC_MODEL)
#define _get_latest_c(fs) !!__atomic_load_n(((int *) fs), __ATOMIC_MODEL)
#define _read_data_c(fs,p,i,item,elemsz) do { \
    int elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1; \
    void *ptr = ((void *) fs) + ((3 << CACHE_LINE_SIZE_LOG2) + (2 * p + i)*(elemlines << CACHE_LINE_SIZE_LOG2)); \
    memcpy(item,ptr,elemsz); \
  } while (0)
#define _get_read_slot_c(fs,rp) !!__atomic_load_n(((char *) fs) + ((1 + rp) << CACHE_LINE_SIZE_LOG2), __ATOMIC_MODEL)

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
