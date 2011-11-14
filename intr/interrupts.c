/*
 * Interrupt handling (C portion)
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
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#define MODULE "intr"
#include "debug/log.h"

region_t intr_region = { 0x89000000, (void *) 0xFFF00000, &l1pt, 1, 20, R_C | R_B, R_PM };
void intr_init(void)
{
  extern u32 _vector_table_size;
  int i; u32 *vt = (u32 *) 0xFFFF0000;
  extern void *_vector_table;
  u32 *init_vt = (u32 *) &_vector_table;

  if(vmm_map_region(&intr_region) != OK) {
    early_panic("Unable to map interrupt vector table.");
    return;
  }

  for(i=0;i < _vector_table_size;i++)
    vt[i] = init_vt[i];
  for(i=0;i < _vector_table_size;i++)
    printf_uart3("%#x ", vt[i]);
  putc_uart3('\n');
  /* set vector table to 0xFFFF0000 */
  arm_mmu_ctrl(MMU_CTRL_HIGHVT, MMU_CTRL_HIGHVT);
}

void _handle_reset(void)
{
  DLOG(1, "_handle_reset\n");
}

void _handle_undefined_instruction(void)
{
  DLOG(1, "_handle_undefined_instruction\n");
}

void _handle_swi(void)
{
  DLOG(1, "_handle_swi\n");
}

void _handle_prefetch_abort(void)
{
  DLOG(1, "_handle_prefetch_abort\n");
}

void _handle_data_abort(void)
{
  DLOG(1, "_handle_data_abort\n");
}

void _handle_irq(void)
{
  DLOG(1, "_handle_irq\n");
}

void _handle_fiq(void)
{
  DLOG(1, "_handle_fiq\n");
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
