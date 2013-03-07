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
#include <pool.h>
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

#define USB_TYPE_DEV_DESC      0x01
#define USB_TYPE_CFG_DESC      0x02
#define USB_TYPE_STR_DESC      0x03
#define USB_TYPE_IF_DESC       0x04
#define USB_TYPE_EPT_DESC      0x05
#define USB_TYPE_QUA_DESC      0x06
#define USB_TYPE_SPD_CFG_DESC  0x07
#define USB_TYPE_IF_PWR_DESC   0x08

#define USB_REQ_HOST_TO_DEVICE 0
#define USB_REQ_DEVICE_TO_HOST BIT(7)
#define USB_REQ_TYPE_STANDARD  0
#define USB_REQ_TYPE_CLASS     BIT(5)
#define USB_REQ_TYPE_VENDOR    BIT(6)
#define USB_REQ_RECIP_DEVICE   0
#define USB_REQ_RECIP_IFACE    BIT(0)
#define USB_REQ_RECIP_ENDPT    BIT(1)
#define USB_REQ_RECIP_OTHER    (BIT(0) | BIT(1))

#define USB_GET_STATUS           0x00
#define USB_CLEAR_FEATURE        0x01
#define USB_SET_FEATURE          0x03
#define USB_SET_ADDRESS          0x05
#define USB_GET_DESCRIPTOR       0x06
#define USB_SET_DESCRIPTOR       0x07
#define USB_GET_CONFIGURATION    0x08
#define USB_SET_CONFIGURATION    0x09
#define USB_GET_INTERFACE        0x0A
#define USB_SET_INTERFACE        0x0B
#define USB_SYNCH_FRAME          0x0C

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

/*
 * USB_CFG_DESC : Standard Configuration Descriptor
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 199
 */
PACKED_STRUCT(_usb_cfg_desc)
{
  PACKED_FIELD(u8, bLength);
  PACKED_FIELD(u8, bDescriptorType);
  PACKED_FIELD(u16, wTotalLength);
  PACKED_FIELD(u8, bNumInterfaces);
  PACKED_FIELD(u8, bConfigurationValue);
  PACKED_FIELD(u8, iConfiguration);
  PACKED_FIELD(u8, bmAttributes);
  PACKED_FIELD(u8, bMaxPower);
} PACKED_END;
typedef struct _usb_cfg_desc usb_cfg_desc_t;

void dump_usb_cfg_desc(usb_cfg_desc_t *d)
{
  DLOG(1, "USB_CFG_DESC:\n");
  DLOG_NO_PREFIX(1, "  bLength=%d bDescriptorType=%d wTotalLength=%d\n",
                 d->bLength, d->bDescriptorType, d->wTotalLength);
  DLOG_NO_PREFIX(1, "  bNumInterfaces=%d bConfigurationValue=%#x iConfiguration=%d\n",
                 d->bNumInterfaces, d->bConfigurationValue, d->iConfiguration);
  DLOG_NO_PREFIX(1, "bmAttributes=%#x %s%sbMaxPower=%dmA\n",
                 d->bmAttributes,
                 (d->bmAttributes & BIT(6)) ? "(self-powered) " : "",
                 (d->bmAttributes & BIT(5)) ? "(remote-wakeup) " : "",
                 d->bMaxPower*2);
}

/*
 * USB_IF_DESC : Standard Interface Descriptor
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 202
 */
PACKED_STRUCT(_usb_if_desc)
{
  PACKED_FIELD(u8, bLength);
  PACKED_FIELD(u8, bDescriptorType);
  PACKED_FIELD(u8, bInterfaceNumber);
  PACKED_FIELD(u8, bAlternateSetting);
  PACKED_FIELD(u8, bNumEndpoints);
  PACKED_FIELD(u8, bInterfaceClass);
  PACKED_FIELD(u8, bInterfaceSubClass);
  PACKED_FIELD(u8, bInterfaceProtocol);
  PACKED_FIELD(u8, iInterface);
} PACKED_END;
typedef struct _usb_if_desc usb_if_desc_t;

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

/*
 * USB_EPT_DESC : Standard Endpoint Descriptor
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 203
 */
