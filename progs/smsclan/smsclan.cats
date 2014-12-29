#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include<user.h>
//#include<pool.h>
#include "pointer.cats"
#include "integer.cats"
#include "virtual.h"

/* unsigned int _scheduler_capacity = 3 << 14;
 * unsigned int _scheduler_period = 12 << 14; */
unsigned int _scheduler_capacity = 1 << 11;
unsigned int _scheduler_period = 4 << 11;
unsigned int _scheduler_affinity = 1;

ipcmapping_t _ipcmappings[] = {
  { IPC_SEEK, "ehci_info", IPC_READ, "fixedslot", 1, NULL },
  { IPC_SEEK, "ehci_usbdevices", IPC_READ | IPC_WRITE, "usbdevices", 1, NULL },
  {0}
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
  /* FIXME: use nextv */
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

#if 0
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

typedef struct {
  ehci_qh_t qh;
  u32 address;
  usb_dev_desc_t dev_desc;
} usb_device_t;

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

void usb_device_request(usb_dev_req_t *req, u32 bmrt, u32 br, u32 wv, u32 wi, u32 l)
{
  req->bmRequestType = bmrt;
  req->bRequest = br;
  req->wValue = wv;
  req->wIndex = wi;
  req->wLength = l;
}

void ehci_detach_td(ehci_qh_t *qh)
{
  qh->current_td = EHCI_QH_PTR_T;
  qh->next_td = EHCI_QH_PTR_T;
  qh->alt_td = EHCI_QH_PTR_T;
}

status ehci_attach_td(ehci_qh_t *qh, ehci_td_t *td)
{
  qh->current_td = EHCI_QH_PTR_T; /* clear current TD */
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

status get_usb_dev_desc(usb_device_t *usbd)
{
  /* get_descriptor: device descriptor */
  status s;
  if((s=usb_control_read(usbd, USB_REQ_DEVICE_TO_HOST, USB_GET_DESCRIPTOR,
                         USB_TYPE_DEV_DESC << 8, 0, sizeof(usb_dev_desc_t), &usbd->dev_desc)) != OK) {
    DLOG(1, "ehci_setup_new_device: Unable to read device descriptor (s=%d)\n",s);
    /* FIXME: should I reuse address? */
    return s;
  }

  return OK;
}

void test(void)
{
  usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  while(*((volatile u32 *) (_ipcmappings[1].address)) == 0);
  DLOG(1, "smsclan ****************** usbdevices[0]=%#x\n", *((u32 *) (_ipcmappings[1].address)));
  status s = get_usb_dev_desc(&usbd[0]);
  DLOG(1, "smsclan get_usb_dev_desc(&usbd[0]) = %d\n", s);
  if(s == OK) {
    dump_usb_dev_desc(&usbd->dev_desc);
  }
}

#endif  /* 0 */


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
