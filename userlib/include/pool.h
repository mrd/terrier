/*
 * Static memory pool macros
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2011 Matthew Danish.
 *
 * All rights reserved. Please see enclosed LICENSE file for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _POOL_H_
#define _POOL_H_

#define POOL_PROTO(name,ty)                     \
  extern ty *name##_pool_alloc(void);           \
  extern void name##_pool_free(ty *)

#define POOL_DEFN(name,ty,size,align)                           \
  ALIGNED(align,static ty, __##name##_pool_array[size]);        \
  static u32 __##name##_pool_bitmap[size >> 3];                 \
  void name##_pool_init(void) {                                 \
    int i;                                                      \
    for(i=0;i<size;i++)                                         \
      BITMAP_CLR(__##name##_pool_bitmap, i);                    \
  }                                                             \
  ty *name##_pool_alloc(void) {                                 \
    int i;                                                      \
    for(i=0;i<size;i++)                                         \
      if(BITMAP_TST(__##name##_pool_bitmap, i) == 0) {          \
        BITMAP_SET(__##name##_pool_bitmap, i);                  \
        return &__##name##_pool_array[i];                       \
      }                                                         \
    return NULL;                                                \
  }                                                             \
  void name##_pool_free(ty *x) {                                \
    int i;                                                      \
    for(i=0;i<size;i++)                                         \
      if((u32) x == (u32) &__##name##_pool_array[i]) {          \
        BITMAP_CLR(__##name##_pool_bitmap, i);                  \
        return;                                                 \
      }                                                         \
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
