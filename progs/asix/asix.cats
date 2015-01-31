#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "pats_ccomp_typedefs.h"
#include<user.h>
//#include<pool.h>
#include "pointer.cats"
#include "integer.cats"
#include "virtual.h"
//#include "uip/clock-arch.h"
//#include "uip/uip.h"
//#include "uip/uip_arp.h"
//#include "uip/timer.h"



/* unsigned int _scheduler_capacity = 3 << 14;
 * unsigned int _scheduler_period = 12 << 14; */
unsigned int _scheduler_capacity = 2 << 7;
unsigned int _scheduler_period = 4 << 7;
unsigned int _scheduler_affinity = 1;

ipcmapping_t _ipcmappings[] = {
  { IPC_SEEK, "ehci_info", IPC_READ, "fixedslot", 1, NULL },
  { IPC_SEEK, "ehci_usbdevices", IPC_READ | IPC_WRITE, "usbdevices", 1, NULL },
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

/* ************************************************** */

void dump_indent(indent)
{
  u32 i;
  for(i=0;i<indent;i++) DLOG_NO_PREFIX(1, " ");
}

void dump_qh(ehci_qh_t *qh, u32 indent)
{
#ifdef USE_VMM
  /* FIXME: physical->virtual */
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "QH: %#x %#x %#x %#x %#x %#x\n",
                 qh->next,
                 qh->characteristics,
                 qh->capabilities,
                 qh->current_td,
                 qh->next_td,
                 qh->alt_td);

#else
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "QH: %#x %#x %#x %#x %#x %#x %#x %d\n",
                 qh->next,
                 qh->characteristics,
                 qh->capabilities,
                 qh->current_td,
                 qh->next_td,
                 qh->alt_td,
                 qh->overlay[0],
                 GETBITS(qh->overlay[0],31,1));
  if(!(qh->current_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->current_td & EHCI_TD_PTR_MASK), indent+2);
  else if(!(qh->next_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->next_td & EHCI_TD_PTR_MASK), indent+2);
  else if(!(qh->alt_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->alt_td & EHCI_TD_PTR_MASK), indent+2);

  // need to handle circular
  //if(!(qh->next & EHCI_QH_PTR_T))
  //  dump_qh((ehci_qh_t *) (qh->next & EHCI_QH_PTR_MASK), indent);
#endif
}

static inline void dump_usb_dev_qh(usb_device_t *usbd)
{
  dump_qh(&QH(*usbd,0), 0);
}

static inline void dump_usb_dev_desc(usb_dev_desc_t *d)
{
  DLOG(1, "asix USB_DEV_DESC:\n");
  DLOG_NO_PREFIX(1, "  bLength=%d bDescriptorType=%d bcdUSB=%d.%d\n",
                 d->bLength, d->bDescriptorType, GETBITS(d->bcdUSB, 8, 8), GETBITS(d->bcdUSB, 0, 8));
  DLOG_NO_PREFIX(1, "  bDeviceClass=%d bDeviceSubClass=%d bDeviceProtocol=%d\n",
                 d->bDeviceClass, d->bDeviceSubClass, d->bDeviceProtocol);
  DLOG_NO_PREFIX(1, "  bMaxPacketSize0=%d idVendor=%#x idProduct=%#x\n",
                 d->bMaxPacketSize0, d->idVendor, d->idProduct);
  DLOG_NO_PREFIX(1, "  bcdDevice=%d.%d iManufacturer=%d iProduct=%d\n",
                 GETBITS(d->bcdDevice, 8, 8), GETBITS(d->bcdDevice, 0, 8), d->iManufacturer, d->iProduct);
  DLOG_NO_PREFIX(1, "  bNumConfigurations=%d\n", d->bNumConfigurations);
}

