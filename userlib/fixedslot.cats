#ifndef _FIXEDSLOT_CATS_
#define _FIXEDSLOT_CATS_

#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "types.h"
#include <user.h>

#define PAGE_SIZE_LOG2 12
#define CACHE_LINE_SIZE_LOG2 6
#define CACHE_LINE_SIZE (1<<CACHE_LINE_SIZE_LOG2)

// fs status word: p=2 bits, t=2 bits, f=2 bits, S=16 least significant bits

static inline int _pick_ri (unsigned int *fs)
{
/* incr S
 * if(p == t) t <- f
 * ri <- t */
  register int w, ri, status, fs0 = (int) fs;
  register int t, p, f;
  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "ADD %[w], %[w], #1\n"            /* incr S */
      "AND %[t], %[w], #3 << 18\n"      /* t = w & (3 << 18) */
      "AND %[p], %[w], #3 << 20\n"      /* p = w & (3 << 20) */
      "MOV %[p], %[p], LSR #2\n"        /* p = p >> 2 */
      "CMP %[t], %[p]\n"                /* p == t ? */

      "BICEQ %[w], %[w], #3 << 18\n" /* if (p == t) clear bit 18:19 */
      "ANDEQ %[f], %[w], #3 << 16\n" /* if (p == t) f = w & (3 << 16) */
      "ORREQ %[w], %[w], %[f], LSL #2\n" /* if (p == t) w |= (f << 2) // because (f << 2) overwrites t */

      "STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b\n"

      "AND %[t], %[w], #3 << 18\n" /* t = w & (3 << 18) */
      "MOV %[ri], %[t], LSR #18\n"      /* ri = t >> 18 */

      :[fs] "+r" (fs0), [w] "=r" (w), [status] "=r" (status), [ri] "=r" (ri), [t] "=r" (t), [p] "=r" (p), [f] "=r" (f)
      );
  return ri;
}

/* rcount comes after the first word */
static inline void _incr_rcount (unsigned int *fs, int i)
{
  register int w, status, rcount = (int) (fs + 1 + i);
  ASM("1: LDREX %[w], [%[rcount]]\n"        /* load link... */
      "ADD %[w], %[w], #1\n"                /* rcount[i]++ */
      "STREX %[status], %[w], [%[rcount]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"
      :[rcount] "+r" (rcount), [w] "=r" (w), [status] "=r" (status));
}

static inline void _decr_rcount (unsigned int *fs, int i)
{
  register int w, status, rcount = (int) (fs + 1 + i);
  ASM("1: LDREX %[w], [%[rcount]]\n"        /* load link... */
      "SUB %[w], %[w], #1\n"                /* rcount[i]-- */
      "STREX %[status], %[w], [%[rcount]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"
      :[rcount] "+r" (rcount), [w] "=r" (w), [status] "=r" (status));
}

static inline void _decr_S (unsigned int *fs)
{
  register int w, status, fs0 = (int) fs;
  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "SUB %[w], %[w], #1\n"            /* S-- */
      "STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"
      :[fs] "+r" (fs0), [w] "=r" (w), [status] "=r" (status));
}

static inline void _check_previous (unsigned int *fs)
{
  /* if (S == 0 && rcount[p] == 0) p = t; else p = p; */

  register int w, rc, status, fs0 = (int) fs;
  register int t, p, f;

  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "MOVW %[status], #0xFFFF\n"
      "ANDS %[status], %[status], %[w]\n" /* S == 0? set Zero Flag if result is 0 */
      "BNE 2f\n"
      "AND %[p], %[w], #3 << 20\n" /* p = w & (3 << 20) */
      "MOV %[p], %[p], LSR #20\n"  /* p >>= 20 */
      "ADD %[p], %[p], #1\n"       /* p++ */
      "DMB; LDR %[rc], [%[fs], %[p], LSR #2]\n"  /* rc = rcount[p] */
      "CMP %[rc], #0\n"         /* test if rcount[p] == 0? */
      "BICEQ %[w], %[w], #3 << 20\n" /* if (S == 0 && rcount[p] == 0) clear bits 20:21 */
      "ANDEQ %[t], %[w], #3 << 18\n" /* if (S == 0 && rcount[p] == 0) t = w & (3 << 18) */
      "ORREQ %[w], %[w], %[t], LSL #2\n" /* if (...) w |= (t << 2) // because (t << 2) overwrites p */

      "2: STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"

      :[fs] "+r" (fs0), [w] "=r" (w), [status] "=r" (status), [rc] "=r" (rc), [t] "=r" (t), [p] "=r" (p), [f] "=r" (f)
      );
}

static inline void _read_data(void *fs, unsigned int ri, void *dest, unsigned int size)
{
  memcpy (dest, fs + CACHE_LINE_SIZE + (ri * size), size);
}

static inline void _get_triple (unsigned int *fs, int *p, int *t, int *f)
{
  register int w, rp, rt, rf, fs0 = (int) fs;
  ASM("DMB; LDR %[w], [%[fs]]\n"
      "AND %[f], %[w], #3 << 16\n" /* f = w & (3 << 16) */
      "AND %[t], %[w], #3 << 18\n" /* t = w & (3 << 18) */
      "AND %[p], %[w], #3 << 20\n" /* p = w & (3 << 20) */
      "MOV %[f], %[f], LSR #16\n"        /* f >>= 16 */
      "MOV %[t], %[t], LSR #18\n"        /* t >>= 18 */
      "MOV %[p], %[p], LSR #20\n"        /* p >>= 20 */
      :[fs] "+r" (fs0), [w] "=r" (w), [t] "=r" (rt), [p] "=r" (rp), [f] "=r" (rf)
      );
  *p = rp; *t = rt; *f = rf;
}

static inline void _set_wfilled (unsigned int *fs, unsigned int f)
{
  register int w, status, fs0 = (int) fs;
  ASM("1: LDREX %[w], [%[fs]]\n"       /* load link ... */
      "BIC %[w], %[w], #3 << 16\n"     /* clear bits 16:17 */
      "ORR %[w], %[w], %[f], LSL #16\n"  /* w |= (f << 16) */
      "STREX %[status], %[w], [%[fs]]\n" /* ... store conditional */
      "CMP %[status], #0\nBNE 1b"
      :[fs] "+r" (fs0), [w] "=r" (w), [status] "=r" (status): [f] "r" (f)
      );
}

#define _write_data(fs, wi, src, size)                          \
  {                                                             \
    memcpy (fs + CACHE_LINE_SIZE + (wi * size), &src, size);    \
  }

static inline int _intset_nil (void) { return 0; }
static inline int _intset_cons (int x, int s) { return ((1 << x) | s); }
static inline int _intset_ffz (int s)
{
  register int i;               /* figure out a faster way later */
  ASM("ANDS %[i], %[s], #1\n"
      "MOVEQ %[i], #0\nBEQ 1f\n"
      "ANDS %[i], %[s], #2\n"
      "MOVEQ %[i], #1\nBEQ 1f\n"
      "ANDS %[i], %[s], #4\n"
      "MOVEQ %[i], #2\nBEQ 1f\n"
      "ANDS %[i], %[s], #8\n"
      "MOVEQ %[i], #3\n"
      "1:"
      :[i] "=r" (i):[s] "r" (s));
  return i;
}

#define _fixedslot_initialize_reader(x,n) x
#define _fixedslot_initialize_writer(x,n) x
#define _fixedslot_free(x) x

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
