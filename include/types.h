/*
 * C type definitions
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

#ifndef _TYPES_H_
#define _TYPES_H_

#include "status.h"

#ifdef __GNUC__
#define PACKED __attribute__((packed))
#define PACKED_STRUCT(name) struct name
#define PACKED_FIELD(typ,name) typ name PACKED
#define PACKED_END PACKED
#else
#error "PACKED unsupported"
#endif

#ifdef __GNUC__
#define ALIGNED(n,typ,name) typ name __attribute__((aligned(n)))
#else
#error "ALIGNED unsupported"
#endif

#ifdef __GNUC__
#define NO_RETURN __attribute__((noreturn))
#else
#error "NO_RETURN unsupported"
#endif

#define NULL ((void *) 0)

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef u32 physaddr, size_t;
typedef s32 status;

#define BIT(x) ((u32) (1<<(x)))

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({                      \
      const typeof( ((type *)0)->member ) *__mptr = (ptr);      \
      (type *)( (u8 *)__mptr - offsetof(type,member) );})

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
