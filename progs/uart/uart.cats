#include "ats_types.h"
#include "ats_basics.h"
#include<user.h>

/* unsigned int _scheduler_capacity = 3 << 14;
 * unsigned int _scheduler_period = 12 << 14; */
unsigned int _scheduler_capacity = 1 << 14;
unsigned int _scheduler_period = 2 << 14;
unsigned int _scheduler_affinity = 2;

static inline void svc3(char *ptr, u32 len, u32 noprefix)
{
  register char *r0 asm("r0") = ptr;
  register u32 r1 asm("r1") = len;
  register u32 r2 asm("r2") = noprefix;
  asm volatile("SVC #3"::"r"(r0),"r"(r1),"r"(r2));
}

void *atspre_null_ptr = 0;      //FIXME

ats_bool_type atspre_pgt(const ats_ptr_type p1, const ats_ptr_type p2) {
  return (p1 > p2) ;
}
ats_bool_type atspre_ieq(ats_int_type a, ats_int_type b)
{
  return a == b;
}

#define atspre_mul_bool1_bool1 atspre_mul_bool_bool
ats_bool_type atspre_mul_bool_bool (ats_bool_type b1, ats_bool_type b2) {
  if (b1) { return b2 ; } else { return ats_false_bool ; }
}

#define atspre_ineq atspre_neq_int_int
ats_bool_type atspre_neq_int_int (ats_int_type i1, ats_int_type i2) {
  return (i1 != i2);
}
ats_bool_type atspre_eq_int_int (ats_int_type i1, ats_int_type i2) {
  return (i1 == i2);
}
ats_int_type atspre_add_int_int (ats_int_type i1, ats_int_type i2) {
  return (i1 + i2);
}


void mydelay(void)
{
  int i;
  for(i=0;i<100000000;i++) asm volatile("mov r0,r0");
}

void debuglog(u32 noprefix, u32 lvl, const char *fmt, ...)
{
#define DLOG_BUFLEN 256
  static char buffer[DLOG_BUFLEN];
  va_list args;
  va_start(args, fmt);

  vsnprintf(buffer, DLOG_BUFLEN, fmt, args);

  va_end(args);

  buffer[DLOG_BUFLEN - 1] = 0;
  svc3(buffer, DLOG_BUFLEN - 1, noprefix);
#undef DLOG_BUFLEN
}

#define DLOG(lvl,fmt,...) debuglog(0,lvl,fmt,##__VA_ARGS__)
#define DLOG_NO_PREFIX(lvl,fmt,...) debuglog(1,lvl,fmt,##__VA_ARGS__)

void printnum(int i)
{
  DLOG(1, "uart num=%d\n", i);
}

ipcmapping_t _ipcmappings[] = {
  { IPC_OFFER, "uart", IPC_READ, "fourslot2w", 1, NULL },
  {0}
};


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
