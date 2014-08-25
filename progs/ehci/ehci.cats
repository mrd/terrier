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
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "prelude/CATS/pointer.cats"
#include "prelude/CATS/integer.cats"
#include <pool.h>
#include <user.h>

unsigned int _scheduler_capacity = 4 << 14;
unsigned int _scheduler_period = 15 << 14;
unsigned int _scheduler_affinity = 1;

mapping_t _mappings[] = {
  { 0x48050000, NULL, 10, 12, 0, 0, R_RW, "needed for GPIO" },
  { 0x4A064000, NULL, 1, 12, 0, 0, R_RW, "EHCI base" },
  { 0x4A009000, NULL, 1, 12, 0, 0, R_RW, "CM base" },
  { 0x4A062000, NULL, 1, 12, 0, 0, R_RW, "USBTLL base" },
  { 0x4A064000, NULL, 1, 12, 0, 0, R_RW, "HSUSBHOST base" },
  { 0x4A310000, NULL, 2, 12, 0, 0, R_RW, "needed for GPIO" },
  { 0x4A30A000, NULL, 1, 12, 0, 0, R_RW, "SCRM base" },

  { 0, 0, 0, 0, 0, 0, 0, 0 }
};

u8 *_irq_status_table = 0;      /* FIXME: must be initialized, until I
                                 * figure out what to do about ELF
                                 * "Common" symbols. */

volatile u32 irq_state = 0;     /* IRQ state avoids race condition
                                 * between saving previous context and
                                 * loading interrupt context. */

u32 saved_context[18] = {0};
u32 *_kernel_saved_context = NULL;

/* ************************************************** */

void *atspre_null_ptr = 0;      //FIXME

ats_bool_type atspre_pgt(const ats_ptr_type p1, const ats_ptr_type p2) {
  return (p1 > p2) ;
}

ipcmapping_t _ipcmappings[] = {
  { IPC_SEEK, "uart", IPC_WRITE, "fourslot2w", 1, NULL },
  { IPC_OFFER, "ehci_info", IPC_WRITE, "multireader", 1, NULL },
  {0}
};

/* ************************************************** */

typedef struct { char s[124]; } buf_t;

static inline int strnlen(const char *s, int max)
{
  int count = 0;
  while(s[count] != 0 && count < max) count++;
  return count;
}

#define buf_init(bref,str) do { memset((bref)->s, 0, 124); memcpy((bref)->s,str,strnlen(str, 123)); } while (0)

static int uart_token = 0;
static inline int get_uart_token_vt() { return uart_token; }
static inline void put_uart_token_vt(int t) { uart_token = t; }

/* some helper stuff for now until I sort out where it should go */
#if 0
static inline u32 memcpy(void *dest, void *src, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  u8 *src8 = (u8 *) src;
  for(i=0;i<count;i++)
    dest8[i] = src8[i];
  return i;
}
#endif

static inline u8 *svc1(void)
{
  register u8 *r0 asm("r0");
  asm volatile("SVC #1":"=r" (r0));
  return r0;
}

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
  char buffer[DLOG_BUFLEN];
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

#if 0
static inline u32 memset(void *dest, u32 byte, u32 count)
{
  u32 i;
  u8 *dest8 = (u8 *) dest;
  for(i=0;i<count;i++)
    dest8[i] = (u8) byte;
  return i;
}
#endif

#ifdef USE_VMM

#define arm_mmu_va_to_pa(vaddr, op2, res)                       \
  ASM("MCR p15,0,%1,c7,c8,%2\n"                                 \
      "MRC p15,0,%0,c7,c4,0":"=r"(res):"r"(vaddr),"I"(op2))

status vmm_get_phys_addr(void *vaddr, physaddr *paddr)
{
  u32 addr = (u32) vaddr;
  u32 res;
  arm_mmu_va_to_pa((void *) (addr & 0xFFFFF000), 0, res);
  if (res & 1) return EINVALID;
  *paddr = (res & 0xFFFFF000) + (addr & 0xFFF);
  return OK;
}
#endif

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

/* ************************************************** */

