/*
 * OMAP4 High Speed USB Host Controller
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

#include "ats_types.h"
#include "ats_basics.h"
#include <user.h>

unsigned int _scheduler_capacity = 4 << 14;
unsigned int _scheduler_period = 15 << 14;
unsigned int _scheduler_affinity = 1;

/* ************************************************** */

/* some helper stuff for now until I sort out where it should go */

static inline void svc3(char *ptr, u32 len, u32 noprefix)
{
  register char *r0 asm("r0") = ptr;
  register u32 r1 asm("r1") = len;
  register u32 r2 asm("r2") = noprefix;
  asm volatile("SVC #3"::"r"(r0),"r"(r1),"r"(r2));
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

static inline u32 memset(void *dest, u32 byte, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  for(i=0;i<count;i++)
    dest8[i] = (u8) byte;
  return i;
}

/* ************************************************** */

#define REGOFFSET(base,off) ((base)+(off))
#define R(r) (*((volatile u32 *) (r)))

#define OMAP4460
#ifdef OMAP4460

/* ************************************************** */

/* USB datastructures */

/*
 * USB_DEV_REQ : USB Device Request
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 183
 */
PACKED_STRUCT(_usb_dev_req) {
  PACKED_FIELD(u8, bmRequestType);
  PACKED_FIELD(u8, bRequest);
  PACKED_FIELD(u16, wValue);
  PACKED_FIELD(u16, wIndex);
  PACKED_FIELD(u16, wLength);
} PACKED_END;
typedef struct _usb_dev_req usb_dev_req_t;


/*
 * USB_DEV_DESC : Standard Device Descriptor
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 197
 */
PACKED_STRUCT(_usb_dev_desc)
{
  PACKED_FIELD(u8  ,bLength);
  PACKED_FIELD(u8  ,bDescriptorType);
  PACKED_FIELD(u16 ,bcdUSB);
  PACKED_FIELD(u8  ,bDeviceClass);
  PACKED_FIELD(u8  ,bDeviceSubClass);
  PACKED_FIELD(u8  ,bDeviceProtocol);
  PACKED_FIELD(u8  ,bMaxPacketSize0);
  PACKED_FIELD(u16 ,idVendor);
  PACKED_FIELD(u16 ,idProduct);
  PACKED_FIELD(u16 ,bcdDevice);
  PACKED_FIELD(u8  ,iManufacturer);
  PACKED_FIELD(u8  ,iProduct);
  PACKED_FIELD(u8  ,iSerialNumber);
  PACKED_FIELD(u8  ,bNumConfigurations);
} PACKED_END;
typedef struct _usb_dev_desc usb_dev_desc_t;

void dump_usb_dev_desc(usb_dev_desc_t *d)
{
  DLOG(1, "USB_DEV_DESC:\n");
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

/* ************************************************** */
/* EHCI registers */

#define EHCI_BASE 0x4A064C00
#define EHCI_HCSPARAMS (EHCI_BASE + 0x04)
#define EHCI_HCCPARAMS (EHCI_BASE + 0x08)
#define EHCI_CMD (EHCI_BASE + 0x10)
#define EHCI_STS (EHCI_BASE + 0x14)
#define EHCI_USBINTR (EHCI_BASE + 0x18)
#define EHCI_FRINDEX (EHCI_BASE + 0x1C)
#define EHCI_PERIODICLISTBASE (EHCI_BASE + 0x24)
#define EHCI_ASYNCLISTADDR (EHCI_BASE + 0x28)
#define EHCI_CONFIGFLAG (EHCI_BASE + 0x50)
#define EHCI_PORTSC(i) (EHCI_BASE + 0x54 + (i << 2))

/* EHCI interrupt */

#define OMAP_EHCI_IRQ 77

/* ************************************************** */
/* EHCI data structures */

#define EHCI_QH_PTR_MASK (~BITMASK(5, 0))
#define EHCI_QH_PTR_T BIT(0)

/* Queue Head (section 3.6) */
struct _ehci_qh {
  u32 next;
  u32 characteristics;
  u32 capabilities;
  u32 current_td;
  u32 next_td;
  u32 alt_td;
  u32 overlay[10];              /* used by hardware */
} __attribute__((aligned (32)));
typedef struct _ehci_qh ehci_qh_t;

#define EHCI_TD_PTR_MASK (~BITMASK(5, 0))
#define EHCI_TD_PTR_T BIT(0)

/* Transfer Descriptor (section 3.5) */
struct _ehci_td {
  u32 next;
  u32 alt_next;
  u32 token;
  u32 buf[5];
} __attribute__((aligned (32)));
typedef struct _ehci_td ehci_td_t;

/* ************************************************** */

void dump_indent(indent)
{
  u32 i;
  for(i=0;i<indent;i++) DLOG_NO_PREFIX(1, " ");
}

void dump_td(ehci_td_t *td, u32 indent)
{
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "TD: %#x %#x %#x (%s) [%#x %#x %#x %#x %#x]\n",
                 td->next, td->alt_next, td->token,
                 td->token & BIT(7) ? "A" : "a", /* active? */
                 td->buf[0], td->buf[1], td->buf[2],
                 td->buf[3], td->buf[4]);
  if(!(td->next & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (td->next & EHCI_TD_PTR_MASK), indent);
}

void dump_qh(ehci_qh_t *qh, u32 indent)
{
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "QH: %#x %#x %#x %#x %#x %#x\n",
                 qh->next,
                 qh->characteristics,
                 qh->capabilities,
                 qh->current_td,
                 qh->next_td,
                 qh->alt_td);
  if(!(qh->current_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->current_td & EHCI_TD_PTR_MASK), indent+2);
  else if(!(qh->next_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->next_td & EHCI_TD_PTR_MASK), indent+2);
  else if(!(qh->alt_td & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (qh->alt_td & EHCI_TD_PTR_MASK), indent+2);

  // need to handle circular
  //if(!(qh->next & EHCI_QH_PTR_T))
  //  dump_qh((ehci_qh_t *) (qh->next & EHCI_QH_PTR_MASK), indent);
}

/* ************************************************** */
/* GPIO */

static u32 omap4460_gpio_base[] = {
  0x4A310000, 0x48055000, 0x48057000,
  0x48059000, 0x4805B000, 0x4805D000
};

#define GPIO_BASE(x) (omap4460_gpio_base[(x) >> 5])
#define GPIO_INDX(x) ((x) & 0x1F)

#define GPIO_SETDATAOUT 0x194
#define GPIO_CLRDATAOUT 0x190
#define GPIO_OE 0x134
#define GPIO_DATAIN 0x138

static inline void gpio_set_data_out(x) {
  R(GPIO_BASE(x) + GPIO_SETDATAOUT) = (1 << GPIO_INDX(x));

  /* set direction to output */
  u32 v = R(GPIO_BASE(x) + GPIO_OE);
  v &= ~(1 << GPIO_INDX(x));
  R(GPIO_BASE(x) + GPIO_OE) = v;
}
static inline void gpio_clr_data_out(x) {
  R(GPIO_BASE(x) + GPIO_CLRDATAOUT) = (1 << GPIO_INDX(x));

  /* set direction to output */
  u32 v = R(GPIO_BASE(x) + GPIO_OE);
  v &= ~(1 << GPIO_INDX(x));
  R(GPIO_BASE(x) + GPIO_OE) = v;
}
static inline void gpio_set_value(x, v) {
  R(GPIO_BASE(x) + (v ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1 << GPIO_INDX(x));;
}
static inline u32 gpio_get_value(x) {
  return (R(GPIO_BASE(x) + GPIO_DATAIN) & (1 << GPIO_INDX(x))) != 0;
}

/* ************************************************** */

typedef struct {
  u32 address;
  usb_dev_desc_t dev_desc;
} usb_device_t;

void usb_clear_device(usb_device_t *d)
{
  d->address = 0;
  d->dev_desc.bMaxPacketSize0 = 8; /* default */
}

void ehci_init_bulk_qh(usb_device_t *d, ehci_qh_t *qh, u32 endpt)
{
  memset(qh, 0, 1<<6);
  qh->next = EHCI_QH_PTR_T;
  qh->characteristics =
    /* Max Packet Size */
    SETBITS(d->dev_desc.bMaxPacketSize0, 16, 11) |
    /* Data toggle control */
    //BIT(14) |
    /* endpoint speed: high */
    SETBITS(2, 12, 2) |
    /* endpoint number */
    SETBITS(endpt, 8, 4) |
    /* device address */
    SETBITS(d->address, 0, 7) |
    0;
  qh->capabilities =
    /* Mult */
    SETBITS(3, 30, 2) |
    /* S-mask: 0 for async */
    SETBITS(0, 0, 8) |
    0;
  qh->next_td = EHCI_TD_PTR_T;
  qh->alt_td = EHCI_TD_PTR_T;
  qh->current_td = EHCI_TD_PTR_T;
}

void ehci_set_qh_address(ehci_qh_t *qh, u32 addr)
{
  u32 characteristics = qh->characteristics;
  characteristics &= ~BITMASK(0, 7);
  characteristics |= SETBITS(addr, 0, 7);
  qh->characteristics = characteristics;
}

void ehci_set_qh_max_packet_size(ehci_qh_t *qh, u32 mps)
{
  u32 characteristics = qh->characteristics;
  characteristics &= ~BITMASK(16, 11);
  characteristics |= SETBITS(mps, 16, 11);
  qh->characteristics = characteristics;
}

void ehci_detach_td(ehci_qh_t *qh)
{
  qh->current_td = EHCI_TD_PTR_T;
  qh->next_td = EHCI_TD_PTR_T;
  qh->alt_td = EHCI_TD_PTR_T;
}

void ehci_attach_td(ehci_qh_t *qh, ehci_td_t *td)
{
  qh->current_td = EHCI_TD_PTR_T; /* clear current TD */
  qh->overlay[0] = 0;             /* clear status of overlay */
  qh->next_td = (u32) td;         /* set new value for next TD */
}

status ehci_insert_qh(ehci_qh_t *cur, ehci_qh_t *new)
{
  new->next = cur->next;
#ifdef USE_VMM
  u32 phys;
  if(vmm_get_phys_addr(new, &phys) == OK) {
    cur->next = phys | BIT(1);  /* indicate type is QH */
    return OK;
  }
  return EINVALID;
#else
  cur->next = ((u32) new) | BIT(1); /* indicate type is QH */
  return OK;
#endif
}

#define EHCI_PIDCODE_OUT   0
#define EHCI_PIDCODE_IN    1
#define EHCI_PIDCODE_SETUP 2

u32 ehci_fill_td(ehci_td_t *td, u32 pidcode, void *data, u32 len, ehci_td_t *prev_td)
{
  u32 bytes = (len > 0x5000 ? 0x5000 : len); /* 0x5000 is max for single TD */
  memset(td, 0, 1<<5);
  td->next = EHCI_TD_PTR_T;
  if(prev_td != NULL)
    prev_td->next = (u32) td;
  td->alt_next = EHCI_TD_PTR_T;
  td->token =
    /* total bytes to transfer */
    SETBITS(bytes, 16, 15) |
    /* interrupt-on-complete */
    BIT(15) |
    /* current page */
    SETBITS(0, 12, 3) |
    /* error counter */
    SETBITS(3, 10, 2) |
    /* token: PID code: 0=OUT 1=IN 2=SETUP */
    SETBITS(pidcode, 8, 2) |
    /* status: active */
    BIT(7) |
    0;

  u32 cpage = 0;
  u32 addr = (u32) data, next;
  while(len > 0 && cpage < 5) {
#ifdef USE_VMM
    vmm_get_phys_addr((void *) addr, &td->buf[cpage])
#else
    td->buf[cpage] = addr;
#endif
    next = (addr & (~(PAGE_SIZE - 1))) + PAGE_SIZE;
    len -= next - addr;
    addr = next;
    cpage++;
  }

  if(len < 0)
    return 0;
  else
    return len;
}

/* ************************************************** */

#define USB_TYPE_DEV_DESC      0x01

#define USB_REQ_HOST_TO_DEVICE 0
#define USB_REQ_DEVICE_TO_HOST BIT(7)
#define USB_REQ_TYPE_STANDARD  0
#define USB_REQ_TYPE_CLASS     BIT(5)
#define USB_REQ_TYPE_VENDOR    BIT(6)
#define USB_REQ_RECIP_DEVICE   0
#define USB_REQ_RECIP_IFACE    BIT(0)
#define USB_REQ_RECIP_ENDPT    BIT(1)
#define USB_REQ_RECIP_OTHER    (BIT(0) | BIT(1))

#define USB_SET_ADDRESS          0x05
#define USB_GET_DESCRIPTOR       0x06

/* [micro]frames are 1/8th of a millisecond */
void ehci_microframewait(u32 count)
{
  u32 start = R(EHCI_FRINDEX);
  u32 end = (start + count) & 0x3FFF; /* mod 2^14 */
  if(end < start)
    /* wait for rollover */
    while(R(EHCI_FRINDEX) > end);

  while(R(EHCI_FRINDEX) < end);
}

void ehci_reset_root_port(u32 n)
{
  R(EHCI_PORTSC(n)) |= BIT(8);  /* assert reset */
  ehci_microframewait(10 << 3);
  //timer_32k_delay(1 << 9);      /* minimum 10ms wait */
  R(EHCI_PORTSC(n)) &= ~(BIT(8));               /* de-assert reset */
  while(GETBITS(R(EHCI_PORTSC(n)), 8, 1) == 1); /* wait for reset */
}

void usb_device_request(usb_dev_req_t *req, u32 bmrt, u32 br, u32 wv, u32 wi, u32 l)
{
  req->bmRequestType = bmrt;
  req->bRequest = br;
  req->wValue = wv;
  req->wIndex = wi;
  req->wLength = l;
}

void ehci_irq_handler(u32 v)
{
  DLOG(1, "IRQ\n");
}

void hsusbhc_init()
{
  usb_dev_req_t devr;

  DLOG(1, "hsusbhc_init\n");

  /* ************************************************** */
  /* SCRM */
  /* (necessary) */
#define SCRM_BASE 0x4A30A000
#define SCRM_ALTCLKSRC (SCRM_BASE + 0x110)
#define SCRM_AUXCLK3 (SCRM_BASE + 0x31C)

  DLOG(1, "rev_scrm=%#x\n", R(SCRM_BASE + 0x0));

  /* p1069 */
  /* SRCSELECT: clear bits 1:2 */
  R(SCRM_AUXCLK3) &= ~(BIT(1) | BIT(2));
  /* BIT(16): CLKDIV = DIV_BY_2 set */
  R(SCRM_AUXCLK3) |= BIT(16);
  /* BIT(8): enable */
  R(SCRM_AUXCLK3) |= BIT(8);

  /* p1056 */
  /* BIT(0) | BIT(2) | BIT(3): active, enable_int, enable_ext */
  R(SCRM_ALTCLKSRC) = BIT(0) | BIT(2) | BIT(3);

  DLOG(1, "SCRM_AUXCLK3=%#x\n", R(SCRM_AUXCLK3));
  DLOG(1, "SCRM_ALTCLKSRC=%#x\n", R(SCRM_ALTCLKSRC));

  /* ************************************************** */
  /* Use GPIO to control onboard hub */
  /* (necessary) */
#define USB_HUB_POWER 1
#define USB_HUB_NRESET 62

  /* disable hub power */
  gpio_clr_data_out(USB_HUB_POWER);
  gpio_clr_data_out(USB_HUB_NRESET);
  gpio_set_value(USB_HUB_POWER, 0);
  /* reset it */
  gpio_set_value(USB_HUB_NRESET, 1);
  DLOG(1, "USB_HUB_POWER=%d\n", gpio_get_value(USB_HUB_POWER));

  /* ************************************************** */
  /* Clocks */
  /* (necessary) */
#define CM_BASE 0x4a009000
  /* p978 */
  R(CM_BASE + 0x3D0) |= BIT(1);
  DLOG(1, "CM_L3INIT_FSUSB_CLKCTRL=%#x\n", R(0x4A0093D0));

  /* p980 */
  /* BIT(0): Module is managed automatically by hardware according to
     clock domain transition.  */
  /* BIT(8): optional clock control: PHY_48M enabled */
  R(CM_BASE + 0x3E0) = BIT(8) | BIT(0);
  DLOG(1, "CM_L3INIT_USBPHY_CLKCTRL=%#x\n", R(CM_BASE + 0x3E0));

  /* p974 */
  /* BIT(1): Module is explicitly enabled. Interface clock (if not
     used for functions) may be gated according to the clock domain
     state. Functional clocks are guarantied to stay present. As long as in
     this configuration, power domain sleep transition cannot happen. */
  /* BIT(15): USB-HOST optional clock control: FUNC48MCLK enabled */
  /* BIT(24): The functional clock is provided by an external PHY
     through an I/O pad. */
  R(CM_BASE + 0x358) = BIT(24) | BIT(15) | BIT(1);
  DLOG(1, "CM_L3INIT_HSUSBHOST_CLKCTRL=%#x\n", R(CM_BASE + 0x358));

  /* p978 */
  /* BIT(0): Module is managed automatically by hardware according to
     clock domain transition.  */
  R(CM_BASE + 0x368) = BIT(0);
  DLOG(1, "CM_L3INIT_HSUSBTLL_CLKCTRL=%#x\n", R(CM_BASE + 0x368));

  //R(0x4A009300) |= BIT(1);
  DLOG(1, "CM_L3INIT_CLKSTCTRL=%#x\n", R(CM_BASE + 0x300));

  /* ************************************************** */
  /* USBTLL p5110 */
  /* (optional) */
#define USBTLL_BASE 0x4a062000
#define USBTLL_SYS_CONFIG (USBTLL_BASE + 0x10)
#define USBTLL_SYS_STATUS (USBTLL_BASE + 0x14)

  R(USBTLL_SYS_CONFIG) = 0;
  R(USBTLL_SYS_CONFIG) = BIT(1);    /* reset */
  while(R(USBTLL_SYS_STATUS) == 0); /* wait for reset */
  R(USBTLL_SYS_CONFIG) |= BIT(8);   /* enable auto gating */
  R(USBTLL_SYS_CONFIG) |= BIT(3);   /* disable idle */
  R(USBTLL_SYS_CONFIG) |= BIT(2);   /* enable wakeup */

  DLOG(1, "USBTLL_SYS_CONFIG=%#x USBTLL_SYS_STATUS=%#x\n", R(USBTLL_SYS_CONFIG), R(USBTLL_SYS_STATUS));

  /* ************************************************** */
  /* High-speed Host p5147 */
  /* (optional) */
#define HSUSBHOST_BASE 0x4a064000
#define HSUSBHOST_SYS_CONFIG (HSUSBHOST_BASE + 0x10)
#define HSUSBHOST_HOST_CONFIG (HSUSBHOST_BASE + 0x40)

  /* disable idle */
  R(HSUSBHOST_SYS_CONFIG) &= ~(BIT(2) | BIT(3));
  R(HSUSBHOST_SYS_CONFIG) |= BIT(2);

  /* disable standby */
  R(HSUSBHOST_SYS_CONFIG) &= ~(BIT(4) | BIT(5));
  R(HSUSBHOST_SYS_CONFIG) |= BIT(4);

  /* enable HS operation of Ports 1 and 2 */
  R(HSUSBHOST_HOST_CONFIG) &= ~(BITMASK(16,4));

  R(HSUSBHOST_HOST_CONFIG) |= BIT(31); /* APP_START_CLK */

  DLOG(1, "HSUSBHOST_SYS_CONFIG=%#x HSUSBHOST_HOST_CONFIG=%#x\n",
       R(HSUSBHOST_SYS_CONFIG), R(HSUSBHOST_HOST_CONFIG));

  /* ************************************************** */

  /* Use GPIO to enable hub power */
  /* (necessary) */
  gpio_set_value(USB_HUB_POWER, 1);
  DLOG(1, "USB_HUB_POWER=%d\n", gpio_get_value(USB_HUB_POWER));

  /* ************************************************** */
  /* EHCI initialization */

  /* reset EHCI */
  R(EHCI_CMD) |= BIT(1);
  while(GETBITS(R(EHCI_CMD), 1, 1) == 1);

  /* program interrupts */
  R(EHCI_USBINTR) = BITMASK(0, 6);
  //FIXME
  //intc_set_irq_handler(OMAP_EHCI_IRQ, ehci_irq_handler);
  //intc_unmask_irq(OMAP_EHCI_IRQ);

  /* program periodiclistbase */

  /* enable EHCI */
  R(EHCI_CMD) |= BIT(0);

  R(EHCI_CONFIGFLAG) = 1;
  R(EHCI_PORTSC(0)) |= BIT(12); /* Port Power Enable */
  R(EHCI_PORTSC(1)) |= BIT(12); /* Port Power Enable */
  R(EHCI_PORTSC(2)) |= BIT(12); /* Port Power Enable */

  DLOG(1, "EHCI_FRINDEX=%#x\n", R(EHCI_FRINDEX));
  DLOG(1, "EHCI_HCSPARAMS=%#x EHCI_HCCPARAMS=%#x\n",
       R(EHCI_HCSPARAMS), R(EHCI_HCCPARAMS));
  DLOG(1, "EHCI_CMD=%#x EHCI_STS=%#x\n",
       R(EHCI_CMD), R(EHCI_STS));
  ehci_microframewait(500 << 3); /* 500 ms */
  DLOG(1, "EHCI PORTSC0=%#x PORTSC1=%#x PORTSC2=%#x\n",
       R(EHCI_PORTSC(0)), R(EHCI_PORTSC(1)), R(EHCI_PORTSC(2)));
  if(GETBITS(R(EHCI_PORTSC(0)),0,1)) {
    DLOG(1, "PORTSC0: connected\n");
    if(GETBITS(R(EHCI_PORTSC(0)),1,1))
      R(EHCI_PORTSC(0)) |= BIT(1); /* clear bit */
    switch(GETBITS(R(EHCI_PORTSC(0)),10,2)) {
    case 0:                       /* SE0 */
    case 2:                       /* J-state */
    case 3:                       /* undefined */
      DLOG(1, "PORTSC0: not low speed device.\n");
      break;
    default:                      /* K-state */
      DLOG(1, "PORTSC0: low speed device.\n");
      break;
    }
    ehci_reset_root_port(0);

    DLOG(1, "EHCI_PORTSC0=%#x\n", R(EHCI_PORTSC(0)));
    if(GETBITS(R(EHCI_PORTSC(0)),2,1))
      DLOG(1, "PORTSC0: enabled\n");
  }
  DLOG(1, "EHCI_FRINDEX=%#x\n", R(EHCI_FRINDEX));

  /* setup USB device */
  usb_device_t usbd;
  usb_clear_device(&usbd);
  ehci_td_t td1, td2, td3;
  ehci_qh_t qh1;

  ehci_init_bulk_qh(&usbd, &qh1, 0);
  ehci_insert_qh(&qh1, &qh1);

  /* don't insert QH until it is setup */
  R(EHCI_ASYNCLISTADDR) = (u32) &qh1;

  /* can't seem to enable async schedule until something is on the list */

  /* enable asynchronous schedule */
  R(EHCI_CMD) |= BIT(5);

  /* send SET_ADDRESS control request */
  u32 new_addr = 1;
  usb_device_request(&devr, USB_REQ_HOST_TO_DEVICE, USB_SET_ADDRESS, new_addr, 0, 0);

  ehci_detach_td(&qh1);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */
  ehci_attach_td(&qh1, &td1);

  /* wait for transaction */

  ehci_microframewait(500 << 3); /* 500 ms */
  dump_qh(&qh1, 0);

  usbd.address = new_addr;
  ehci_set_qh_address(&qh1, new_addr);

  /* get_descriptor: device descriptor */
  usb_device_request(&devr, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR, USB_TYPE_DEV_DESC << 8, 0, sizeof(usb_dev_desc_t));

  /* setup qTDs */
  ehci_detach_td(&qh1);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &usbd.dev_desc, sizeof(usb_dev_desc_t), &td1); /* DATA stage */
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */

  /* setup QH */
  //ehci_init_bulk_qh(&usbd, &qh1, 0);
  //ehci_insert_qh(&qh1, &qh1);
  ehci_attach_td(&qh1, &td1);

  /* queue it */
  dump_qh(&qh1, 0);

  DLOG(1, "EHCI_ASYNCLISTADDR=%#x\n", R(EHCI_ASYNCLISTADDR));
  DLOG(1, "EHCI_CMD=%#x EHCI_STS=%#x\n",
       R(EHCI_CMD), R(EHCI_STS));

  ehci_microframewait(500 << 3); /* 500 ms */
  dump_qh(&qh1, 0);

  ehci_detach_td(&qh1);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL);
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &usbd.dev_desc, sizeof(usb_dev_desc_t), &td1);
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */
  ehci_set_qh_max_packet_size(&qh1, usbd.dev_desc.bMaxPacketSize0);
  ehci_attach_td(&qh1, &td1);

  ehci_microframewait(500 << 3); /* 500 ms */
  dump_qh(&qh1, 0);

  dump_usb_dev_desc(&usbd.dev_desc);

  DLOG(1, "EHCI_CMD=%#x EHCI_STS=%#x\n",
       R(EHCI_CMD), R(EHCI_STS));

  DLOG(1, "initialization complete\n");
  ehci_microframewait(1000 << 3); /* 1000 ms */
}

#else
#error "High Speed USB Host Controller not implemented"
#endif



/* int main(void)
 * {
 *   hsusbhc_init();
 *
 *   for(;;);
 *   return 0;
 * } */


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
