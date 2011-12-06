/*
 * OMAP35x Timer subsystem
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
#include "intr/interrupts.h"
#include "omap3/early_uart3.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#define MODULE "timer"
#include "debug/log.h"
#include "debug/cassert.h"

extern region_t l4wakeup_region;
region_t l4perip_region = { 0x49000000, (void *) 0x49000000, &l1pt, 1, 20, 0, 0, R_PM };
region_t l4core_region = { 0x48000000, (void *) 0x48000000, &l1pt, 1, 20, 0, 0, R_PM };

/* 32-kHz sync timer counter */
static volatile u32 *reg_32ksyncnt_cr = (u32 *) 0x48320010;

PACKED_STRUCT(gptimer) {
  PACKED_FIELD(u32, TIDR);
  u8 _rsvd[12];
  PACKED_FIELD(u32, TIOCP_CFG);
  PACKED_FIELD(u32, TISTAT);
  PACKED_FIELD(u32, TISR);
#define GPTIMER_TISR_MAT_IT_FLAG BIT(0) /* Timer match interrupt status */
#define GPTIMER_TISR_OVF_IT_FLAG BIT(1) /* Timer overflow interrupt status */
#define GPTIMER_TISR_TCAR_IT_FLAG BIT(2) /* Timer compare interrupt status */
  PACKED_FIELD(u32, TIER);
#define GPTIMER_TIER_MAT_IT_ENA BIT(0) /* Timer match interrupt enable */
#define GPTIMER_TIER_OVF_IT_ENA BIT(1) /* Timer overflow interrupt enable */
#define GPTIMER_TIER_TCAR_IT_ENA BIT(2) /* Timer compare interrupt enable */
  PACKED_FIELD(u32, TWER);
  PACKED_FIELD(u32, TCLR);
#define GPTIMER_TCLR_ST BIT(0)  /* Start/stop timer */
#define GPTIMER_TCLR_AR BIT(1)  /* Autoreload mode */
#define GPTIMER_TCLR_PRE BIT(5) /* Prescaler enable */
#define GPTIMER_TCLR_CE BIT(6)  /* Compare enable */
  PACKED_FIELD(u32, TCRR);
  PACKED_FIELD(u32, TLDR);
  PACKED_FIELD(u32, TTGR);
  PACKED_FIELD(u32, TWPS);
  PACKED_FIELD(u32, TMAR);
  PACKED_FIELD(u32, TCAR1);
  PACKED_FIELD(u32, TSICR);
  PACKED_FIELD(u32, TCAR2);
  PACKED_FIELD(u32, TPIR);
  PACKED_FIELD(u32, TNIR);
  PACKED_FIELD(u32, TCVR);
  PACKED_FIELD(u32, TOCR);
  PACKED_FIELD(u32, TOWR);
} PACKED_END;
/* CASSERT(offsetof(struct gptimer, TOWR) == 0x58, timer); */

/* GPTIMER1 - GPTIMER11 */
static volatile struct gptimer *gptimer[] = {
  NULL,
  (struct gptimer *) 0x48318000,
  (struct gptimer *) 0x49032000,
  (struct gptimer *) 0x49034000,
  (struct gptimer *) 0x49036000,
  (struct gptimer *) 0x49038000,
  (struct gptimer *) 0x4903A000,
  (struct gptimer *) 0x4903C000,
  (struct gptimer *) 0x4903E000,
  (struct gptimer *) 0x49040000
  //,(struct gptimer *) 0x48086000
  //,(struct gptimer *) 0x48088000
};
#define NUM_TIMERS ((sizeof(gptimer) / sizeof(struct gptimer *)) - 1)

/* GPTIMER1 source is set in CM_CLKSEL_WKUP. */
/* Bit 0 -> 0=32K_FCLK. 1=SYS_CLK. */
/* Bit 1:2 -> 1=div-by-1. 2=div-by-2 */
/* others reserved */
static volatile u32 *CM_CLKSEL_WKUP = (u32 *) 0x48004c40;

u32 timer_32khz_value(void)
{
  return *reg_32ksyncnt_cr;
}

static inline void set_upward_timer(int i, u32 value)
{
  DLOG(1,"setting gptimer[%d]->TCRR=%#x\n",i,value);
  gptimer[i]->TCRR = value;
}

static inline void start_timer(int i)
{
  gptimer[i]->TCLR |= GPTIMER_TCLR_ST;
}

static inline void enable_timer_autoreload(int i)
{
  gptimer[i]->TCLR |= GPTIMER_TCLR_AR;
}

static inline void enable_timer_overflow_interrupt(int i)
{
  gptimer[i]->TIER |= GPTIMER_TIER_OVF_IT_ENA;
}

static inline void stop_timer(int i)
{
  gptimer[i]->TCLR &= ~(GPTIMER_TCLR_ST | GPTIMER_TCLR_AR);
  gptimer[i]->TISR = GPTIMER_TISR_TCAR_IT_FLAG | GPTIMER_TISR_OVF_IT_FLAG | GPTIMER_TISR_MAT_IT_FLAG;
}

static void timer_irq_handler(u32 activeirq)
{
  DLOG(1, "timer_irq_handler(%#x)\n", activeirq);
}

static void timing_loop(void)
{
  u32 start;
  u32 cur;
  u32 cyc_start;
  u32 cyc_cur;

  for(cyc_start = arm_read_cycle_counter(), start = timer_32khz_value(); (cur=timer_32khz_value()) - start < 32768;);
  cyc_cur = arm_read_cycle_counter();
  DLOG(1, "%d cycles after %d timer ticks\n", cyc_cur - cyc_start, cur - start);
}

void timer_init(void)
{
  int i;
  if(vmm_map_region(&l4wakeup_region) != OK) {
    early_panic("Unable to map L4 wakeup registers.");
    return;
  }
  if(vmm_map_region(&l4perip_region) != OK) {
    early_panic("Unable to map L4 peripheral registers.");
    return;
  }
  if(vmm_map_region(&l4core_region) != OK) {
    early_panic("Unable to map L4 core registers.");
    return;
  }

  /* For some reason, this is defaulting to SYS_CLK on my board, even
   * though the manual says otherwise. Set it to 32K_FCLK, div-by-1. */
  *CM_CLKSEL_WKUP = (*CM_CLKSEL_WKUP & (~0x7)) | 0x2;

  for(i=0;i<NUM_TIMERS;i++) {
    intc_set_irq_handler(0x25 + i, timer_irq_handler);
    intc_unmask_irq(0x25 + i);
    DLOG(1, "gptimer[%d] revision=%#x TCLR=%#x\n", i+1, gptimer[i+1]->TIDR, gptimer[i+1]->TCLR);
  }
  timing_loop();
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
