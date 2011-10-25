volatile unsigned int * const UART3 = (unsigned int *)0x49020000;
#define THR_REG_IDX 0
#define LSR_REG_IDX 5

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
    while ((UART3[LSR_REG_IDX] & (1<<6)) == 0);
    UART3[THR_REG_IDX] = (unsigned int)(*s); /* Transmit char */
    s++; /* Next char */
  }
}
	 
void c_entry() {
  print_uart3("Hello world!\n");
}
