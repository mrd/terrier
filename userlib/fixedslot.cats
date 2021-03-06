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
#define __ATOMIC_MODEL __ATOMIC_SEQ_CST

// layout of fixedslot region in IPC memory:
// [status word], [rcount0] ... [rcount3], ... cache line offset ..., [data]

// fs status word: p=2 bits, t=2 bits, f=2 bits, S=16 least significant bits
// data starts at cache-line-size offset
// probably should separate slots into separate cache lines too

struct fixedslot { u32 *p; };
typedef struct fixedslot fixedslot_t;

static inline int _pick_ri (fixedslot_t fs)
{
  /* incr S
   * if(p == t) t <- f
   * ri <- t */
  register int w, status, fs0 = (int) fs.p;
  register int t, p, f;
  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "AND %[t], %[w], #3 << 18\n"      /* t = w & (3 << 18) */
      "AND %[p], %[w], #3 << 20\n"      /* p = w & (3 << 20) */
      "ADD %[w], %[w], #1\n"            /* incr S */
      "MOV %[p], %[p], LSR #2\n"        /* p = p >> 2 */
      "CMP %[t], %[p]\n"                /* p == t ? */

      "BICEQ %[w], %[w], #3 << 18\n" /* if (p == t) clear bit 18:19 */
      "ANDEQ %[f], %[w], #3 << 16\n" /* if (p == t) f = w & (3 << 16) */
      "ORREQ %[w], %[w], %[f], LSL #2\n" /* if (p == t) w |= (f << 2) // because (f << 2) overwrites t */

      "STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b\n"

      :[w] "=&r" (w), [status] "=&r" (status), [t] "=&r" (t), [p] "=&r" (p), [f] "=&r" (f):[fs] "r" (fs0):"cc"
      );
  return GETBITS(t,18,2);
}

/* rcount comes after the first word */
static inline void _incr_rcount (fixedslot_t fs, int i)
{
  __atomic_add_fetch (&fs.p[1 + i], 1, __ATOMIC_MODEL);
}

static inline void _decr_rcount (fixedslot_t fs, int i)
{
  __atomic_sub_fetch (&fs.p[1 + i], 1, __ATOMIC_MODEL);
}

static inline void _decr_S (fixedslot_t fs)
{
  __atomic_sub_fetch (&fs.p[0], 1, __ATOMIC_MODEL);
}

static inline void _check_previous (fixedslot_t fs)
{
  /* if (S == 0 && rcount[p] == 0) p = t; else p = p; */

  register int w, rc, status, fs0 = (int) fs.p;
  register int p, t;

  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "MOVW %[status], #0xFFFF\n"
      "ANDS %[status], %[status], %[w]\n" /* S == 0? set Zero Flag if result is 0 */
      "BNE 2f\n"
      "AND %[p], %[w], #3 << 20\n" /* p = w & (3 << 20) */
      "MOV %[p], %[p], LSR #20\n"  /* p >>= 20 */
      "ADD %[p], %[p], #1\n"       /* p++ */
      "LDR %[rc], [%[fs], %[p], LSL #2]\n"  /* rc = rcount[p] */
      "CMP %[rc], #0\n"         /* test if rcount[p] == 0? */
      "BICEQ %[w], %[w], #3 << 20\n" /* if (S == 0 && rcount[p] == 0) clear bits 20:21 */
      "ANDEQ %[t], %[w], #3 << 18\n" /* if (S == 0 && rcount[p] == 0) t = w & (3 << 18) */
      "ORREQ %[w], %[w], %[t], LSL #2\n" /* if (...) w |= (t << 2) // because (t << 2) overwrites p */
      "2: STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"

      : [w] "=&r" (w), [status] "=&r" (status), [rc] "=&r" (rc), [p] "=&r" (p), [t] "=&r" (t): [fs] "r" (fs0): "cc"
      );
}

static inline void _read_data(fixedslot_t fs, unsigned int ri, void *dest, unsigned int size)
{
  memcpy (dest, ((void *) fs.p) + CACHE_LINE_SIZE + (ri * size), size);
}

