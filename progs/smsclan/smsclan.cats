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
unsigned int _scheduler_capacity = 19 << 1;
unsigned int _scheduler_period = 20 << 1;
unsigned int _scheduler_affinity = 2;

ipcmapping_t _ipcmappings[] = {
  { IPC_SEEK, "ehci_info", IPC_READ, "multireader", 1, NULL },
  { IPC_SEEK, "ehci_usbdevices", IPC_READ | IPC_WRITE | IPC_DEVICEMEM, "usbdevices", 1, NULL },
  { IPC_SEEK, "uip-in", IPC_WRITE, "fixedslot", 4, NULL },
  { IPC_OFFER, "uip-out", IPC_READ, "fixedslot", 4, NULL },
  {0}
};

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
  DLOG(1, "smsclan USB_DEV_DESC:\n");
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
  DLOG(1, "smsclan num=%d\n", i);
}

void printbuf(buf_t b)
{
  DLOG(1, "smsclan: %s\n", b.s);
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

// FIXME: figure out why this is needed:
#define smsclan_write_wait() do { int _ww; for(_ww=0;_ww<3000000;_ww++) { ASM("NOP"); } } while (0)

// UIP support
void copy_into_uip_buf(u8 *buf, u32 len)
{
  // Assume buf has a 4-byte USB Ethernet header (not counted in len)
  memcpy(uip_buf, buf + 4, len);
  uip_len = len;
}

u32 copy_from_uip_buf(u8 *buf)
{
  // We must leave space for two command words (USB Ethernet: SMSC LAN)
  memcpy(buf + 8, uip_buf, uip_len);
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

#define get_uip_len() uip_len
#define get_uip_buftype() (((struct uip_eth_hdr *)&uip_buf[0])->type)

#define copy_from_uip_array(tbuf, arr, txlen) memcpy(((u8 *) (tbuf)) + 8, ((u8 *) (arr)) + 12, txlen)
#define copy_into_uip_array(arr, rbuf, rxlen) memcpy(((u8 *) (arr)) + 12, ((u8 *) (rbuf)) + 4, rxlen)

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