PACKED_STRUCT(_usb_ept_desc)
{
  PACKED_FIELD(u8, bLength);
  PACKED_FIELD(u8, bDescriptorType);
  PACKED_FIELD(u8, bEndpointAddress);
  PACKED_FIELD(u8, bmAttributes);
  PACKED_FIELD(u16, wMaxPacketSize);
  PACKED_FIELD(u8, bInterval);
} PACKED_END;
typedef struct _usb_ept_desc usb_ept_desc_t;

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
                 d->bLength, d->bDescriptorType, d->bEndpointAddress,
                 (d->bEndpointAddress & BIT(7)) ? "IN" : "OUT");
  DLOG_NO_PREFIX(1, "  bmAttributes=%#x wMaxPacketSize=%#x bInterval=%d\n",
                 d->bmAttributes, d->wMaxPacketSize, d->bInterval);
  DLOG_NO_PREFIX(1, "  %s%s%s\n", tt, st, ut);
}

/* 
 * USB_STR_DESC: Standard String Descriptor
 *
 * Reference :
 *     Universal Serial Bus Specification
 *     Revision 1.1, Page 205
 */

PACKED_STRUCT(_usb_str_desc)
{
  PACKED_FIELD(u8, bLength);
  PACKED_FIELD(u8, bDescriptorType);
  PACKED_FIELD(u8, bString[]);
} PACKED_END;
typedef struct _usb_str_desc usb_str_desc_t;


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
  volatile u32 next;
  volatile u32 characteristics;
  volatile u32 capabilities;
  volatile u32 current_td;
  volatile u32 next_td;
  volatile u32 alt_td;
  volatile u32 overlay[10];              /* used by hardware */
} __attribute__((aligned (32)));
typedef struct _ehci_qh ehci_qh_t;

#define EHCI_TD_PTR_MASK (~BITMASK(5, 0))
#define EHCI_TD_PTR_T BIT(0)
#define EHCI_TD_TOKEN_A BIT(7)

