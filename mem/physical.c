/*
 * Physical memory manager
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

#include "types.h"
#include "omap3/early_uart3.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#define MODULE "pmm"
#include "debug/log.h"

#define DEFAULT_CS0_START 0x80000000
#define BITMAP_VIRTADDR ((void *) 0xC0000000)

static region_t sdrc_region = { 0x6D000000, (void *) 0x6D000000, &l1pt, 1, 20, 0, 0, R_PM };

typedef struct {
  u32 *map;
  u32 map_bytes;
  physaddr start;
} bitmap_t;

static bitmap_t bitmap[2];
static u32 num_bitmaps;

#define BITMAP_SET(table,index) ((table)[(index)>>5] |= (1 << ((index) & 31)))
#define BITMAP_CLR(table,index) ((table)[(index)>>5] &= ~(1 << ((index) & 31)))
#define BITMAP_TST(table,index) ((table)[(index)>>5] & (1 << ((index) & 31)))

/* Pick and mark a physical page as allocated. Zero on error. */
physaddr physical_alloc_page(void)
{
  int i,j,k;
  for(i=0; i<num_bitmaps; i++)
    for(j=0; j<bitmap[i].map_bytes; j++)
      if(bitmap[i].map[j] != 0xFF)
        for(k=0;k<8;k++)
          if(!BITMAP_TST(bitmap[i].map, (j<<3) + k)) {
            BITMAP_SET(bitmap[i].map, (j<<3) + k);
            return bitmap[i].start + (((j<<3) + k) << PAGE_SIZE_LOG2);
          }
  return 0;
}

/* Unmark physical page given by address (assume aligned) */
void physical_free_page(physaddr addr)
{
  int i;
  for(i=num_bitmaps-1; i>=0; --i) {
    if(addr >= bitmap[i].start) {
      BITMAP_CLR(bitmap[i].map, (addr - bitmap[i].start) >> PAGE_SIZE_LOG2);
    }
  }
}

void physical_init(void)
{
  if(vmm_map_region(&sdrc_region) != OK) {
    DLOG(1, "Unable to map sdrc_region.\n");
    return;
  }
  extern void *_physical_start, *_kernel_pages;
  volatile u32 *SDRC_MCFG_0 = (u32 *) 0x6D000080;
  volatile u32 *SDRC_MCFG_1 = (u32 *) 0x6D0000B0;
  volatile u32 *SDRC_CS_CFG = (u32 *) 0x6D000040;
  u32 cs0_ramsize = ((*SDRC_MCFG_0 >> 8) & 0x3FF) << 1;
  u32 cs1_ramsize = ((*SDRC_MCFG_1 >> 8) & 0x3FF) << 1;
  u32 cs1_start   = (((*SDRC_CS_CFG >> 8) & 0x3) | ((*SDRC_CS_CFG & 0xF) << 2)) * 0x2000000 + DEFAULT_CS0_START;
  u32 cs0_start   = DEFAULT_CS0_START;
  u32 kern_pages  = (u32) &_kernel_pages;
  u32 kp_start    = (u32) &_physical_start;
  u32 bitmap0_len = cs0_ramsize << (20 - PAGE_SIZE_LOG2 - 3);
  u32 bitmap1_len = cs1_ramsize << (20 - PAGE_SIZE_LOG2 - 3);
  u32 i;

  bitmap[0].map = (u32 *) BITMAP_VIRTADDR;
  bitmap[0].map_bytes = bitmap0_len;
  bitmap[0].start = cs0_start;

  if(cs1_ramsize) {
    num_bitmaps = 2;
    bitmap[1].map = (void *) (BITMAP_VIRTADDR + bitmap0_len);
    bitmap[1].map_bytes = bitmap1_len;
    bitmap[1].start = cs1_start;
  } else {
    num_bitmaps = 1;
    bitmap[1].map = NULL;
  }

  /* clear bitmap0: bitmap0_len is a power of 2 */
  arm_memset_log2(bitmap[0].map, 0, 31 - count_leading_zeroes(bitmap[0].map_bytes));
  if(bitmap[1].map)
    /* clear bitmap: bitmap1_len is a power of 2 */
    arm_memset_log2(bitmap[1].map, 0, 31 - count_leading_zeroes(bitmap[1].map_bytes));

  /* mark space used by bitmap0 */
  for(i=0; i < (bitmap[0].map_bytes >> PAGE_SIZE_LOG2); i++)
    BITMAP_SET(bitmap[0].map, i + (((u32) BITMAP_VIRTADDR - (u32) bitmap[0].map) >> PAGE_SIZE_LOG2));

  if(bitmap[1].map)
    /* mark space used by bitmap1 */
    for(i=0; i < (bitmap1_len >> PAGE_SIZE_LOG2); i++)
      BITMAP_SET(bitmap[0].map, i + (((u32) BITMAP_VIRTADDR + bitmap[0].map_bytes - (u32) bitmap[0].map) >> PAGE_SIZE_LOG2));

  /* mark space used by kernel */
  for(i=0; i < kern_pages; i++)
    BITMAP_SET(bitmap[0].map, ((kp_start - bitmap[0].start) >> PAGE_SIZE_LOG2) + i);

  DLOG(1, "physical memory bitmap: ");
  for(i=0;i<64;i++)
    if(BITMAP_TST(bitmap[0].map,i))
      debuglog_no_prefix(1, "X");
    else
      debuglog_no_prefix(1, ".");
  debuglog_no_prefix(1, "\n");
}

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
