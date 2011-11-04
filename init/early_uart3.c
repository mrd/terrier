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
#include <stdarg.h>

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

static u32 base10_u32_divisors[10] = {
  1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1
};

void closure_vprintf (void putc_clo (void *, char), void *data, const char *fmt,
                      va_list args)
{
  int precision, width, mode, upper, ells;
  char padding;
#define putc(c) putc_clo(data,c)
  while (*fmt) {
    /* handle ordinary characters and directives */
    switch (*fmt) {
    case '\0':
      return;
    case '%':
      fmt++;
      precision = 0;
      width = 0;
      upper = 1;
      padding = ' ';
      ells = 0;
#define PRINTF_MODE_PRECISION 1
      mode = 0;
      /* handle directive arguments */
      while (*fmt) {
        switch (*fmt) {
        case 'p':{
            /* pointer value */
            u32 x = va_arg (args, u32);
            int i, li;

            for (i = 0; i < 8; i++) {
              if ((li = (x >> ((7 - i) << 2)) & 0x0F) > 9)
                putc ('A' + li - 0x0A);
              else
                putc ('0' + li);
            }
            goto directive_finished;
          }
        case 'x':
          upper = 0;
        case 'X':{
            /* hexadecimal output */
            u64 x;
            int i, li, print_padding = 0, print_digits = 0;
            int w = (ells == 2 ? 16 : 8);

            if (ells == 2)
              x = va_arg (args, u64);
            else
              x = va_arg (args, u32);

            for (i = 0; i < w; i++) {
              li = (x >> (((w - 1) - i) << 2)) & 0x0F;

#define HANDLE_OPTIONS(q,max_w,end_i)                                   \
            if (q != 0 || i == end_i)                                   \
              print_digits = 1;                                         \
            if (q == 0 && !print_digits && i >= (max_w - width))        \
              print_padding = 1;                                        \
            if (q == 0 && !print_digits && i >= (max_w - precision))    \
              print_digits = 1;                                         \
            if (q == 0 && print_padding && !print_digits)               \
              putc(padding);

              HANDLE_OPTIONS (li, w, (w-1));

              if (print_digits) {
                if (li > 9)
                  putc ((upper ? 'A' : 'a') + li - 0x0A);
                else
                  putc ('0' + li);
              }
            }

            goto directive_finished;
          }
        case 'u':{
            /* decimal output */
            u32 x = va_arg (args, u32);
            int i, q, print_padding = 0, print_digits = 0;
            u32 *divisors = base10_u32_divisors;

            for (i = 0; i < 10; i++) {
              q = x / divisors[i];
              x %= divisors[i];

              HANDLE_OPTIONS (q, 10, 9);

              if (print_digits)
                putc ('0' + q);
            }

            goto directive_finished;
          }
        case 'd':{
            /* decimal output */
            signed long x = va_arg (args, signed long);
            int i, q, print_padding = 0, print_digits = 0;
            u32 *divisors = base10_u32_divisors;

            if (x < 0) {
              putc ('-');
              x *= -1;
            }
            for (i = 0; i < 10; i++) {
              q = x / divisors[i];
              x %= divisors[i];

              HANDLE_OPTIONS (q, 10, 9);

              if (print_digits)
                putc ('0' + q);
            }

            goto directive_finished;
          }
        case 's':{
            /* string argument */
            char *s = va_arg (args, char *);
            if (s) {
              if (precision > 0)
                while (*s && precision-- > 0)
                  putc (*s++);
              else
                while (*s)
                  putc (*s++);
            } else {
              putc ('('); putc ('n'); putc ('u'); putc ('l'); putc ('l'); putc (')');
            }
            goto directive_finished;
          }
        case 'c':{
            /* character argument */
            char c = (char) va_arg (args, int);
            /* char is promoted to int when passed through va_arg */
            putc (c);
            goto directive_finished;
          }
        case '%':{
            /* single % */
            putc ('%');
            goto directive_finished;
          }
        case 'l':
          /* "long" annotation */
          ells++;
          break;
        case '.':
          mode = PRINTF_MODE_PRECISION;
          break;
        default:
          if ('0' <= *fmt && *fmt <= '9') {
            if (mode == PRINTF_MODE_PRECISION) {
              /* precision specifier */
              precision *= 10;
              precision += *fmt - '0';
            } else if (mode == 0 && width == 0 && *fmt == '0') {
              /* padding char is zero */
              padding = '0';
            } else {
              /* field width */
              width *= 10;
              width += *fmt - '0';
            }
          }
          break;
        }
        fmt++;
      }
    directive_finished:
      break;
    default:
      /* regular character */
      putc (*fmt);
      break;
    }
    fmt++;
  }
#undef putc
}

static void _putc_uart3(void *x, char c)
{
  putc_uart3(c);
}

void printf_uart3(const char *fmt, ...)
{
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
