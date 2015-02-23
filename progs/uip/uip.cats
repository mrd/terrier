#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "pats_ccomp_typedefs.h"
#include<user.h>
//#include<pool.h>
#include "pointer.cats"
#include "integer.cats"
#include "virtual.h"
#include "uip/clock-arch.h"
#include "uip/uip.h"
#include "uip/uip_arp.h"
#include "uip/timer.h"



/* unsigned int _scheduler_capacity = 3 << 14;
 * unsigned int _scheduler_period = 12 << 14; */
unsigned int _scheduler_capacity = 20 << 1;
unsigned int _scheduler_period = 20 << 1;
unsigned int _scheduler_affinity = 1;

ipcmapping_t _ipcmappings[] = {
  { IPC_OFFER, "uip-in", IPC_READ, "fixedslot", 4, NULL },
  { IPC_SEEK, "uip-out", IPC_WRITE, "fixedslot", 4, NULL },
  {0}
};

#define OMAP4460 // FIXME: assume OMAP4460 for now

/* 32-kHz sync timer counter */
#ifdef OMAP3530
#define TIMER_ADDR 0x48320010
#endif
#ifdef OMAP4460
#define TIMER_ADDR 0x4A304010
#endif

mapping_t _mappings[] = {
  { (TIMER_ADDR & (~0xFFF)), NULL, 1, 12, 0, 0, R_RW, "32-kHz sync timer" },
  { 0 }
};

typedef struct { char s[124]; } buf_t;

static inline void svc3(char *ptr, u32 len, u32 noprefix)
{
  register char *r0 asm("r0") = ptr;
  register u32 r1 asm("r1") = len;
  register u32 r2 asm("r2") = noprefix;
  asm volatile("SVC #3"::"r"(r0),"r"(r1),"r"(r2));
}

void *atspre_null_ptr = 0;      //FIXME

void mydelay(void)
{
  int i;
  for(i=0;i<100000000;i++) asm volatile("mov r0,r0");
}

void debuglog(const u32 noprefix, u32 lvl, const char *fmt, ...)
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
  DLOG(1, "uip num=%d\n", i);
}

void printbuf(buf_t b)
{
  DLOG(1, "uip: %s\n", b.s);
}

/* ************************************************** */

#define make_VendorRequest(x) x
static inline u32 swap_endian32(u32 x)
{
  u8 *data = (u8 *) &x;
  u32 result = ((u32) (data[3]<<0)) | ((u32) (data[2]<<8)) | ((u32) (data[1]<<16)) | ((u32) (data[0]<<24));
  return result;
}

static inline u32 get4bytes_le(u8 *data)
{
  u32 result = ((u32) (data[0]<<0)) | ((u32) (data[1]<<8)) | ((u32) (data[2]<<16)) | ((u32) (data[3]<<24));
  return result;
}

static inline void dump_buf(u8 *buf, int n)
{
  int i,j;
  DLOG(1, "dump_buf (%#.8x, %d):\n", buf, n);
  for(i=0;;i++) {
    for(j=0;j<8;j++) {
      if(8*i+j >= n) { DLOG_NO_PREFIX(1, "\n"); return; }
      DLOG_NO_PREFIX(1, " %.2x", buf[8*i+j]);
    }
    DLOG_NO_PREFIX(1, "\n");
  }
}

extern volatile unsigned int *reg_32ksyncnt_cr;

static inline void msleep(int msec)
{
  u32 now = *reg_32ksyncnt_cr;
  u32 then = now + (msec << 5); // a msec is approx 32-multiplier
  for(;*reg_32ksyncnt_cr < then;) ASM("MOV r0,r0");
}

int counterv = 0;
#define counter() counterv++

// UIP support
void copy_into_uip_buf(u8 *buf, u32 len)
{
  // buf layout in words: [itag, otag, pkt size, data...]
  memcpy(uip_buf, buf + 12, len);
  uip_len = len;
}

u32 copy_from_uip_buf(void *buf)
{
  // buf layout in words: [itag, otag, pkt size, data...]
  memcpy(((u8 *) buf) + 12, uip_buf, uip_len);
  return uip_len;
}

static inline void uip_eth_addr(struct uip_eth_addr *eaddr, int a, int b, int c, int d, int e, int f)
{
  eaddr->addr[0] = a;
  eaddr->addr[1] = b;
  eaddr->addr[2] = c;
  eaddr->addr[3] = d;
  eaddr->addr[4] = e;
  eaddr->addr[5] = f;
}

static inline void uip_eth_addr_array(struct uip_eth_addr *eaddr, u8 *array)
{
  eaddr->addr[0] = array[0];
  eaddr->addr[1] = array[1];
  eaddr->addr[2] = array[2];
  eaddr->addr[3] = array[3];
  eaddr->addr[4] = array[4];
  eaddr->addr[5] = array[5];
}

#define get_uip_len() uip_len
#define get_uip_buftype() (((struct uip_eth_hdr *)&uip_buf[0])->type)


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