/* Transfer Descriptor (section 3.5) */
struct _ehci_td {
  volatile u32 next;
  volatile u32 alt_next;
  volatile u32 token;
  volatile u32 buf[5];
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
                 td->token & EHCI_TD_TOKEN_A ? "A" : "a", /* active? */
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

typedef struct _usb_port_t {
  void (*reset)(struct _usb_port_t *); /* resets port to find device */
  status (*get_connect_status)(struct _usb_port_t *); /* returns OK if connected  */
  void (*clr_connect_status)(struct _usb_port_t *); /* clears connect status on device */
  u32 (*get_line_status)(struct _usb_port_t *);     /* get line status value (e.g. J-state, K-state, SE0) */
  status (*enabled)(struct _usb_port_t *);          /* is enabled? */
} usb_port_t;

void usb_port_reset(usb_port_t *p)
{
  p->reset(p);
}

status usb_port_get_connect_status(usb_port_t *p)
{
  return p->get_connect_status(p);
}

void usb_port_clr_connect_status(usb_port_t *p)
{
  p->clr_connect_status(p);
}

u32 usb_port_get_line_status(usb_port_t *p)
{
  return p->get_line_status(p);
}

status usb_port_enabled(usb_port_t *p)
{
  return p->enabled(p);
}

typedef struct {
  usb_port_t port;
  u32 n;
} usb_root_port_t;

static void usb_root_port_reset(usb_port_t *p)
{
  void ehci_reset_root_port(u32 n);
  ehci_reset_root_port(container_of(p,usb_root_port_t,port)->n);
}

static status usb_root_port_get_connect_status(usb_port_t *p)
{
  return GETBITS(R(EHCI_PORTSC(container_of(p,usb_root_port_t,port)->n)), 0, 1) ? OK : EINVALID;
}

static u32 usb_root_port_get_line_status(usb_port_t *p)
{
  return GETBITS(R(EHCI_PORTSC(container_of(p,usb_root_port_t,port)->n)), 10, 2);
}

static void usb_root_port_clr_connect_status(usb_port_t *p)
{
  u32 status = R(EHCI_PORTSC(container_of(p,usb_root_port_t,port)->n));
  
  if(GETBITS(status, 1, 1))
    R(EHCI_PORTSC(container_of(p,usb_root_port_t,port)->n)) = status | BIT(1);
}

static status usb_root_port_enabled(usb_port_t *p)
{
  u32 x;
  x = GETBITS(R(EHCI_PORTSC(container_of(p,usb_root_port_t,port)->n)),2,1) ? OK : EINVALID;
  return x;
}

/* ************************************************** */

typedef struct {
  ehci_qh_t qh;
  u32 address;
  usb_dev_desc_t dev_desc;
} usb_device_t;

// create a pool for simple dynamic allocation
POOL_DEFN(usb_device,usb_device_t,8,32);

void usb_clear_device(usb_device_t *d)
{
  d->address = 0;
  d->dev_desc.bMaxPacketSize0 = 8; /* default */
}

void ehci_init_bulk_qh(usb_device_t *d, u32 endpt)
{
  ehci_qh_t *qh = &d->qh;
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

void ehci_set_qh_endpoint(ehci_qh_t *qh, u32 endp)
{
  u32 characteristics = qh->characteristics;
  characteristics &= ~BITMASK(8, 4);
  characteristics |= SETBITS(endp, 8, 4);
  qh->characteristics = characteristics;
}

u32 ehci_get_qh_address(ehci_qh_t *qh)
{
  return GETBITS(qh->characteristics, 0, 7);
}

u32 ehci_get_qh_endpoint(ehci_qh_t *qh)
{
  return GETBITS(qh->characteristics, 8, 4);
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

void usb_init_subdevice(usb_device_t *d, u32 endpt, usb_device_t *parent)
{
  usb_clear_device(d);
  ehci_init_bulk_qh(d, endpt);
  ehci_insert_qh(&parent->qh, &d->qh);
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
  DLOG(1, "ehci_reset_root_port(%d)\n", n);
  R(EHCI_PORTSC(n)) |= BIT(8);  /* assert reset */
  ehci_microframewait(10 << 3);
  //timer_32k_delay(1 << 9);      /* minimum 10ms wait */
  R(EHCI_PORTSC(n)) &= ~(BIT(8));               /* de-assert reset */
  while(GETBITS(R(EHCI_PORTSC(n)), 8, 1) == 1); /* wait for reset */
  DLOG(1, "ehci_reset_root_port(%d) complete\n", n);
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

status ehci_set_configuration(ehci_qh_t *qh, u32 cfgval)
{
  usb_dev_req_t devr;
  ehci_td_t td1, td2;

  usb_device_request(&devr, USB_REQ_HOST_TO_DEVICE, USB_SET_CONFIGURATION, cfgval, 0, 0);

  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */

  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td2.token & EHCI_TD_TOKEN_A);

  return OK;
}

/* ************************************************** */
/* Hub */

#define USB_HUB_PORT_STAT_POWER 0x0100
#define USB_HUB_PORT_RESET 4
#define USB_HUB_PORT_POWER 8
#define USB_HUB_PORT_C_RESET 20
#define USB_HUB_CHARACTERISTICS_LPSMODE BITMASK(2, 0) /* logical power switching */
#define USB_HUB_CHARACTERISTICS_COMPOUND BIT(2)       /* identifies compound device */
#define USB_HUB_CHARACTERISTICS_OPMODE BITMASK(2, 3)  /* over-current protection */

PACKED_STRUCT(_usb_hub_desc)
{
  PACKED_FIELD(u8, bDescLength);
  PACKED_FIELD(u8, bDescriptorType);
  PACKED_FIELD(u8, bNbrPorts);
  PACKED_FIELD(u16, wHubCharacteristics);
  PACKED_FIELD(u8, bPwrOn2PwrGood);         /* (in 2ms intervals) */
  PACKED_FIELD(u8, bHubContrCurrent);       /* max power requirement in mA */
  /* followed by DeviceRemovable / PortPwrCtrlMask variable-length fields */
} PACKED_END;
typedef struct _usb_hub_desc usb_hub_desc_t;

void dump_usb_hub_desc(usb_hub_desc_t *d)
{
  u32 cr;
  ASM("MRC p15, #0, %0, c1, c0, #0":"=r"(cr));
  DLOG(1,"SCTLR=%#x\n", cr);

  DLOG(1, "USB_HUB_DESC\n");
  DLOG_NO_PREFIX(1, "  bDescLength=%d bDescriptorType=%#x bNbrPorts=%d\n",
                 d->bDescLength, d->bDescriptorType, d->bNbrPorts);
  DLOG_NO_PREFIX(1, "  wHubCharacteristics=%#x bPwrOn2PwrGood=%dms bHubContrCurrent=%dmA\n",
                 d->wHubCharacteristics, d->bPwrOn2PwrGood << 1, d->bHubContrCurrent);
}

typedef struct {
  usb_port_t port;
  usb_device_t *dev;
  ehci_qh_t *qh;
  u32 n, pwr_wait;
} usb_hub_port_t;

static void usb_hub_port_reset(usb_port_t *p)
{
  usb_hub_port_t *hubp = container_of(p,usb_hub_port_t,port);
  usb_dev_req_t devr;
  ehci_td_t td1, td2;
  ehci_qh_t *qh = hubp->qh;
  u32 portn = hubp->n;

  /* ** assert reset ** */
  usb_device_request(&devr,
                     USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                     USB_SET_FEATURE,
                     USB_HUB_PORT_RESET, portn, 0);

  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */
  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td2.token & EHCI_TD_TOKEN_A);

  /* FIXME: check status of C_RESET instead */
  ehci_microframewait(10 << 3); /* wait 10 ms */

  /* ** de-assert reset ** */
  usb_device_request(&devr,
                     USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                     USB_CLEAR_FEATURE,
                     USB_HUB_PORT_RESET, portn, 0);

  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */
  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td2.token & EHCI_TD_TOKEN_A);

  ehci_microframewait(hubp->pwr_wait << 3);
}

static u32 usb_hub_port_get_status(usb_port_t *p)
{
  usb_hub_port_t *hubp = container_of(p,usb_hub_port_t,port);
  usb_dev_req_t devr;
  ehci_td_t td1, td2, td3;
  ehci_qh_t *qh = hubp->qh;
  u32 portn = hubp->n;
  u32 portstatus;

  /* check status */
  usb_device_request(&devr,
                     USB_REQ_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                     USB_GET_STATUS,
                     0, portn, 4);
  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &portstatus, 4, &td1); /* DATA stage */
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */
  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td3.token & EHCI_TD_TOKEN_A);

  return portstatus;
}

