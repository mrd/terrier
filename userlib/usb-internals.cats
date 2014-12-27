#ifndef _USERLIB_USB_INTERNALS_CATS_
#define _USERLIB_USB_INTERNALS_CATS_

#include "ats_types.h"
#include "pats_ccomp_basics.h"
#include "pats_ccomp_typedefs.h"
#include "types.h"
#include <pool.h>

DEVICE_POOL_DEFN (usb_dev_req, usb_dev_req_t, 4, 64);
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