//#define DEBUGREGS
#ifdef DEBUGREGS
struct omapregs {
  char *desc; u32 addr;
} omapregs[] = {
  // ---	Port padconfs		---
  // -------------------------------------
  { "USBB2_ULPITLL_CLK_NXT", 0x4A002890 },
  { "USBB2_ULPITLL_DIR_STP", 0x4A002894 },
  { "USBB2_ULPITLL_DAT0_DAT1", 0x4A002898 },
  { "USBB2_ULPITLL_DAT2_DAT3", 0x4A00289C },
  { "USBB2_ULPITLL_DAT4_DAT5", 0x4A0028A0 },
  { "USBB2_ULPITLL_DAT6_DAT7", 0x4A0028A4 },
  { "USBB1_ULPIPHY_CLK_NXT", 0x4A0028A8 },
  { "USBB1_ULPIPHY_DIR_STP", 0x4A0028AC },
  { "USBB1_ULPIPHY_DAT0_DAT1", 0x4A0028B0 },
  { "USBB1_ULPIPHY_DAT2_DAT3", 0x4A0028B4 },
  { "USBB1_ULPIPHY_DAT4_DAT5", 0x4A0028B8 },
  { "USBB1_ULPIPHY_DAT6_DAT7", 0x4A0028BC },
  { "USBB1_HSIC_DAT_STR", 0x4A0028C0 },
  { "USBB2_HSIC_DAT_STR", 0x4A0028C4 },
  { "USBB3_HSIC_STR", 0x4A0029DC },
  { "USBB3_HSIC_DAT", 0x4A0029E0 },

  // ---	TLL Module		---
  // -------------------------------------
  { "USBTLL_REVISION", 0x4A062000 },
  { "USBTLL_HWINFO", 0x4A062004 },
  { "USBTLL_SYSCONFIG", 0x4A062010 },
  { "USBTLL_SYSSTATUS", 0x4A062014 },
  { "USBTLL_IRQSTATUS", 0x4A062018 },
  { "USBTLL_IRQENABLE", 0x4A06201C },
  { "TLL_SHARED_CONF", 0x4A062030 },
  { "TLL_CHANNEL_CONF_i (0)", 0x4A062040 },
  { "TLL_CHANNEL_CONF_i (1)", 0x4A062044 },
  { "TLL_CHANNEL_CONF_i (2)", 0x4A062048 },


  // ---	UHH_config Module	---
  // -------------------------------------
  { "UHH_REVISION", 0x4A064000 },
  { "UHH_HWINFO", 0x4A064004 },
  { "UHH_SYSCONFIG", 0x4A064010 },
  { "UHH_SYSSTATUS", 0x4A064014 },
  { "UHH_HOSTCONFIG", 0x4A064040 },
  { "UHH_DEBUG_CSR", 0x4A064044 },


  // ---	EHCI Module		---
  // -------------------------------------
  { "HCCAPBASE", 0x4A064C00 },
  { "HCSPARAMS", 0x4A064C04 },
  { "HCCPARAMS", 0x4A064C08 },
  { "USBCMD", 0x4A064C10 },
  { "USBSTS", 0x4A064C14 },
  { "USBINTR", 0x4A064C18 },
  { "FRINDEX", 0x4A064C1C },
  { "CTRLDSSEGMENT", 0x4A064C20 },
  { "PERIODICLISTBASE", 0x4A064C24 },
  { "ASYNCLISTADDR", 0x4A064C28 },
  { "CONFIGFLAG", 0x4A064C50 },
  { "PORTSC_0", 0x4A064C54 },
  { "PORTSC_1", 0x4A064C58 },
  { "PORTSC_2", 0x4A064C5C },
  { "INSNREG00", 0x4A064C90 },
  { "INSNREG01", 0x4A064C94 },
  { "INSNREG02", 0x4A064C98 },
  { "INSNREG03", 0x4A064C9C },
  { "INSNREG04", 0x4A064CA0 },
  { "INSNREG05_UTMI", 0x4A064CA4 },
  { "INSNREG06", 0x4A064CA8 },
  { "INSNREG07", 0x4A064CAC },
  { "INSNREG08", 0x4A064CB0 },


  // ---	CKGEN_CM2		---
  // -------------------------------------
  { "CM_CLKSEL_USB_60MHZ", 0x4A008104 },
  { "CM_CLKMODE_DPLL_USB", 0x4A008180 },
  { "CM_IDLEST_DPLL_USB", 0x4A008184 },
  { "CM_AUTOIDLE_DPLL_USB", 0x4A008188 },
  { "CM_CLKSEL_DPLL_USB", 0x4A00818C },
  { "CM_DIV_M2_DPLL_USB", 0x4A008190 },
  { "CM_SSC_DELTAMSTEP_DPLL_USB", 0x4A0081A8 },
  { "CM_SSC_MODFREQDIV_DPLL_USB", 0x4A0081AC },
  { "CM_CLKDCOLDO_DPLL_USB", 0x4A0081B4 },


  // ---	L3INIT_CM2		---
  // -------------------------------------
  { "CM_L3INIT_CLKSTCTRL", 0x4A009300 },
  { "CM_L3INIT_HSUSBHOST_CLKCTRL", 0x4A009358 },
  { "CM_L3INIT_HSUSBTLL_CLKCTRL", 0x4A009368 }
};

