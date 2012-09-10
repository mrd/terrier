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
#include "omap/early_uart3.h"
#include "omap/timer.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#define MODULE "timer"
#include "debug/log.h"
#include "debug/cassert.h"

#ifdef USE_VMM
#ifdef OMAP3530
extern region_t l4wakeup_region; /* 0x48300000 */
region_t l4perip_region = { 0x49000000, (void *) 0x49000000, &l1pt, 1, 20, 0, 0, R_PM };
region_t l4core_region = { 0x48000000, (void *) 0x48000000, &l1pt, 1, 20, 0, 0, R_PM };
#endif
#ifdef OMAP4460
region_t l4perip_region = { 0x48000000, (void *) 0x48000000, &l1pt, 1, 20, 0, 0, R_PM };
region_t l4abe_region = { 0x49000000, (void *) 0x49000000, &l1pt, 1, 20, 0, 0, R_PM };
static region_t l4wakeup_region = { 0x4A300000, (void *) 0x4A300000, &l1pt, 1, 20, 0, 0, R_PM };
region_t timer_region = { 0, 0, &l1pt, 1, 20, 0, 0, R_PM }; /* fill in later */
#endif
#endif

/* 32-kHz sync timer counter */
#ifdef OMAP3530
volatile u32 *reg_32ksyncnt_cr = (u32 *) 0x48320010;
#endif
#ifdef OMAP4460
volatile u32 *reg_32ksyncnt_cr = (u32 *) 0x4A304010;
#endif

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
#ifdef OMAP3530
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
#endif
#ifdef OMAP4460
  (struct gptimer *) 0x4A318000,
  (struct gptimer *) 0x48032000,
  /* // GPTIMERS 1,2,11 are different from the others. Sigh.
  (struct gptimer *) 0x48034000,
  (struct gptimer *) 0x48036000,
  (struct gptimer *) 0x49038000,
  (struct gptimer *) 0x4903A000,
  (struct gptimer *) 0x4903C000,
  (struct gptimer *) 0x4903E000,
  (struct gptimer *) 0x4803E000,
  (struct gptimer *) 0x48086000,
  (struct gptimer *) 0x48088000
  */
#endif
};
#define NUM_TIMERS ((sizeof(gptimer) / sizeof(struct gptimer *)) - 1)

#ifdef OMAP3530
/* GPTIMER1 source is set in CM_CLKSEL_WKUP. */
/* Bit 0 -> 0=32K_FCLK. 1=SYS_CLK. */
/* Bit 1:2 -> 1=div-by-1. 2=div-by-2 */
/* others reserved */
static volatile u32 *CM_CLKSEL_WKUP = (u32 *) 0x48004c40;
#endif
#ifdef OMAP4460
/* Table 3-785. bit 24: 0 => SYS_CLK; 1 => 32kHz */
static volatile u32 *CM_WKUP_GPTIMER1_CLKCTRL = (u32 *) 0x4A307840;

volatile struct pvttimer *PVTTIMER;
volatile struct watchdog *WATCHDOG;
volatile struct gbltimer *GBLTIMER;
#endif


u32 timer_32khz_value(void)
{
  return *reg_32ksyncnt_cr;
}

static inline void set_upward_timer(int i, u32 value)
{
  DLOG(1,"setting gptimer[%d]->TCRR=%#x\n",i,value);
  gptimer[i]->TCRR = value;
}

status timer_gp_set(int i, u32 value)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  set_upward_timer(i, value);
  return OK;
}

static inline void start_timer(int i)
{
  gptimer[i]->TCLR |= GPTIMER_TCLR_ST;
}

status timer_gp_start(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  start_timer(i);
  return OK;
}

static inline void enable_timer_autoreload(int i)
{
  gptimer[i]->TCLR |= GPTIMER_TCLR_AR;
}

status timer_gp_enable_autoreload(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  enable_timer_autoreload(i);
  return OK;
}

static inline void enable_timer_overflow_interrupt(int i)
{
  gptimer[i]->TIER |= GPTIMER_TIER_OVF_IT_ENA;
}

status timer_gp_enable_overflow_interrupt(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  enable_timer_overflow_interrupt(i);
  return OK;
}

static inline void disable_timer_overflow_interrupt(int i)
{
  gptimer[i]->TIER &= ~GPTIMER_TIER_OVF_IT_ENA;
}

status timer_gp_disable_overflow_interrupt(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  disable_timer_overflow_interrupt(i);
  return OK;
}

static inline void ack_timer_overflow_interrupt(int i)
{
  gptimer[i]->TISR = 2;
  gptimer[i]->TCLR |= GPTIMER_TCLR_ST; /* auto-disables upon overflow */
}

status timer_gp_ack_overflow_interrupt(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  ack_timer_overflow_interrupt(i);
  return OK;
}

