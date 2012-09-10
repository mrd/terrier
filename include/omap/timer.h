/*
 * Timer (GP) support
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

#ifndef _OMAP3_TIMER_H_
#define _OMAP3_TIMER_H_

#include "types.h"
#include "intr/interrupts.h"

#define GPTIMER_BASE_IRQ 0x25

status timer_gp_set(int i, u32 value);
status timer_gp_start(int i);
status timer_gp_enable_autoreload(int i);
status timer_gp_enable_overflow_interrupt(int i);
status timer_gp_disable_overflow_interrupt(int i);
status timer_gp_ack_overflow_interrupt(int i);
status timer_gp_stop(int i);
status timer_gp_set_handler(int i, void (*handler)(u32));

extern volatile u32 *reg_32ksyncnt_cr; /* access to the 32-khz clock value */

/* spin-wait using the 32khz clock: maximum effective wait time (2^17) seconds */
static inline void timer_32k_delay(u32 count)
{
  u32 now = *reg_32ksyncnt_cr, finish = now + count;
  if(finish < now)
    /* wait for roll-over */
    while(*reg_32ksyncnt_cr > finish);
  while(*reg_32ksyncnt_cr < finish);
}


/* Abstract private timer interface for schedulers: */
static UNUSED void pvttimer_set(u32 count);
static UNUSED void pvttimer_enable_interrupt(void);
static UNUSED void pvttimer_disable_interrupt(void);
static UNUSED void pvttimer_ack_interrupt(void);
static UNUSED u32 pvttimer_is_triggered(void);
static UNUSED void pvttimer_start(void);
static UNUSED void pvttimer_stop(void);
static UNUSED void pvttimer_set_handler(void (*handler)(u32));

/* ************************************************** */

#ifdef OMAP4460
/* Cortex-A9 MPCore timers: */

/* Cortex-A9 MPCore Technical Reference Manual 4.2 */
PACKED_STRUCT(pvttimer) {
  PACKED_FIELD(u32, load);
  PACKED_FIELD(u32, counter);
  PACKED_FIELD(u32, control);
  PACKED_FIELD(u32, interrupt_status);
} PACKED_END;

PACKED_STRUCT(watchdog) {
  PACKED_FIELD(u32, load);
  PACKED_FIELD(u32, counter);
  PACKED_FIELD(u32, control);
  PACKED_FIELD(u32, interrupt_status);
  PACKED_FIELD(u32, reset_status);
  PACKED_FIELD(u32, disable);
} PACKED_END;

/* Cortex-A9 MPCore Technical Reference Manual 4.4 */
PACKED_STRUCT(gbltimer) {
  PACKED_FIELD(u32, counter_lo);
  PACKED_FIELD(u32, counter_hi);
  PACKED_FIELD(u32, control);
  PACKED_FIELD(u32, interrupt_status);
  PACKED_FIELD(u32, comparator_lo);
  PACKED_FIELD(u32, comparator_hi);
  PACKED_FIELD(u32, auto_increment);
} PACKED_END;

#define PVTTIMER_INTID 29       /* Cortex-A9 MPCore TRM 4.2.4 */
extern volatile struct pvttimer *PVTTIMER;
extern u32 pvttimer_fastest_count_per_sec, pvttimer_32khz_prescaler;

#endif

/* ************************************************** */

#ifdef OMAP4460
static void pvttimer_set(u32 count)
{
  /* FIXME: look into more efficient way of doing this;
   * and/or support higher clocks-per-sec in interface. */
  PVTTIMER->counter = count * pvttimer_32khz_prescaler;
}

static void pvttimer_enable_interrupt(void)
{
  PVTTIMER->control |= 4;
  intc_unmask_intid(PVTTIMER_INTID);
}

static void pvttimer_disable_interrupt(void)
{
  PVTTIMER->control &= ~4;
}

static void pvttimer_ack_interrupt(void)
{
  PVTTIMER->interrupt_status = 1;
  intc_unmask_intid(PVTTIMER_INTID);
}

static u32 pvttimer_is_triggered(void)
{
  return PVTTIMER->interrupt_status;
}

static void pvttimer_start(void)
{
  PVTTIMER->control |= 1;
}

static void pvttimer_stop(void)
{
  PVTTIMER->control &= ~1;
}

static void pvttimer_set_handler(void (*handler)(u32))
{
  intc_set_intid_handler(PVTTIMER_INTID, handler);
}

#endif
#ifdef OMAP3530
#define PVTTIMER_GPTIMER 1      /* use GPTIMER1 to implement "private" timer */
static void pvttimer_set(u32 count)
{
  timer_gp_set(PVTTIMER_GPTIMER, -count);
}

static void pvttimer_enable_interrupt(void)
{
  timer_gp_enable_overflow_interrupt(PVTTIMER_GPTIMER);
}

static void pvttimer_disable_interrupt(void)
{
  timer_gp_disable_overflow_interrupt(PVTTIMER_GPTIMER);
}

static void pvttimer_ack_interrupt(void)
{
  timer_gp_ack_overflow_interrupt(PVTTIMER_GPTIMER);
  intc_unmask_irq(GPTIMER_BASE_IRQ + (PVTTIMER_GPTIMER - 1));
}

static u32 pvttimer_is_triggered(void)
{
  return timer_gp_is_overflow_interrupt(PVTTIMER_GPTIMER);
}

static void pvttimer_start(void)
{
  timer_gp_start(PVTTIMER_GPTIMER);
}

static void pvttimer_stop(void)
{
  timer_gp_stop(PVTTIMER_GPTIMER);
}

static void pvttimer_set_handler(void (*handler)(u32))
{
  timer_gp_set_handler(PVTTIMER_GPTIMER, handler);
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
