#ifndef _USB_CATS_
#define _USB_CATS_

#include "ats_types.h"
//#include "ats_basics.h"
#include "pats_ccomp_basics.h"
#include "types.h"
#include <user.h>
#include <pool.h>

extern ipcmapping_t _ipcmappings[];

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
