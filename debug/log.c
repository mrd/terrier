/*
 * Debug logging support
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
#include "omap/early_uart3.h"
#include "arm/asm.h"
#include <stdarg.h>

static int current_debug_level = 3;

static u32 base10_u32_divisors[10] = {
  1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1
};

void closure_vprintf (void putc_clo (void *, char), void *data, const char *fmt,
                      va_list args)
{
  int precision, width, mode,
    upper,                      /* upper-case */
    ells,                       /* number of 'l's (long) */
    alt;                        /* "alternate form" */
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
      alt = 0;
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

            if (alt) {
              putc ('0');
              putc ('x');
            }

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
        case '#':
          alt = 1;
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
  if(c == '\n')
    putc_uart3('\r');
  putc_uart3(c);
}

void debuglog(const char *src, int lvl, const char *fmt, ...)
{
  if(lvl <= current_debug_level) {
    va_list args;
    va_start(args, fmt);
    printf_uart3("%.8x: ", arm_read_cycle_counter());
    print_uart3(src); print_uart3(": ");
    closure_vprintf(_putc_uart3, NULL, fmt, args);
    va_end(args);
  }
}

void debuglog_no_prefix(int lvl, const char *fmt, ...)
{
  if(lvl <= current_debug_level) {
    va_list args;
    va_start(args, fmt);
    closure_vprintf(_putc_uart3, NULL, fmt, args);
    va_end(args);
  }
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
