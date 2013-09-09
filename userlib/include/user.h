/*
 * "Userspace" library header file
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2013 Matthew Danish.
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

#ifndef __BUILTIN__USER_LIB_H__
#define __BUILTIN__USER_LIB_H__

#include<stdarg.h>
#include"../../include/types.h"
#include"../../include/status.h"
#include"../../include/arm/memory-basics.h"

PACKED_STRUCT(mapping) {
  physaddr pstart;              /* starting physical address of region */
  void *vstart;                 /* starting virtual address of region */
  u32 page_count;               /* number of pages in this region */
  u16 page_size_log2;           /* size of pages in this region (log2) */
#define R_B 1                   /* set "buffered" bit */
#define R_C 2                   /* set "cached" bit */
  u8 cache_buf:4;               /* cache/buffer attributes */
#define R_S 1                   /* set "shared" bit */
#define R_NG 2                  /* set "not-global" bit */
  u8 shared_ng:4;               /* shared/not-global attributes */
#define R_NA 0                  /* no access (besides use of System/ROM bits) */
#define R_PM 1                  /* privileged mode only */
#define R_RO 2                  /* user-mode can read-only */
#define R_RW 3                  /* user-mode can read-write */
  u8 access;                    /* access permission attributes */
  char *desc;                   /* description of mapping */
} PACKED_END;
typedef struct mapping mapping_t;

typedef struct {
#define IPC_SEEK 1
#define IPC_OFFER 2
#define IPC_MULTIOFFER 3
  u32 type;
  char *name;
#define IPC_READ 1
#define IPC_WRITE 2
  u32 flags;
  char *proto;
  u32 pages;
  void *address;
} ipcmapping_t;

int snprintf(char *str, int size, const char *format, ...);
int vsnprintf(char *str, int size, const char *format, va_list ap);

static inline void *memcpy(void *dest, void *src, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  u8 *src8 = (u8 *) src;
  for(i=0;i<count;i++)
    dest8[i] = src8[i];
  return dest;
}

static inline void *memset(void *dest, u32 byte, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  for(i=0;i<count;i++)
    dest8[i] = (u8) byte;
  return dest;
}

#define _DEVICE_SECTION ".device"
#define DEVICE_ATTR __attribute__((section(_DEVICE_SECTION)))

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