static status usb_hub_port_get_connect_status(usb_port_t *p)
{
  return (usb_hub_port_get_status(p) & 1) ? OK : EINVALID;
}

static void usb_hub_port_clr_connect_status(usb_port_t *p)
{
  /* FIXME: CLEAR_PORT_FEATURE C_CONNECTION */
}

static u32 usb_hub_port_get_line_status(usb_port_t *p)
{
  /* BIT(9) == 1 iff low-speed device attached */
  return (usb_hub_port_get_status(p) & BIT(9)) ? 1 /* pretend as K-state */ : 3 /* pretend as 'undefined' */ ;
}

#define HUBPORT  { .reset = usb_hub_port_reset,                 \
      .get_connect_status = usb_hub_port_get_connect_status,    \
      .clr_connect_status = usb_hub_port_clr_connect_status,    \
      .get_line_status    = usb_hub_port_get_line_status,       \
      .enabled            = usb_hub_port_enabled }

static status usb_hub_port_enabled(usb_port_t *p)
{
  return (usb_hub_port_get_status(p) & BIT(1)) ? OK : EINVALID;
}

static status setup_hub(usb_device_t *d, u32 cfgval, u32 ept)
{
  ehci_qh_t *qh = &d->qh;
  usb_device_t *newd;
  usb_dev_req_t devr;
  ehci_td_t td1, td2, td3;
  usb_hub_desc_t hubd;

  ehci_set_configuration(qh, cfgval);

#define USB_HUB_DESCRIPTOR_TYPE 0x29
  usb_device_request(&devr, USB_REQ_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS, USB_GET_DESCRIPTOR,
                     USB_HUB_DESCRIPTOR_TYPE << 8, 0, sizeof(usb_hub_desc_t));

  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &hubd, sizeof(usb_hub_desc_t), &td1); /* DATA stage */
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */

  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td3.token & EHCI_TD_TOKEN_A);

  dump_usb_hub_desc(&hubd);

  /* power on port */

  u32 portn = 1;
  u32 portstatus = 0;
  u32 attempts;
  for(portn = 1; portn <= hubd.bNbrPorts; portn++) {
    attempts = 0;
    portstatus = 0;
    while((portstatus & USB_HUB_PORT_STAT_POWER) == 0 && attempts < 5) {

      DLOG(1, "Attempting to power on port %d\n", portn);

      usb_device_request(&devr,
                         USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                         USB_SET_FEATURE,
                         USB_HUB_PORT_POWER, portn, 0);

      /* setup qTDs */
      ehci_detach_td(qh);
      ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
      ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */

      /* queue it */
      ehci_attach_td(qh, &td1);

      /* wait for it */
      while(td2.token & EHCI_TD_TOKEN_A);

      ehci_microframewait((hubd.bPwrOn2PwrGood << 1) << 3);


      /* check status */
      usb_device_request(&devr,
                         USB_REQ_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                         USB_GET_STATUS,
                         0, portn, 4);
      /* setup qTDs */
      ehci_detach_td(qh);
      ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
      ehci_fill_td(&td2, EHCI_PIDCODE_IN, &portstatus, 4, &td1); /* DATA stage */
      ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */
      /* queue it */
      ehci_attach_td(qh, &td1);

      /* wait for it */
      while(td3.token & EHCI_TD_TOKEN_A);

      attempts++;
      if(attempts >= 5) break;
    }

    if(attempts >= 5) continue;

    DLOG(1, "portstatus(%d)=%#x\n", portn, portstatus);

    if(portstatus & 1) {
      /* something connected */
      void ehci_enumerate(usb_port_t *, usb_device_t *);
      usb_hub_port_t p = { .port = HUBPORT, .n = portn, .dev = d, .qh = qh, .pwr_wait = hubd.bPwrOn2PwrGood << 1 };
      newd = usb_device_pool_alloc();
      usb_init_subdevice(newd, 0, d);
      ehci_enumerate((usb_port_t *) &p, newd);
    }
  }

  return OK;
}

