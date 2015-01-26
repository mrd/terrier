#ifndef _USERLIB_USB_CATS_
#define _USERLIB_USB_CATS_

#include "ats_types.h"
#include "pats_ccomp_basics.h"
#include "pats_ccomp_typedefs.h"
#include "types.h"
#include <user.h>
#include <pool.h>
#include "virtual.h"
#include "integer.cats"
#include "pointer.cats"

#define atsrefarg0_type(hit) hit
#define atsrefarg1_type(hit) atstype_ref

extern ipcmapping_t _ipcmappings[];

/* ************************************************** */

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
POOL_PROTO(usb_dev_req, usb_dev_req_t);

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

#define make_Request(x) ((u32) (x))

#define make_RequestType(x,y,z) (SETBITS(((u32) (x)),7,1) | SETBITS(((u32) (y)),5,2) | SETBITS(((u32) (z)),0,5))

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

#define EHCI_TD_PTR_MASK (~BITMASK(5, 0))
#define EHCI_TD_PTR_T BIT(0)
#define EHCI_TD_TOKEN_A BIT(7)
#define EHCI_TD_TOKEN_H BIT(6)

#define EHCI_PIDCODE_OUT   0
#define EHCI_PIDCODE_IN    1
#define EHCI_PIDCODE_SETUP 2

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
POOL_PROTO (ehci_td, ehci_td_t);

static inline void ehci_td_chain_free(ehci_td_t *td)
{
  while(td != NULL) {
    ehci_td_t *next = td->nextv;
    ehci_td_pool_free(td);
    td = next;
  }
}

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
  volatile u32 overlay[8];      /* used by hardware */
  volatile u32 priv1;           /* extra space usable by drivers */
  volatile u32 priv2;
} __attribute__((aligned (32)));
typedef struct _ehci_qh ehci_qh_t;

/* ************************************************** */

/* same as in ehci.cats */
#define USB_DEVICE_NUM_QH 4
typedef struct {
  ehci_qh_t qh[USB_DEVICE_NUM_QH];
  //  ehci_td_t *attached[USB_DEVICE_NUM_QH]; /* currently attached TD chain if exists */
  u32 address, flags, num_qh;
  usb_dev_desc_t dev_desc;
} usb_device_t;
#define QH(usbd,i) ((usbd).qh[(i)])
#define FOR_QH(var,usbd) do { ehci_qh_t *var; \
  for(var = &(usbd).qh[0]; var < &(usbd).qh[(usbd).num_qh]; var++)
#define END_FOR_QH } while (0)
#define ATTACHED(usbd,i) ((ehci_td_t *) ((usbd).qh[(i)].priv1))
#define SET_ATTACHED(usbd,i,v) ((usbd).qh[(i)].priv1) = (u32) (v)
#define URB_ATTACHED(urb) ((ehci_td_t *) ((*(urb)).priv1))
#define SET_URB_ATTACHED(urb,v)  ((*(urb)).priv1) = (u32) (v)
#define URB_QH(urb) (*(urb))

/* ************************************************** */

void debuglog(u32 noprefix, u32 lvl, const char *fmt, ...);
#define debuglog_no_prefix(lvl,fmt,...) debuglog(1,lvl,fmt,##__VA_ARGS__)

static void dump_usb_dev_req(usb_dev_req_t *d)
{
  debuglog(0, 1, "smsclan USB_DEV_REQ:\n");
  debuglog_no_prefix(1, "  bmRequestType=%#.2x bRequest=%#.2x wValue=%#.4x wIndex=%#.4x wLength=%d\n",
                 d->bmRequestType, d->bRequest, d->wValue, d->wIndex, d->wLength);
}

