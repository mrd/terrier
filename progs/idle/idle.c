int main(void)
{
  /* code that uses a lot of registers -- and gets messed up if registers are corrupted */
  asm volatile("MOV r0, #1\n MOV r1, #11\n MOV r2, #13\n MOV r4, #0":::"r0","r1","r2","r4");
  for(;;) {
    asm volatile("SVC #1");
    asm volatile("1: ADD r4, r4, #1\n"
                 "   ADD r6, r1, r2\n"
                 "   MOV r5, r0, LSL r6\n"
                 "   CMP r5, r4\n"
                 "   BNE 1b\n"
                 "   MOV r0, #1\n"
                 "   MOV r1, #11\n"
                 "   MOV r2, #13\n"
                 "   MOV r4, #0\n":::"r0","r1","r2","r3","r4","r5","r6");

  }
  return 0;
}
