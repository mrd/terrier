#include<user.h>

unsigned int _scheduler_capacity = 3 << 14;
unsigned int _scheduler_period = 12 << 14;
unsigned int _scheduler_affinity = 1;

static inline void svc3(char *ptr, u32 len, u32 noprefix)
{
  register char *r0 asm("r0") = ptr;
  register u32 r1 asm("r1") = len;
  register u32 r2 asm("r2") = noprefix;
  asm volatile("SVC #3"::"r"(r0),"r"(r1),"r"(r2));
}

int main(void)
{
  svc3("Test\n", 6, 0);

  /* code that uses a lot of registers -- and gets messed up if registers are corrupted */
  asm volatile("MOV r0, #1\n MOV r1, #11\n MOV r2, #13\n MOV r4, #0":::"r0","r1","r2","r4");
  for(;;) {
    asm volatile("SVC #2");
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