static void dump_td(ehci_td_t *td, u32 indent)
{
  u32 i;
  for(i=0;i<indent;i++) debuglog_no_prefix(1, " ");

#ifdef USE_VMM
  /* FIXME: physical->virtual */

  int pid = GETBITS(td->token, 8, 2);
  char *pidcode = (pid == 0 ? "OUT" : (pid == 1 ? "IN" : (pid == 2 ? "SETUP" : "reserved")));
  debuglog_no_prefix(1, "TD: %#.8x %#.8x %#.8x (%s%s%s%s%s%s%s%s) (CERR=%d) %s [%#x %#x %#x %#x %#x]\n",
                 td->next, td->alt_next, td->token,
                 td->token & EHCI_TD_TOKEN_A ? "A" : "", /* active? */
                 td->token & EHCI_TD_TOKEN_H ? "H" : "", /* halted? */
                 td->token & BIT(5)          ? "D" : "", /* data buffer error */
                 td->token & BIT(4)          ? "B" : "", /* babble */
                 td->token & BIT(3)          ? "X" : "", /* transaction error */
                 td->token & BIT(2)          ? "M" : "", /* missed microframe error */
                 td->token & BIT(1)          ? "S" : "", /* split transaction state */
                 td->token & BIT(0)          ? "P" : "", /* ping state/ERR */
                 GETBITS(td->token, 10, 2),
                 pidcode,
                 td->buf[0], td->buf[1], td->buf[2],
                 td->buf[3], td->buf[4]);
  /* FIXME: use nextv */
#else

  int pid = GETBITS(td->token, 8, 2);
  char *pidcode = (pid == 0 ? "OUT" : (pid == 1 ? "IN" : (pid == 2 ? "SETUP" : "reserved")));
  debuglog_no_prefix(1, "TD: %#.8x %#.8x %#.8x (%s%s%s%s%s%s%s%s) (CERR=%d) %s [%#x %#x %#x %#x %#x]\n",
                 td->next, td->alt_next, td->token,
                 td->token & EHCI_TD_TOKEN_A ? "A" : "", /* active? */
                 td->token & EHCI_TD_TOKEN_H ? "H" : "", /* halted? */
                 td->token & BIT(5)          ? "D" : "", /* data buffer error */
                 td->token & BIT(4)          ? "B" : "", /* babble */
                 td->token & BIT(3)          ? "X" : "", /* transaction error */
                 td->token & BIT(2)          ? "M" : "", /* missed microframe error */
                 td->token & BIT(1)          ? "S" : "", /* split transaction state */
                 td->token & BIT(0)          ? "P" : "", /* ping state/ERR */
                 GETBITS(td->token, 10, 2),
                 pidcode,
                 td->buf[0], td->buf[1], td->buf[2],
                 td->buf[3], td->buf[4]);
  if(!(td->next & EHCI_TD_PTR_T))
    dump_td((ehci_td_t *) (td->next & EHCI_TD_PTR_MASK), indent);
#endif
}

/* ************************************************** */

#define usb_device_num_configurations(d) (d)->dev_desc.bNumConfigurations;

/* ************************************************** */

static inline void *usb_acquire_device(int i)
{
  /* FIXME: more comprehensive way of picking ipc mappings */
  usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  FOR_QH(q,usbd[i]) {
    q->current_td = EHCI_QH_PTR_T;
    q->next_td = EHCI_QH_PTR_T;
    q->alt_td = EHCI_QH_PTR_T;
  } END_FOR_QH;
  return (void *) &usbd[i];
}

#define usb_release_device(usbd)

typedef ehci_qh_t *urb_t;

static inline status usb_device_alloc_urb(usb_device_t *usbd, void **_urb)
{
  urb_t *urb = (urb_t *) _urb;
  FOR_QH(q,*usbd) {
    if(q->priv2 == 0) {
      q->priv2 = 1;
      *urb = q;
      return OK;
    }
  } END_FOR_QH;
  return ENOSPACE;
}

static inline void usb_device_release_urb(usb_device_t *usbd, urb_t urb)
{
  urb->priv1 = 0;
  urb->priv2 = 0;
}

#define urb_clr_current_td(urb) URB_QH(urb).current_td = EHCI_QH_PTR_T
#define urb_clr_overlay_status(urb) URB_QH(urb).overlay[0] = 0
#define urb_clr_alt_td(urb) URB_QH(urb).alt_td = EHCI_QH_PTR_T
#define urb_set_next_td(urb,virt,phys) do { URB_QH(urb).next_td = (phys); SET_URB_ATTACHED((urb),(virt)); } while (0)

static inline ehci_td_t *urb_detach(urb_t urb)
{
  URB_QH(urb).current_td = EHCI_QH_PTR_T;
  URB_QH(urb).next_td = EHCI_QH_PTR_T;
  URB_QH(urb).alt_td = EHCI_QH_PTR_T;
  ehci_td_t *td = URB_ATTACHED(urb);
  SET_URB_ATTACHED(urb,NULL);
  return td;
}