#define _takeout_read_data_ptr(fs,ri,size) (((void *) (fs).p) + CACHE_LINE_SIZE + ((ri) * (size)))

static inline void _get_triple (fixedslot_t fs, int *p, int *t, int *f)
{
  u32 w = __atomic_load_n(&fs.p[0], __ATOMIC_MODEL);
  *p = GETBITS(w,20,2);
  *t = GETBITS(w,18,2);
  *f = GETBITS(w,16,2);
}

static inline void _set_wfilled (fixedslot_t fs, unsigned int wi)
{
  register int w, status, fs0 = (int) fs.p, f = wi & 3;
  ASM("1: LDREX %[w], [%[fs]]\n"       /* load link ... */
      "BIC %[w], %[w], #3 << 16\n"     /* clear bits 16:17 */
      "ORR %[w], %[w], %[f], LSL #16\n"  /* w |= (f << 16) */
      "STREX %[status], %[w], [%[fs]]\n" /* ... store conditional */
      "CMP %[status], #0\nBNE 1b"
      :[w] "=&r" (w), [status] "=&r" (status): [f] "r" (f), [fs] "r" (fs0):"cc"
      );
}

#define _write_data(fs, wi, src, size)                          \
  {                                                             \
    memcpy (((void *) fs.p) + CACHE_LINE_SIZE + (wi * size), &src, size); \
  }
#define _write_data_ptr(fs, wi, src, size)                      \
  {                                                             \
    memcpy (((void *) fs.p) + CACHE_LINE_SIZE + (wi * size), src, size);  \
  }

static inline int _intset_nil (void) { return 0; }
static inline int _intset_cons (int x, int s) { return ((1 << x) | s); }
static inline int _intset_ffz (int s)
{
  register int i;               /* figure out a faster way later */
  ASM("TST %[s], #1\n"
      "MOVEQ %[i], #0\nBEQ 1f\n"
      "TST %[s], #2\n"
      "MOVEQ %[i], #1\nBEQ 1f\n"
      "TST %[s], #4\n"
      "MOVEQ %[i], #2\nBEQ 1f\n"
      "TST %[s], #8\n"
      "MOVEQ %[i], #3\n"
      "1:"
      :[i] "=&r" (i):[s] "r" (s):"cc");
  return i;
}

#define _fixedslot_initialize_reader(x,n) ((struct fixedslot) { .p = x })
#define _fixedslot_initialize_writer(x,n) ((struct fixedslot) { .p = x })
#define _fixedslot_free(fs) (fs).p

#if 0
static inline void _check_previous (fixedslot_t fs)
{
  /* if (S == 0 && rcount[p] == 0) p = t; else p = p; */

  register int t, w, status, fs0 = (int) fs.p;

  /* FIXME: verify this */
  { u32 w = __atomic_load_n(&fs.p[0], __ATOMIC_MODEL);
    u32 p = GETBITS(w,20,2);
    u32 rc_p = __atomic_load_n (&fs.p[1 + p], __ATOMIC_MODEL);
    if (rc_p != 0) return;
  }

  ASM("1: LDREX %[w], [%[fs]]\n"        /* load link... */
      "MOVW %[status], #0xFFFF\n"
      "TST %[w], %[status]\n"   /* S == 0?  */
      "BICEQ %[w], %[w], #3 << 20\n" /* if (S == 0 && rcount[p] == 0) clear bits 20:21 */
      "ANDEQ %[t], %[w], #3 << 18\n" /* if (S == 0 && rcount[p] == 0) t = w & (3 << 18) */
      "ORREQ %[w], %[w], %[t], LSL #2\n" /* if (...) w |= (t << 2) // because (t << 2) overwrites p */
      "STREX %[status], %[w], [%[fs]]\n" /* ...store conditional */
      "CMP %[status], #0\nBNE 1b"

      :[w] "=&r" (w), [status] "=&r" (status), [t] "=&r" (t):[fs] "r" (fs0):"cc"
      );
}
#endif

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
