/*
 * Basic UART3 serial port support
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
#include "mem/virtual.h"
#include "arm/memory.h"
#include <stdarg.h>

static region_t uart3_region = { 0x49000000, (void *) 0x49000000, &l1pt, 1, 20, 0, R_PM };

#define NS16650_REG(x) u8 x; u8 _unused_##x[3];

PACKED_STRUCT(ns16650_regs) {
  NS16650_REG(thr);
  NS16650_REG(ier);
  NS16650_REG(fcr);
  NS16650_REG(lcr);
  NS16650_REG(mcr);
  NS16650_REG(lsr);
#define LSR_REG_TX_SR_E   (1<<6)
#define LSR_REG_TX_FIFO_E (1<<5)
  NS16650_REG(msr);
  NS16650_REG(spr);
  NS16650_REG(mdr1);
} PACKED_END;
#define dll thr
#define dlh ier

typedef volatile struct ns16650_regs *ns16650_regs_t;

ns16650_regs_t UART3 = (ns16650_regs_t) 0x49020000;

/***************************************************************************
 * THR_REG (offset 0x0)                                                    *
 *   bits 0-7 THR -- transmit holding register                             *
 *                                                                         *
 *   The transmitter section consists of the transmit holding register     *
 *   (THR_REG) and the transmit shift register. The transmit holding       *
 *   register is a 64-byte FIFO. The MPU writes data to the THR_REG. The   *
 *   data is placed in the transmit shift register where it is shifted out *
 *   serially on the TX output. If the FIFO is disabled, location zero of  *
 *   the FIFO is used to store the data.                                   *
 ***************************************************************************/

/****************************************************************************
 * LSR_REG (offset 0x14)                                                    *
 *   bit 6 TX_SR_E   -- TX hold (FIFO) and shift regs: 0=not empty, 1=empty *
 *   bit 5 TX_FIFO_E -- TX hold (FIFO) reg: 0=not empty, 1=empty            *
 ****************************************************************************/

#define LCRVAL_8N1 0x3
#define LCR_BKSE 0x80           /* bank select enable */

#define LCRVAL LCRVAL_8N1
#define MCRVAL 0x3              /* DTR|RTS */
#define FCRVAL 0x7              /* EN|RX_CLR|TX_CLR */

/* Reset UART3 and configure it for 8N1 115200 operation. */
void reset_uart3(void)
{
  u16 divisor = 0x001A;         /* 115200 baud */

  if(vmm_map_region(&uart3_region) != OK) {
    void early_panic(char *);
    /* This is bad. Disable MMU and attempt to report the problem. */
    arm_ctrl(0, CTRL_MMU | CTRL_DCACHE | CTRL_ICACHE);
    early_panic("Unable to map UART3 region.");
  }

  UART3->ier = 0x00;            /* disable interrupts */

  UART3->mdr1 = 0x7;            /* disable */

  UART3->lcr = LCR_BKSE | LCRVAL;
  UART3->dll = 0;
  UART3->dlh = 0;
  UART3->lcr = LCRVAL;
  UART3->mcr = MCRVAL;
  UART3->fcr = FCRVAL;
  UART3->lcr = LCR_BKSE | LCRVAL;
  UART3->dll = divisor & 0xFF;
  UART3->dlh = (divisor >> 8) & 0xFF;
  UART3->lcr = LCRVAL;

  UART3->mdr1 = 0;              /* UART 16x mode */
}

void print_uart3(const char *s)
{
  while(*s != '\0') { /* Loop until end of string */
    while((UART3->lsr & LSR_REG_TX_SR_E) == 0);
    UART3->thr = (unsigned int)(*s); /* Transmit char */
    s++; /* Next char */
  }
}

void putc_uart3(const char c)
{
  while((UART3->lsr & LSR_REG_TX_SR_E) == 0);
  UART3->thr = (unsigned int) c; /* Transmit char */
}

void putx_uart3(u32 l)
{
  int i, li;

  for (i = 7; i >= 0; i--)
    if ((li = (l >> (i << 2)) & 0x0F) > 9)
      putc_uart3 ('A' + li - 0x0A);
    else
      putc_uart3 ('0' + li);
}

void early_panic(char *msg)
{
  print_uart3("EARLY PANIC\n");
  print_uart3(msg);
  for (;;);
}

int raise(int sig)
{
  early_panic("raise invoked\n");
  return 0;
}

void test(int fmt, ...)
{
  va_list ap;
  int i;

  va_start(ap, fmt);
  for (i = fmt; i >= 0; i = va_arg(ap, int))
    putx_uart3(i);
  va_end(ap);
  putc_uart3('\n');
}

static void _putc_uart3(void *x, char c)
{
  putc_uart3(c);
}

void printf_uart3(const char *fmt, ...)
{
  void closure_vprintf (void putc_clo (void *, char), void *data, const char *fmt,
                        va_list args);
  va_list args;
  va_start(args, fmt);
  closure_vprintf(_putc_uart3, NULL, fmt, args);
  va_end(args);
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