#endif

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

DEVICE_POOL_DEFN (usb_dev_req, usb_dev_req_t, 4, 64);

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
  DLOG_NO_PREFIX(1, "  bmAttributes=%#x %s%sbMaxPower=%dmA\n",
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
                 d->bLength, d->bDescriptorType, GETBITS(d->bEndpointAddress, 0, 4),
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

#define EHCI_BASE (_mappings[1].vstart + 0xC00) // 0x4A064C00
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
#define EHCI_TD_TOKEN_H BIT(6)

/* Transfer Descriptor (section 3.5) */
struct _ehci_td {
  volatile u32 next;
  volatile u32 alt_next;
  volatile u32 token;
  volatile u32 buf[5];
  /* HW part ^ */
  struct _ehci_td *nextv;
} __attribute__((aligned (32)));
typedef struct _ehci_td ehci_td_t;

/* memory pool of TDs for dynamic alloc */
DEVICE_POOL_DEFN (ehci_td, ehci_td_t, 16, 64);

/* ************************************************** */

void dump_indent(indent)
{
  u32 i;
  for(i=0;i<indent;i++) DLOG_NO_PREFIX(1, " ");
}

void dump_td(ehci_td_t *td, u32 indent)
{
#ifdef USE_VMM
  /* FIXME: physical->virtual */
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "TD: %#x %#x %#x (%s) [%#x %#x %#x %#x %#x]\n",
                 td->next, td->alt_next, td->token,
                 td->token & EHCI_TD_TOKEN_A ? "A" : "a", /* active? */
                 td->buf[0], td->buf[1], td->buf[2],
                 td->buf[3], td->buf[4]);
#else
  dump_indent(indent);
  DLOG_NO_PREFIX(1, "TD: %#x %#x %#x (%s) [%#x %#x %#x %#x %#x]\n",
                 td->next, td->alt_next, td->token,
                 td->token & EHCI_TD_TOKEN_A ? "A" : "a", /* active? */
                 td->buf[0], td->buf[1], td->buf[2],
                 td->buf[3], td->buf[4]);
  if(!(td->next & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (td->next & EHCI_TD_PTR_MASK), indent);
#endif
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
#endif
}

/* ************************************************** */
/* GPIO */

static u32 omap4460_gpio_base[6];
/*  = {
 *   0x4A310000, 0x48055000, 0x48057000,
 *   0x48059000, 0x4805B000, 0x4805D000
 * }; */

void omap4460_gpio_init(void)
{
  omap4460_gpio_base[0] = (u32) _mappings[5].vstart;
  omap4460_gpio_base[1] = (u32) _mappings[0].vstart + 0x5000;
  omap4460_gpio_base[2] = (u32) _mappings[0].vstart + 0x7000;
  omap4460_gpio_base[3] = (u32) _mappings[0].vstart + 0x9000;
  omap4460_gpio_base[4] = (u32) _mappings[0].vstart + 0xB000;
  omap4460_gpio_base[5] = (u32) _mappings[0].vstart + 0xD000;
}

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
DEVICE_POOL_DEFN(usb_device,usb_device_t,8,32);

void usb_clear_device(usb_device_t *d)
{
  d->address = 0;
  d->dev_desc.bMaxPacketSize0 = 8; /* default */
}

typedef u8 scratch_buf_t[128];
DEVICE_POOL_DEFN(scratch_buf,scratch_buf_t,4,4);

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

status ehci_attach_td(ehci_qh_t *qh, ehci_td_t *td)
{
  qh->current_td = EHCI_TD_PTR_T; /* clear current TD */
  qh->overlay[0] = 0;             /* clear status of overlay */
#ifdef USE_VMM
  u32 phys;
  if(vmm_get_phys_addr(td, &phys) == OK) {
    qh->next_td = phys;
    return OK;
  }
  return EINVALID;
#else
  qh->next_td = (u32) td;         /* set new value for next TD */
  return OK;
#endif
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

u32 ehci_fill_td(ehci_td_t *td, u32 pidcode, void *data, s32 len, ehci_td_t *prev_td)
{
#ifdef USE_VMM
  u32 paddr = 0;
#endif
  u32 bytes = (len > 0x5000 ? 0x5000 : len); /* 0x5000 is max for single TD */
  memset(td, 0, 1<<5);
  td->next = EHCI_TD_PTR_T;
  td->nextv = NULL;
  if(prev_td != NULL) {
#ifdef USE_VMM
    if(vmm_get_phys_addr((void *) td, &paddr) != OK) {
      DLOG(1, "vmm_get_phys_addr returned NOT OK\n");
      return 0;                 /* FIXME: what to do */
    }
    prev_td->next = paddr;
    prev_td->nextv = td;
#else
    prev_td->next = (u32) td;
    prev_td->nextv = td;
#endif
  }
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
    if(vmm_get_phys_addr((void *) addr, &paddr) != OK) {
      DLOG(1, "vmm_get_phys_addr returned NOT OK\n");
      return 0;                 /* FIXME: what to do */
    }
    td->buf[cpage] = paddr;
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

status ehci_td_chain_active(ehci_td_t *td)
{
  while(td != NULL) {
    if((td->token & EHCI_TD_TOKEN_A) == 0 && (td->next & EHCI_TD_PTR_T) == 1)
      return EINVALID;          /* not active */
    if((td->token & EHCI_TD_TOKEN_H) != 0)
      return EINVALID;          /* halted */

    if((td->next & EHCI_TD_PTR_T) == 1)
      break;                    /* done */
    td = (ehci_td_t *) td->nextv;
  }
  return OK;
}

status ehci_td_chain_status(ehci_td_t *td)
{
  while(td != NULL) {
    if((td->token & EHCI_TD_TOKEN_A) == 0 && (td->next & EHCI_TD_PTR_T) == 1)
      return OK;                /* not active */
    if((td->token & EHCI_TD_TOKEN_H) != 0)
      return EDATA;             /* halted */

    if((td->next & EHCI_TD_PTR_T) == 1)
      break;                    /* done */
    td = (ehci_td_t *) td->nextv;
  }
  return EINCOMPLETE;
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

void NAKED ehci_irq_handler(u32 v)
{
  DLOG(1, "ehci_irq_handler: IRQ\n");
  irq_state = 3;                /* Once IRQ state is 3, any interrupt
                                 * will cause the process of context
                                 * restoration to take place,
                                 * returning us to our previous place
                                 * in the program. */

  ASM("B restore_saved_context");
}

/* macros to help use proper device memory for TDs */
#define ALLOC_TDS(devr,td,n)                            \
  usb_dev_req_t *devr = usb_dev_req_pool_alloc();       \
  ehci_td_t *td[n];                                     \
  do {                                                  \
    int i;                                              \
    if(devr == NULL) return ENOSPACE;                   \
    for(i=0;i<n;i++) {                                  \
      td[i] = ehci_td_pool_alloc();                     \
      if(td[i] == NULL) {                               \
        DLOG(1, "ALLOC_TDS: unable to alloc TD.\n");    \
        usb_dev_req_pool_free(devr);                    \
        for(;i>0;--i)                                   \
            ehci_td_pool_free(td[i-1]);                 \
        return ENOSPACE; } } } while (0)

#define FREE_TDS(devr,tdvar,n) do {             \
    int i; usb_dev_req_pool_free(devr);         \
    for(i=0;i<n;i++) ehci_td_pool_free(td[i]);  \
  } while(0)


/* data must be in device memory */
static status usb_control_write(usb_device_t *d, u32 bmRequestType, u32 bRequest, u32 wValue, u32 wIndex, u32 wLength, void *data)
{
  ALLOC_TDS(devr,td,3);

  usb_device_request(devr, bmRequestType, bRequest, wValue, wIndex, wLength);

  /* setup qTDs */
  ehci_detach_td(&d->qh);
  ehci_fill_td(td[0], EHCI_PIDCODE_SETUP, devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  if(ehci_fill_td(td[1], EHCI_PIDCODE_OUT, data, wLength, td[0]) != 0) { /* DATA stage */
    FREE_TDS(devr,td,3);
    return ENOSPACE;                                      /* FIXME: cannot handle control xfers longer than 0x5000 bytes */
  }
  ehci_fill_td(td[2], EHCI_PIDCODE_IN, NULL, 0, td[1]); /* STATUS stage */

  /* queue it */
  ehci_attach_td(&d->qh, td[0]);

  /* wait for it */
  while(ehci_td_chain_active(td[0]) == OK);

  ehci_detach_td(&d->qh);       /* detach before unwinding TD memory */

  status s = ehci_td_chain_status(td[0]);
  FREE_TDS(devr,td,3);
  return s;
}

/* data must be in device memory */
static status usb_control_read(usb_device_t *d, u32 bmRequestType, u32 bRequest, u32 wValue, u32 wIndex, u32 wLength, void *data)
{
  ALLOC_TDS(devr,td,3);

  usb_device_request(devr, bmRequestType, bRequest, wValue, wIndex, wLength);

  /* setup qTDs */
  ehci_detach_td(&d->qh);
  ehci_fill_td(td[0], EHCI_PIDCODE_SETUP, devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  if(ehci_fill_td(td[1], EHCI_PIDCODE_IN, data, wLength, td[0]) != 0) { /* DATA stage */
    FREE_TDS(devr,td,3);
    return ENOSPACE;                                      /* FIXME: cannot handle control xfers longer than 0x5000 bytes */
  }
  ehci_fill_td(td[2], EHCI_PIDCODE_OUT, NULL, 0, td[1]); /* STATUS stage */

  /* queue it */
  ehci_attach_td(&d->qh, td[0]);

  /* wait for it */
  while(ehci_td_chain_active(td[0]) == OK);

  ehci_detach_td(&d->qh);       /* detach before unwinding TD memory */

  status s = ehci_td_chain_status(td[0]);
  FREE_TDS(devr,td,3);
  return s;
}

static status usb_control_nodata(usb_device_t *d, u32 bmRequestType, u32 bRequest, u32 wValue, u32 wIndex)
{
  ALLOC_TDS(devr,td,2);

  usb_device_request(devr, bmRequestType, bRequest, wValue, wIndex, 0);

  /* setup qTDs */
  ehci_detach_td(&d->qh);
  ehci_fill_td(td[0], EHCI_PIDCODE_SETUP, devr, sizeof(usb_dev_req_t), NULL); /* SETUP stage */
  ehci_fill_td(td[1], EHCI_PIDCODE_IN, NULL, 0, td[0]); /* STATUS stage */

  /* queue it */
  ehci_attach_td(&d->qh, td[0]);

  /* wait for it */
  while(ehci_td_chain_active(td[0]) == OK);

  ehci_detach_td(&d->qh);       /* detach before unwinding TD memory */

  status s = ehci_td_chain_status(td[0]);
  FREE_TDS(devr,td,2);
  return s;
}

static status usb_set_feature(usb_device_t *d, u32 bmRequestType, u32 wValue, u32 wIndex)
{
  return usb_control_nodata(d, bmRequestType, USB_SET_FEATURE, wValue, wIndex);
}

static status usb_clear_feature(usb_device_t *d, u32 bmRequestType, u32 wValue, u32 wIndex)
{
  return usb_control_nodata(d, bmRequestType, USB_CLEAR_FEATURE, wValue, wIndex);
}

static status usb_get_status(usb_device_t *d, u32 bmRequestType, u32 wValue, u32 wIndex, u32 wLength, void *buf)
{
  scratch_buf_t *sb = scratch_buf_pool_alloc(); if(sb == NULL) return ENOSPACE;
  status s = usb_control_read(d, bmRequestType, USB_GET_STATUS, wValue, wIndex, wLength, sb);
  if(s == OK) memcpy(buf, sb, wLength);
  scratch_buf_pool_free(sb);
  return s;
}

status usb_set_configuration(usb_device_t *d, u32 cfgval)
{
  return usb_control_nodata(d, USB_REQ_HOST_TO_DEVICE, USB_SET_CONFIGURATION, cfgval, 0);
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
  u32 n, pwr_wait;
} usb_hub_port_t;

static void usb_hub_port_reset(usb_port_t *p)
{
  usb_hub_port_t *hubp = container_of(p,usb_hub_port_t,port);

  /* ** assert reset ** */
  usb_set_feature(hubp->dev, USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                  USB_HUB_PORT_RESET, hubp->n);

  /* FIXME: check status of C_RESET instead */
  ehci_microframewait(10 << 3); /* wait 10 ms */

  /* ** de-assert reset ** */
  usb_clear_feature(hubp->dev, USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                    USB_HUB_PORT_RESET, hubp->n);

  /* power-on-to-power-good wait */
  ehci_microframewait(hubp->pwr_wait << 3);
}

static u32 usb_hub_port_get_status(usb_port_t *p)
{
  usb_hub_port_t *hubp = container_of(p,usb_hub_port_t,port);
  u32 portstatus;
  status retval;

  retval = usb_get_status(hubp->dev, USB_REQ_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                          0, hubp->n, 4, &portstatus);

  if(retval == OK)
    return portstatus;
  else
    return 0;
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

static status usb_hub_get_descriptor(usb_device_t *d, usb_hub_desc_t *hubd)
{
#define USB_HUB_DESCRIPTOR_TYPE 0x29
  scratch_buf_t *sb = scratch_buf_pool_alloc(); if(sb == NULL) return ENOSPACE;
  status s = usb_control_read(d, USB_REQ_DEVICE_TO_HOST | USB_REQ_TYPE_CLASS, USB_GET_DESCRIPTOR,
                              USB_HUB_DESCRIPTOR_TYPE << 8, 0, sizeof(usb_hub_desc_t), sb);
  if(s == OK) memcpy(hubd, sb, sizeof(usb_hub_desc_t));
  scratch_buf_pool_free(sb);
  return s;
}

static status usb_hub_port_power_on(usb_port_t *p)
{
  usb_hub_port_t *hubp = container_of(p,usb_hub_port_t,port);
  return usb_set_feature(hubp->dev, USB_REQ_HOST_TO_DEVICE | USB_REQ_TYPE_CLASS | USB_REQ_RECIP_OTHER,
                         USB_HUB_PORT_POWER, hubp->n);
}

static status setup_hub(usb_device_t *d, u32 cfgval, u32 ept)
{
  usb_device_t *newd;
  usb_hub_desc_t hubd;
  status retval;

  usb_set_configuration(d, cfgval);

  retval = usb_hub_get_descriptor(d, &hubd);
  if(retval != OK) return retval;

  dump_usb_hub_desc(&hubd);

  /* power on port */

  u32 portn = 1;
  u32 portstatus = 0;
  u32 attempts;
  for(portn = 1; portn <= hubd.bNbrPorts; portn++) {
    usb_hub_port_t p = { .port = HUBPORT, .n = portn, .dev = d, .pwr_wait = hubd.bPwrOn2PwrGood << 1 };

    attempts = 0;
    portstatus = 0;
    while((portstatus & USB_HUB_PORT_STAT_POWER) == 0 && attempts < 5) {
      DLOG(1, "Attempting to power on port %d\n", portn);

      usb_hub_port_power_on((usb_port_t *) &p);

      /* power-on-to-power-good wait */
      ehci_microframewait((hubd.bPwrOn2PwrGood << 1) << 3);

      portstatus = usb_hub_port_get_status((usb_port_t *) &p);

      attempts++;
      if(attempts >= 5) break;
    }

    if(attempts >= 5) continue;

    DLOG(1, "portstatus(%d)=%#x\n", portn, portstatus);

    if(portstatus & 1) {
      /* something connected */
      void ehci_enumerate(usb_port_t *, usb_device_t *);
      newd = usb_device_pool_alloc();
      usb_init_subdevice(newd, 0, d);

      ehci_enumerate((usb_port_t *) &p, newd);

    }
  }

  return OK;
}

static status probe_hub(usb_device_t *d)
{
  status retval;
  usb_cfg_desc_t *cfgd;
  u32 cfgidx, ifidx, eptidx;
  scratch_buf_t *sb = scratch_buf_pool_alloc();
  if(sb == NULL) return ENOSPACE;

  for(cfgidx=0;cfgidx<d->dev_desc.bNumConfigurations;cfgidx++) {
    /* get config descriptor; mostly to find wTotalLength */
    retval = usb_control_read(d, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                              (USB_TYPE_CFG_DESC << 8) | cfgidx,
                              0, sizeof(usb_cfg_desc_t), sb);
    if(retval != OK)
      DLOG(1, "Get configuration descriptor retval=%d\n", retval);

    cfgd = (usb_cfg_desc_t *) sb;
    dump_usb_cfg_desc(cfgd);

    if(cfgd->wTotalLength > sizeof(scratch_buf_t)) {
      /* FIXME: figure out a better way to handle really long config descriptors */
      DLOG(1, "probe_hub: configuration too long=%d.\n", cfgd->wTotalLength);
      scratch_buf_pool_free(sb);
      return EINVALID;
    }

    /* get config descriptor + interfaces + endpoints */
    retval = usb_control_read(d, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                              (USB_TYPE_CFG_DESC << 8) | cfgidx,
                              0, cfgd->wTotalLength, sb);
    if(retval != OK)
      DLOG(1, "Get configuration descriptor retval=%d\n", retval);

    void *ptr = ((void *) sb) + sizeof(usb_cfg_desc_t);

    for(ifidx=0; ifidx<cfgd->bNumInterfaces; ifidx++) {
      usb_if_desc_t *ifd = (usb_if_desc_t *) ptr;
      dump_usb_if_desc(ifd);
      ptr += sizeof(usb_if_desc_t);
      for(eptidx=0; eptidx<ifd->bNumEndpoints; eptidx++) {
        usb_ept_desc_t *eptd = (usb_ept_desc_t *) ptr;
        dump_usb_ept_desc(eptd);

        if(ifd->bInterfaceClass == 9) {
          u32 cv = cfgd->bConfigurationValue;
          DLOG(1, "Found hub cfgval=%d\n", cv);
          scratch_buf_pool_free(sb);
          return setup_hub(d, cv, 0);
        }

        ptr += sizeof(usb_ept_desc_t);
      }
    }
  }

  scratch_buf_pool_free(sb);
  return EINVALID;
}

/* ************************************************** */

u32 unused_address = 1;         /* store next unused address */

status ehci_setup_new_device(usb_device_t *usbd)
{
  ehci_set_qh_max_packet_size(&usbd->qh, 8); /* start with 8 byte max packet size */

  /* create SET_ADDRESS control request */
  u32 new_addr = unused_address++;
  if(usb_control_nodata(usbd, USB_REQ_HOST_TO_DEVICE, USB_SET_ADDRESS, new_addr, 0) != OK) {
    DLOG(1, "Unable to SET_ADDRESS\n");
    /* FIXME: should I reuse address? */
    return EINVALID;
  }

  usbd->address = new_addr;
  ehci_set_qh_address(&usbd->qh, new_addr);

  DLOG(1, "ehci_setup_new_device new_addr=%d\n", new_addr);

  /* get_descriptor: device descriptor (expect BABBLE) */
  usb_control_read(usbd, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                   USB_TYPE_DEV_DESC << 8, 0, sizeof(usb_dev_desc_t), &usbd->dev_desc);

  /* get it again, but with correct max packet size */
  ehci_set_qh_max_packet_size(&usbd->qh, usbd->dev_desc.bMaxPacketSize0);

  /* get_descriptor: device descriptor */
  if(usb_control_read(usbd, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                      USB_TYPE_DEV_DESC << 8, 0, sizeof(usb_dev_desc_t), &usbd->dev_desc) != OK) {
    DLOG(1, "ehci_setup_new_device: Unable to read device descriptor\n");
    /* FIXME: should I reuse address? */
    return EINVALID;
  }

  DLOG(1, "usb_dev_desc=%#x\n", &usbd->dev_desc);
  dump_usb_dev_desc(&usbd->dev_desc);

  /* FIXME: for now */
  return probe_hub(usbd);
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
  int i;
  _irq_status_table = svc1();
  DLOG(1, "hsusbhc_init _irq_status_table=%#x _kernel_saved_context=%#x\n", _irq_status_table, _kernel_saved_context);
  DLOG(1, "irq_state=%d\n", irq_state);
  for(i=0; i<sizeof(_mappings)/sizeof(mapping_t) && _mappings[i].pstart; i++)
    DLOG(1, "mapping: %#x => %#x desc=\"%s\"\n", _mappings[i].pstart, _mappings[i].vstart, _mappings[i].desc);

  for(i=0; i<sizeof(_ipcmappings)/sizeof(ipcmapping_t) && _ipcmappings[i].type; i++)
    DLOG(1, "ipcmapping: address=%#x desc=\"%s\"\n", _ipcmappings[i].address, _ipcmappings[i].name);

  u32 *ptr = (u32 *)_ipcmappings[0].address;
  DLOG(1, "ptr[0]=%#x\n", ptr[0]);

  /* initialize base addresses of GPIO interface */
  omap4460_gpio_init();

  /* ************************************************** */
  /* SCRM */
  /* (necessary) */
#define SCRM_BASE (_mappings[6].vstart) // 0x4A30A000
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
#define CM_BASE (_mappings[2].vstart) // 0x4a009000


  /* p978 */
  /* full-speed, unnecessary */
  //R(CM_BASE + 0x3D0) |= BIT(1);
  //DLOG(1, "CM_L3INIT_FSUSB_CLKCTRL=%#x\n", R(0x4A0093D0));
  //DLOG(1, "CM_L3INIT_FSUSB_CLKCTRL=%#x\n", R(0x4A0093D0));

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
  R(CM_BASE + 0x368) |= BIT(0);
  DLOG(1, "CM_L3INIT_HSUSBTLL_CLKCTRL=%#x\n", R(CM_BASE + 0x368));

  //R(0x4A009300) |= BIT(1);
  DLOG(1, "CM_L3INIT_CLKSTCTRL=%#x\n", R(CM_BASE + 0x300));

  /* ************************************************** */
  /* USBTLL p5110 */
  /* (optional) */
#define USBTLL_BASE (_mappings[3].vstart) // 0x4a062000
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
#define HSUSBHOST_BASE (_mappings[4].vstart) // 0x4a064000
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

#ifdef USE_VMM
  u32 paddr = 0;
  if(vmm_get_phys_addr((void *) &usbd->qh, &paddr) != OK) {
    DLOG(1, "vmm_get_phys_addr returned NOT OK\n");
    return;                 /* FIXME: what to do */
  }
  R(EHCI_ASYNCLISTADDR) = paddr;
#else
  R(EHCI_ASYNCLISTADDR) = (u32) &usbd->qh;
#endif

  DLOG(1, "EHCI_ASYNCLISTADDR=%#x\n", R(EHCI_ASYNCLISTADDR));

  /* can't seem to enable async schedule until something is on the list */

  /* enable asynchronous schedule */
  R(EHCI_CMD) |= BIT(5);

#ifdef DEBUGREGS
  {
    int i;
    for(i=0;i<(sizeof(omapregs)/sizeof(struct omapregs));i++)
      DLOG(1, "%s = %#x\n", omapregs[i].desc, R(omapregs[i].addr));
  }
#endif

  ehci_enumerate(&rootport[0].port, usbd);
  //ehci_enumerate(&rootport[1].port, &qh1);
  //ehci_enumerate(&rootport[2].port, &qh1);

  DLOG(1, "_irq_status_table[77] = %#x\n", _irq_status_table[77]);
  DLOG(1, "irq_state=%d\n", irq_state);
  DLOG(1, "initialization complete\n");
  ehci_microframewait(1000 << 3); /* 1000 ms */
}

#else
#error "High Speed USB Host Controller not implemented"
#endif

#undef R

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