static status probe_hub(usb_device_t *d)
{
  ehci_qh_t *qh = &d->qh;
  ehci_td_t td1, td2, td3;
  usb_dev_req_t devr;
  usb_cfg_desc_t cfgd;
  u32 cfgidx, ifidx, eptidx;
  u8 buffer[128];               /* temporary buffer */

  if(d->dev_desc.bDeviceClass != 9)
    return EINVALID;

  for(cfgidx=0;cfgidx<d->dev_desc.bNumConfigurations;cfgidx++) {
    /* get_descriptor: config descriptor; mostly to find wTotalLength */
    usb_device_request(&devr, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                       (USB_TYPE_CFG_DESC << 8) | cfgidx,
                       0, sizeof(usb_dev_desc_t));

    /* setup qTDs */
    ehci_detach_td(qh);
    ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
    ehci_fill_td(&td2, EHCI_PIDCODE_IN, &cfgd, sizeof(usb_cfg_desc_t), &td1); /* DATA stage */
    ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */

    /* queue it */
    ehci_attach_td(qh, &td1);

    /* wait for it */
    while(td2.token & EHCI_TD_TOKEN_A); /* td3 may not complete */

    dump_usb_cfg_desc(&cfgd);

    if(cfgd.wTotalLength > 128) return EINVALID; /* FIXME */

    /* get_descriptor: config descriptor + interfaces + endpoints */
    usb_device_request(&devr, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                       (USB_TYPE_CFG_DESC << 8) | cfgidx,
                       0, sizeof(usb_dev_desc_t));

    /* setup qTDs */
    ehci_detach_td(qh);
    ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
    ehci_fill_td(&td2, EHCI_PIDCODE_IN, buffer, cfgd.wTotalLength, &td1); /* DATA stage */
    ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */

    /* queue it */
    ehci_attach_td(qh, &td1);

    /* wait for it */
    while(td3.token & EHCI_TD_TOKEN_A);

    void *ptr = buffer + sizeof(usb_cfg_desc_t);
    for(ifidx=0; ifidx<cfgd.bNumInterfaces; ifidx++) {
      usb_if_desc_t *ifd = (usb_if_desc_t *) ptr;
      dump_usb_if_desc(ifd);
      ptr += sizeof(usb_if_desc_t);
      for(eptidx=0; eptidx<ifd->bNumEndpoints; eptidx++) {
        usb_ept_desc_t *eptd = (usb_ept_desc_t *) ptr;
        //dump_usb_ept_desc(eptd); //FIXME:wtf

        if(ifd->bInterfaceClass == 9 && GETBITS(eptd->bmAttributes, 0, 2) == 0) {
          DLOG(1, "Found hub cfgval=%d ept=%d.\n", cfgd.bConfigurationValue, GETBITS(eptd->bEndpointAddress, 0, 4));
          return setup_hub(d, cfgd.bConfigurationValue, GETBITS(eptd->bEndpointAddress, 0, 4));
        }

        ptr += sizeof(usb_ept_desc_t);
      }
    }
  }

  return EINVALID;
}

/* ************************************************** */

u32 unused_address = 1;         /* store next unused address */

