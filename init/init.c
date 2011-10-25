volatile unsigned int * const UART3 = (unsigned int *)0x49020000;
	 
void print_uart3(const char *s) {
  while(*s != '\0') { /* Loop until end of string */
    /* THR_REG offset = 0 */
    *UART3 = (unsigned int)(*s); /* Transmit char */
    s++; /* Next char */
  }
}
	 
void c_entry() {
  print_uart3("Hello world!\n");
}