/* ************************************************** */

#define ehci_td_set_next_td(td,vaddr,paddr) do { (td).next = (paddr); (td).nextv = (vaddr); } while (0)

static inline void _usb_device_request_fill(usb_dev_req_t *req, u32 bmrt, u32 br, u32 wv, u32 wi, u32 l)
{
  req->bmRequestType = bmrt;
  req->bRequest = br;
  req->wValue = wv;
  req->wIndex = wi;
  req->wLength = l;
}

#define _set_td_buf(td,i,paddr) ((ehci_td_t *) td)->buf[(i)] = (paddr)

static inline void _incr_td_page (ats_ref_type *_addr, ats_ref_type *_len)
{
  u32 addr = *((u32 *) _addr);
  s32 len =  *((s32 *) _len);
  u32 next = (addr & (~(PAGE_SIZE - 1))) + PAGE_SIZE;
  len -= next - addr;
  addr = next;
  *((u32 *) _addr) = addr;
  *((u32 *) _len) = (len < 0 ? 0 : len);
}

#define _clear_td(td) do { memset((td), 0, 1<<5); (td)->next = EHCI_TD_PTR_T; (td)->nextv = NULL; } while (0)
#define _setup_td(td,bytes,ioc,cpage,errcnt,pid,active) do {    \
    ((ehci_td_t *)(td))->alt_next = EHCI_TD_PTR_T;              \
    ((ehci_td_t *)(td))->token =                                \
      /* total bytes to transfer */                             \
      SETBITS((bytes), 16, 15) |                                \
      /* interrupt-on-complete */                               \
      ((ioc) ? BIT(15) : 0) |                                   \
      /* current page */                                        \
      SETBITS((cpage), 12, 3) |                                 \
      /* error counter */                                       \
      SETBITS((errcnt), 10, 2) |                                \
      /* token: PID code: 0=OUT 1=IN 2=SETUP */                 \
      SETBITS((pid), 8, 2) |                                    \
      /* status: active */                                      \
      ((active) ? BIT(7) : 0) |                                 \
      0;                                                        \
  } while(0)

/* ************************************************** */
/* EHCI TD traversals */

#define ehci_td_traversal_null0(td) NULL
#define ehci_td_traversal_null1(td, trav) NULL
#define ehci_td_traversal_start(td) td
#define ehci_td_traversal_next(trav,td,phys) do {       \
    (*((ehci_td_t **)(trav)))->nextv = (td);            \
    (*((ehci_td_t **)(trav)))->next = (phys);           \
    (*((ehci_td_t **)(trav))) = (td);                   \
  } while (0)

#define ehci_td_traversal_set_toggle(trav) (*((ehci_td_t **)(trav)))->token |= BIT(31)

/* ************************************************** */
/* USB transfer */
#define urb_transfer_completed(d) //dump_td(ATTACHED(*d,0), 8)


static inline status urb_transfer_chain_active(urb_t urb)
{
  ehci_td_t *td = URB_ATTACHED(urb);
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

static inline status urb_transfer_result_status(urb_t urb)
{
  ehci_td_t *td = URB_ATTACHED(urb);
  while(td != NULL) {
    //    if((td->token & (0xF << 2)) != 0) return EDATA;
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

static inline void urb_set_endpoint(ehci_qh_t *qh, u32 endpt, u32 maxpkt)
{
  u32 addr = GETBITS(qh->characteristics, 0, 7); // should be setup by EHCI module
  qh->characteristics =
    /* Max Packet Size */
    SETBITS(maxpkt, 16, 11) |
    /* Data toggle control (0=Use HC; 1=use TD) */
    // BIT(14) |
    /* endpoint speed: high */
    SETBITS(2, 12, 2) |
    /* endpoint number */
    SETBITS(endpt, 8, 4) |
    /* device address */
    SETBITS(addr, 0, 7) |
    0;
  qh->capabilities =
    /* Mult */
    SETBITS(3, 30, 2) |
    /* S-mask: 0 for async */
    SETBITS(0, 0, 8) |
    0;
}

#define urb_set_control_endpoint(usbd, urb) urb_set_endpoint(urb, 0, usbd->dev_desc.bMaxPacketSize0)

#define dump_urb(urb,i) dump_qh(urb,i)

#endif

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