void dump_usb_ept_desc(usb_ept_desc_t *d)
{
  char *tt, *st = "", *ut = "";
  u32 ttn = GETBITS(d->bmAttributes, 0, 2);
  if(ttn == 1) {
    tt = "(isochronous) ";
    u32 stn = GETBITS(d->bmAttributes, 2, 2);
    u32 utn = GETBITS(d->bmAttributes, 4, 2);

    if(stn == 0) st = "(no-sync) ";
    else if(stn == 1) st = "(async) ";
    else if(stn == 2) st = "(adaptive) ";
    else st = "(sync) ";

    if(utn == 0) ut = "(data) ";
    else if(utn == 1) ut = "(feedback) ";
    else if(utn == 2) ut = "(implicit-feedback-data) ";
    else ut = "(reserved) ";
  } else if(ttn == 0)
    tt = "(control) ";
  else if(ttn == 2)
    tt = "(bulk) ";
  else
    tt = "(interrupt) ";

  DLOG(1, "USB_EPT_DESC:\n");
  DLOG_NO_PREFIX(1, "  bLength=%d bDescriptorType=%d bEndpointAddress=%d (%s)\n",
                 d->bLength, d->bDescriptorType, GETBITS(d->bEndpointAddress, 0, 4),
                 (d->bEndpointAddress & BIT(7)) ? "IN" : "OUT");
  DLOG_NO_PREFIX(1, "  bmAttributes=%#x wMaxPacketSize=%#x bInterval=%d\n",
                 d->bmAttributes, d->wMaxPacketSize, d->bInterval);
  DLOG_NO_PREFIX(1, "  %s%s%s\n", tt, st, ut);
}

void dump_usb_if_desc(usb_if_desc_t *d)
{
  DLOG(1, "USB_IF_DESC:\n");
  DLOG_NO_PREFIX(1, "  bLength=%d bDescriptorType=%d bInterfaceNumber=%d\n",
                 d->bLength, d->bDescriptorType, d->bInterfaceNumber);
  DLOG_NO_PREFIX(1, "  bAlternateSetting=%#x bNumEndpoints=%d iInterface=%d\n",
                 d->bAlternateSetting, d->bNumEndpoints, d->iInterface);
  DLOG_NO_PREFIX(1, "  bInterfaceClass=%#x bInterfaceSubClass=%#x bInterfaceProtocol=%d\n",
                 d->bInterfaceClass, d->bInterfaceSubClass, d->bInterfaceProtocol);
}

void dump_usb_cfg_desc(usb_cfg_desc_t *d)
{
  DLOG(1, "USB_CFG_DESC:\n");
  DLOG_NO_PREFIX(1, "  bLength=%d bDescriptorType=%d wTotalLength=%d\n",
                 d->bLength, d->bDescriptorType, d->wTotalLength);

  DLOG_NO_PREFIX(1, "  bNumInterfaces=%d bConfigurationValue=%#x iConfiguration=%d\n",
                 d->bNumInterfaces, d->bConfigurationValue, d->iConfiguration);
  DLOG_NO_PREFIX(1, "  bmAttributes=%#x %s%sbMaxPower=%dmA\n",
                 d->bmAttributes,
                 (d->bmAttributes & BIT(6)) ? "(self-powered) " : "",
                 (d->bmAttributes & BIT(5)) ? "(remote-wakeup) " : "",
                 d->bMaxPower*2);
}

static inline int get_usb_dev_desc_vendor(usb_dev_desc_t *d) { return d->idVendor; }

void printnum(int i)
{
  DLOG(1, "asix num=%d\n", i);
}

void printbuf(buf_t b)
{
  DLOG(1, "asix: %s\n", b.s);
}

/* ************************************************** */

#define make_VendorRequest(x) x

volatile unsigned int *reg_32ksyncnt_cr = (volatile unsigned int *) TIMER_ADDR;

static inline void msleep(int msec)
{
  u32 now = *reg_32ksyncnt_cr;
  u32 then = now + (msec << 5); // a msec is approx 32-multiplier
  for(;*reg_32ksyncnt_cr < then;) ASM("MOV r0,r0");
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
