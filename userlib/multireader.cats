#ifndef _MULTIREADER_CATS_
#define _MULTIREADER_CATS_

#include "arm/asm.h"
#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "types.h"
#include <user.h>

#define PAGE_SIZE_LOG2 12
#define CACHE_LINE_SIZE_LOG2 6
#define CACHE_LINE_SIZE (1<<CACHE_LINE_SIZE_LOG2)

#define __ATOMIC_MODEL __ATOMIC_SEQ_CST

/* FIXME: layout for multislot reader, num_readers = n */

/* [n, ip, w[0], w[1], op_0, r_0[0], r_0[1], ..., op_{n-1}, r_{n-1}[0], r_{n-1}[1], data...] */
#define N(ms) (ms)
#define IP(ms) (ms+1)
#define W(ms,j) (ms+2+j)
#define OP(ms,i) (ms+3*i+4)
#define R(ms,i,j) (ms+3*i+5+j)
#define DATA(ms) (ms+448)

#ifndef atspre_neg_bool1
#define atspre_neg_bool1 atspre_neg_bool
static inline ats_bool_type atspre_neg_bool(ats_bool_type b) {
  return (b ? ats_false_bool : ats_true_bool) ;
} // end of [atspre_neg_bool]
#endif

static inline int _multireader_initialize_reader_c(void *p, int pages, int elemsz, int *iref)
{
  int elemlines, reqbytes;

  if(elemsz < 1) return EINVALID;

  /* atomic-increment */

  register int Ra, Rb, Rc = (int) N((int *) p);
  /* can be done in one LL/SC atomic operation: */
  ASM("1: LDREX %[Ra], [%[Rc]]\n"
      "ADD %[Ra], %[Ra], $1\n"
      "STREX %[Rb], %[Ra], [%[Rc]]\n"
      "CMP %[Rb], #0\nBNE 1b":[Ra] "=r" (Ra), [Rb] "=r" (Rb), [Rc] "+r" (Rc));

  /* Ra = number of readers, now */
  /* 2n+2 slots = (2 * Ra + 2) */
  elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1;
  reqbytes = ((3 << CACHE_LINE_SIZE_LOG2) + (2 * Ra + 2) * (elemlines << CACHE_LINE_SIZE_LOG2));

  if((pages << PAGE_SIZE_LOG2) < reqbytes) {
    /* rollback */
    /* can be done in one LL/SC atomic operation: */
    ASM("1: LDREX %[Ra], [%[Rc]]\n"
        "SUB %[Ra], %[Ra], $1\n"
        "STREX %[Rb], %[Ra], [%[Rc]]\n"
        "CMP %[Rb], #0\nBNE 1b":[Ra] "=r" (Ra), [Rb] "=r" (Rb), [Rc] "+r" (Rc));

    return ENOSPACE;
  }

  *iref = Ra - 1;

  return OK;
}

// for the time being, single reader layout:
// ms = [op, ip, r0, r1, w0, w1, data0, data1, data2, data3]

static inline int _multireader_initialize_writer_c(void *p, int pages, int elemsz)
{
  // FIXME: this assumes cache-line-size elements but for testing purposes I won't bother with that
  if(elemsz < 1) return EINVALID;
  int elemlines = ((elemsz - 1) >> CACHE_LINE_SIZE_LOG2) + 1;

  /* check if there's enough space for at least one reader */
  int n = __atomic_load_n(N((int *) p), __ATOMIC_MODEL);
  n = (n > 0 ? n : 1);
  int reqbytes = ((3 << CACHE_LINE_SIZE_LOG2) + (2 * n + 2) * (elemlines << CACHE_LINE_SIZE_LOG2));
  if((pages << PAGE_SIZE_LOG2) < reqbytes) return ENOSPACE;
  memset(((u8 *)p)+4, 0, reqbytes-4); /* don't clear first word */
  return OK;
}


/* hard limit on n is 32 (indices 0..31) because of _ws3 */

/* data begins at 4+3*32 = 100 words = 400 bytes aligned up to cacheline size, so 448 */

/* QUESTION: should non-cache-aligned data be an option? */

/* FIXME: initialization for multireader (should it require static number of readers be declared?) */

/* FIXME: reuse of indices when readers drop out */

static inline void _rs1(int *ms, int i)
/* RS1: r_i <- w */
{
  __atomic_store_n(R(ms,i,0), __atomic_load_n(W(ms,0), __ATOMIC_MODEL), __ATOMIC_MODEL);
  __atomic_store_n(R(ms,i,1), __atomic_load_n(W(ms,1), __ATOMIC_MODEL), __ATOMIC_MODEL);
}

static inline void _rs2(int *ms, int i)
/* RS2: op_i <- ~ip */
{
  __atomic_store_n(OP(ms,i), !__atomic_load_n(IP(ms), __ATOMIC_MODEL), __ATOMIC_MODEL);
}

static inline void _rs3(int *ms, int i, void *dest, int size)
/* RS3: read data from d[op_i, r_i[op_i]] */
{
  int op = __atomic_load_n(OP(ms,i), __ATOMIC_MODEL);
  int r_op = __atomic_load_n(R(ms,i,op), __ATOMIC_MODEL);
  ASM("DMB");                   /* ensure any writes have completed */

  /* FIXME: don't I need the size of the type? at least cacheline size, of course */

  memcpy(dest, DATA(ms) + ((r_op * 2 + op) << CACHE_LINE_SIZE_LOG2), size);
}



#define _ws1(_ms, item, size)                                           \
  /* WS1: write data into d[ip, w[ip]] */                               \
  {                                                                     \
    int *ms = (int *) _ms;                                              \
    int ip = __atomic_load_n(IP(ms), __ATOMIC_MODEL);                   \
    int w_ip =  __atomic_load_n(W(ms,ip), __ATOMIC_MODEL);              \
    memcpy(DATA(ms) + ((w_ip * 2 + ip) << CACHE_LINE_SIZE_LOG2), &item, size); \
  }

static inline void _ws2(int *ms)
/* WS2: ip <- ~ip */
{
  /* are multiple, separate atomic operations satisfactory? */
  //__atomic_store_n(ms+1, !__atomic_load_n(ms+1, __ATOMIC_MODEL), __ATOMIC_MODEL);

  register int Ra, Rb, Rc = (int) IP(ms);
  /* can be done in one LL/SC atomic operation: */
  ASM("1: LDREX %[Ra], [%[Rc]]\n"
      "MVN %[Ra], %[Ra]\n"
      "STREX %[Rb], %[Ra], [%[Rc]]\n"
      "CMP %[Rb], #0\nBNE 1b":[Ra] "=r" (Ra), [Rb] "=r" (Rb), [Rc] "+r" (Rc));
}

/* FIXME: prove linearizability */
static inline void _ws3(int *ms)
/* WS3: w[ip] <- ~(r_0[ip],...,r_1[ip]) */
{
  int ip = __atomic_load_n(IP(ms), __ATOMIC_MODEL);
  int n = __atomic_load_n(N(ms), __ATOMIC_MODEL);
  int x = 0, i;

  /* supports 0 <= n < 32 */
  for(i=0;i<n;i++)
    x |= (1 << __atomic_load_n(R(ms,i,ip), __ATOMIC_MODEL));
  int neg = 31 - count_leading_zeroes(~x);

  __atomic_store_n(W(ms,ip), neg, __ATOMIC_MODEL);
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
