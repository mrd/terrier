#ifndef _MULTISLOT_CATS_
#define _MULTISLOT_CATS_

#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "types.h"
#include <user.h>

#define PAGE_SIZE_LOG2 12
#define CACHE_LINE_SIZE_LOG2 6
#define CACHE_LINE_SIZE (1<<CACHE_LINE_SIZE_LOG2)

#ifndef atspre_neg_bool1
#define atspre_neg_bool1 atspre_neg_bool
static inline ats_bool_type atspre_neg_bool(ats_bool_type b) {
  return (b ? ats_false_bool : ats_true_bool) ;
} // end of [atspre_neg_bool]
#endif

#define _multislot_initialize_reader_c(p,pages,elemsz) OK

// for the time being:
// ms = [op, ip, r0, r1, w0, w1, data0, data1, data2, data3]

static inline int _multislot_initialize_writer_c(void *p, int pages, int elemsz)
{
  // FIXME: this assumes cache-line-size elements but for testing purposes I won't bother with that
  if(elemsz < 1) return EINVALID;
  int elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1;
  int reqbytes = ((3 << CACHE_LINE_SIZE_LOG2) + 4*(elemlines << CACHE_LINE_SIZE_LOG2));
  if((pages << PAGE_SIZE_LOG2) < reqbytes) return ENOSPACE;
  memset(p, 0, reqbytes);
  return OK;
}

#define __ATOMIC_MODEL __ATOMIC_SEQ_CST

static inline void _rs1(int *ms)
/* RS1: r <- w */
{
  /* are multiple, separate atomic operations satisfactory? */
  __atomic_store_n(ms+2, __atomic_load_n(ms+4, __ATOMIC_MODEL), __ATOMIC_MODEL);
  __atomic_store_n(ms+3, __atomic_load_n(ms+5, __ATOMIC_MODEL), __ATOMIC_MODEL);
}

static inline void _rs2(int *ms)
/* RS2: op <- ~ip */
{
  /* are multiple, separate atomic operations satisfactory? */
  __atomic_store_n(ms+0, !__atomic_load_n(ms+1, __ATOMIC_MODEL), __ATOMIC_MODEL);
}

static inline void _rs3(int *ms, void *dest, int size)
/* RS3: read data from d[op, r[op]] */
{
  int op = __atomic_load_n(ms+0, __ATOMIC_MODEL);
  int r_op = __atomic_load_n(ms+2+op, __ATOMIC_MODEL);
  ASM("DMB");                   /* ensure any writes have completed */
  memcpy(dest, (ms+6) + (op*2) + r_op, size);
}

#define _ws1(_ms, item, size)                           \
/* WS1: write data into d[ip, w[ip]] */                 \
{                                                       \
  int *ms = (int *) _ms;                                \
  int ip = __atomic_load_n(ms+1, __ATOMIC_MODEL);       \
  int w_ip =  __atomic_load_n(ms+4+ip, __ATOMIC_MODEL); \
  memcpy((ms+6) + (ip*2) + w_ip, &item, size);          \
}

static inline void _ws2(int *ms)
/* WS2: ip <- ~ip */
{
  /* are multiple, separate atomic operations satisfactory? */
  //__atomic_store_n(ms+1, !__atomic_load_n(ms+1, __ATOMIC_MODEL), __ATOMIC_MODEL);

  register int Ra, Rb, Rc = (int) (ms + 1);
  /* can be done in one LL/SC atomic operation: */
  ASM("1: LDREX %[Ra], [%[Rc]]\n"
      "MVN %[Ra], %[Ra]\n"
      "STREX %[Rb], %[Ra], [%[Rc]]\n"
      "CMP %[Rb], #0\nBNE 1b":[Ra] "=r" (Ra), [Rb] "=r" (Rb), [Rc] "+r" (Rc));
}

static inline void _ws3(int *ms)
/* WS3: w[ip] <- ~r[ip] */
{
  /* are multiple, separate atomic operations satisfactory? */
  int ip = __atomic_load_n(ms+1, __ATOMIC_MODEL);
  __atomic_store_n(ms+4+ip, !__atomic_load_n(ms+2+ip, __ATOMIC_MODEL), __ATOMIC_MODEL);
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
