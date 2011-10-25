/*
 * Begin initialization in C.
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

volatile unsigned int * const UART3 = (unsigned int *)0x49020000;
#define THR_REG_IDX 0
#define LSR_REG_IDX 5
#define LSR_REG_TX_SR_E   (1<<6)
#define LSR_REG_TX_FIFO_E (1<<5)

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

void print_uart3(const char *s) {
  while(*s != '\0') { /* Loop until end of string */
    while ((UART3[LSR_REG_IDX] & LSR_REG_TX_SR_E) == 0);
    UART3[THR_REG_IDX] = (unsigned int)(*s); /* Transmit char */
    s++; /* Next char */
  }
}

void c_entry() {
  print_uart3("Hello world!\n");
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