void ehci_setup_new_device(usb_device_t *usbd)
{
  usb_dev_req_t devr;
  ehci_td_t td1, td2, td3;
  ehci_qh_t *qh = &usbd->qh;

  ehci_set_qh_max_packet_size(qh, 8); /* start with 8 byte max packet size */

  /* create SET_ADDRESS control request */
  u32 new_addr = unused_address++;
  usb_device_request(&devr, USB_REQ_HOST_TO_DEVICE, USB_SET_ADDRESS, new_addr, 0, 0);

  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, NULL, 0, &td1); /* STATUS stage */
  ehci_attach_td(qh, &td1);

  /* wait for transaction */
  while(td2.token & EHCI_TD_TOKEN_A);

  usbd->address = new_addr;
  ehci_set_qh_address(qh, new_addr);

  /* get_descriptor: device descriptor */
  usb_device_request(&devr, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR, USB_TYPE_DEV_DESC << 8, 0, sizeof(usb_dev_desc_t));

  /* setup qTDs */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  /* only get first 8 bytes */
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &usbd->dev_desc, 8, &td1); /* DATA stage */
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */

  /* queue it */
  ehci_attach_td(qh, &td1);

  /* wait for it */
  while(td2.token & EHCI_TD_TOKEN_A); /* td3 may not complete */

  /* get it again, but with correct max packet size */
  ehci_detach_td(qh);
  ehci_fill_td(&td1, EHCI_PIDCODE_SETUP, &devr, sizeof(usb_dev_req_t), NULL);
  ehci_fill_td(&td2, EHCI_PIDCODE_IN, &usbd->dev_desc, sizeof(usb_dev_desc_t), &td1);
  ehci_fill_td(&td3, EHCI_PIDCODE_OUT, NULL, 0, &td2); /* STATUS stage */
  ehci_set_qh_max_packet_size(qh, usbd->dev_desc.bMaxPacketSize0);
  ehci_attach_td(qh, &td1);

  while(td3.token & EHCI_TD_TOKEN_A);

  DLOG(1, "usb_dev_desc=%#x\n", &usbd->dev_desc);
  dump_usb_dev_desc(&usbd->dev_desc);

  /* FIXME: for now */
  probe_hub(usbd);
}

/* use queue head to enumerate a USB device available on port */
void ehci_enumerate(usb_port_t *p, usb_device_t *usbd)
{
  if(usb_port_get_connect_status(p) == OK) {
    DLOG(1, "usb_port <%#x>: connected\n", p);
    usb_port_clr_connect_status(p);
    u32 linestatus = usb_port_get_line_status(p);
    if(linestatus == 1) {
      /* 1: K-state */
      DLOG(1, "usb_port <%#x>: low speed device (skipping).\n", p);
      return;
    } else {
      /* 0: SE0 */
      /* 2: J-state */
      /* 3: undefined */
      DLOG(1, "usb_port <%#x>: not low speed device.\n", p);
      usb_port_reset(p);

      if(usb_port_enabled(p) == OK)
        DLOG(1, "usb_port <%#x>: enabled\n", p);
      else
        return;                    /* not enabled = no attached high speed device */
    }

    ehci_setup_new_device(usbd);
  }
}

void hsusbhc_init()
{
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

  DLOG(1, "EHCI_FRINDEX=%#x\n", R(EHCI_FRINDEX));

  /* setup root ports */
#define ROOTPORT { .reset = usb_root_port_reset, \
      .get_connect_status = usb_root_port_get_connect_status,\
      .clr_connect_status = usb_root_port_clr_connect_status,\
      .get_line_status    = usb_root_port_get_line_status,\
      .enabled            = usb_root_port_enabled }

  usb_root_port_t rootport[] = {
    { .port = ROOTPORT, .n = 0 }
    /* stupid fucking random error being caused by this */
    //,{ .port = ROOTPORT, .n = 1 }
    //,{ .port = ROOTPORT, .n = 2 }
  };

  usb_device_t *usbd = usb_device_pool_alloc();
  usb_init_subdevice(usbd, 0, usbd); /* self-parent */

  /* don't insert QH until it is setup */
  R(EHCI_ASYNCLISTADDR) = (u32) &usbd->qh;

  /* can't seem to enable async schedule until something is on the list */

  /* enable asynchronous schedule */
  R(EHCI_CMD) |= BIT(5);

  DLOG(1, "EHCI_CMD=%#x EHCI_STS=%#x\n",
       R(EHCI_CMD), R(EHCI_STS));

  ehci_enumerate(&rootport[0].port, usbd);
  //ehci_enumerate(&rootport[1].port, &qh1);
  //ehci_enumerate(&rootport[2].port, &qh1);

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