static inline u32 is_timer_overflow_interrupt(int i)
{
  return GETBITS(gptimer[i]->TISR, 1, 1);
}

u32 timer_gp_is_overflow_interrupt(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  return is_timer_overflow_interrupt(i);
}

static inline void stop_timer(int i)
{
  gptimer[i]->TCLR &= ~(GPTIMER_TCLR_ST | GPTIMER_TCLR_AR);
  gptimer[i]->TISR = GPTIMER_TISR_TCAR_IT_FLAG | GPTIMER_TISR_OVF_IT_FLAG | GPTIMER_TISR_MAT_IT_FLAG;
}

status timer_gp_stop(int i)
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  stop_timer(i);
  return OK;
}

static void timer_irq_handler(u32 activeirq)
{
  DLOG(1, "timer_irq_handler(%#x)\n", activeirq);
}

status timer_gp_set_handler(int i, void (*handler)(u32))
{
  if(i == 0 || i > NUM_TIMERS) return EINVALID;
  intc_set_irq_handler(0x25 + i - 1, handler);
  return OK;
}

/* ************************************************** */

/* Measure timers against 32kHz clock */
u32 pvttimer_fastest_count_per_sec, pvttimer_32khz_prescaler;
static void timing_loop(void)
{
  u32 start;
  u32 cur;
  u32 cyc_start;
  u32 cyc_cur;
  u32 pvttimer_cur;

  PVTTIMER->counter = 0xFFFFFFFF; PVTTIMER->control = 1; /* prescaler = 0 */
  for(cyc_start = arm_read_cycle_counter(), start = timer_32khz_value(); (cur=timer_32khz_value()) - start < 32768;);
  PVTTIMER->control &= ~1; pvttimer_cur = PVTTIMER->counter;
  cyc_cur = arm_read_cycle_counter();
  DLOG(1, "%d cycles after %d timer ticks\n", cyc_cur - cyc_start, cur - start);
  pvttimer_fastest_count_per_sec = 0xFFFFFFFF - pvttimer_cur;
  DLOG(1, "PVTTIMER: count decremented %d times after %d timer ticks\n", pvttimer_fastest_count_per_sec, cur - start);
  pvttimer_32khz_prescaler = pvttimer_fastest_count_per_sec >> 15;
  DLOG(1, "PVTTIMER: counts %d times per tick.\n", pvttimer_32khz_prescaler);
}

void timer_init(void)
{
  int i;
#ifdef USE_VMM
  if(vmm_map_region(&l4wakeup_region) != OK) {
    early_panic("Unable to map L4 wakeup registers.");
    return;
  }
  if(vmm_map_region(&l4perip_region) != OK) {
    early_panic("Unable to map L4 peripheral registers.");
    return;
  }
#ifdef OMAP3530
  if(vmm_map_region(&l4core_region) != OK) {
    early_panic("Unable to map L4 core registers.");
    return;
  }
#endif
#ifdef OMAP4460
  if(vmm_map_region(&l4abe_region) != OK) {
    early_panic("Unable to map L4 ABE registers.");
    return;
  }
#endif
#endif

#ifdef OMAP3530
  /* For some reason, this is defaulting to SYS_CLK on my board, even
   * though the manual says otherwise. Set it to 32K_FCLK, div-by-1. */
  *CM_CLKSEL_WKUP = (*CM_CLKSEL_WKUP & (~0x7)) | 0x2;
#endif
#ifdef OMAP4460
  DLOG(1, "CM_WKUP_GPTIMER1_CLKCTRL=%#x (%s)\n", *CM_WKUP_GPTIMER1_CLKCTRL, GETBITS(*CM_WKUP_GPTIMER1_CLKCTRL, 24, 1) ? "32kHz" : "SYS_CLK");
#endif

#ifdef OMAP4460
  GBLTIMER = (void *) (arm_config_base_address() + 0x200);
  PVTTIMER = (void *) (arm_config_base_address() + 0x600);
  WATCHDOG = (void *) (arm_config_base_address() + 0x620);
#ifdef USE_VMM
  timer_region.pstart = ((u32) PVTTIMER) & 0xFFF00000;
  timer_region.vstart = (void *) (((u32) PVTTIMER) & 0xFFF00000);
  /* ensure mapping of private/global timers */
  if(vmm_map_region(&timer_region) != OK) {
    early_panic("Unable to map private/global timers.");
    return;
  }
#endif
  PVTTIMER->control = 0;
  WATCHDOG->control = 0;
  GBLTIMER->control = 0;
#endif

  for(i=0;i<NUM_TIMERS;i++) {
    intc_set_irq_handler(GPTIMER_BASE_IRQ + i, timer_irq_handler);
    intc_unmask_irq(GPTIMER_BASE_IRQ + i);
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
