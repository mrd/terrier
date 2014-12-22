// a USB interface for programs to use

#define ATS_STALOADFLAG 0 // no run-time staloading

staload "ipcmem.sats"
staload "either_vt.sats"
staload "prelude/SATS/status.sats"

%{#

#include "usb.cats"

%}


// USB device
absvtype usb_device_vt (i: int, nTDs: int, active: bool) = $extype "usb_device_t *"
vtypedef usb_device = [i, nTDs: int] [active: bool] usb_device_vt (i, nTDs, active)
fun usb_acquire_device {i: nat} (int i): usb_device_vt (i, 0, false)
fun usb_release_device (usb_device): void

// phys addresses
abst@ype physaddr_t (v: addr) = $extype "physaddr" // (v = associated virtual address). (could be invalidated).
typedef physaddr = [v: agez] physaddr_t v
typedef physaddr0 = [v: agez] physaddr_t v
typedef physaddr1 = [v: agz]  physaddr_t v
fun vmm_get_phys_addr {v: addr} (!ptr v, &physaddr? >> physaddr_t v'): #[s: int] #[v': agez | s <> 0 || v == v'] status s = "mac#vmm_get_phys_addr"

// QH is inside of usb_device
// {next_td, alt_td, current_td} are inside of QH


// USB device request type
absvt@ype usb_dev_req_vt = $extype "usb_dev_req_t"
vtypedef usb_dev_req = usb_dev_req_vt

absvtype usb_dev_req_ptr (l:addr) = ptr l
vtypedef usb_dev_req_ptr0 = [l:agez] usb_dev_req_ptr l
vtypedef usb_dev_req_ptr1 = [l:agz]  usb_dev_req_ptr l
vtypedef usb_dev_req_optr (s: int) = [l: agez | (~(s == 0) || l > null) && (~(s != 0) || l == null)] usb_dev_req_ptr l
castfn usb_dev_req_ptr2ptr {l:addr} (!usb_dev_req_ptr l):<> ptr l
overload ptrcast with usb_dev_req_ptr2ptr

fun _usb_dev_req_pool_alloc (): usb_dev_req_ptr0 = "mac#usb_dev_req_pool_alloc"
fun _usb_dev_req_pool_free (usb_dev_req_ptr1): void = "mac#usb_dev_req_pool_free"
macdef _usb_dev_req_null_ptr = $extval(usb_dev_req_ptr null, "NULL")
prfun _usb_dev_req_pool_free_null (usb_dev_req_ptr null): void = "mac#usb_dev_req_pool_free"


// TD manip
absvt@ype ehci_td_vt = $extype "ehci_td_t"
vtypedef ehci_td = ehci_td_vt



absvtype ehci_td_ptr (l:addr) = ptr l
vtypedef ehci_td_ptr0 = [l:agez] ehci_td_ptr l
vtypedef ehci_td_ptr1 = [l:agz]  ehci_td_ptr l
vtypedef ehci_td_optr (s: int) = [l: agez | (~(s == 0) || l > null) && (~(s != 0) || l == null)] ehci_td_ptr l
castfn ehci_td_ptr2ptr {l:addr} (!ehci_td_ptr l):<> ptr l
overload ptrcast with ehci_td_ptr2ptr

castfn ehci_td_fold {l: agz} (ehci_td @ l | ptr l): ehci_td_ptr l
castfn ehci_td_unfold {l: agz} (ehci_td_ptr l):<> (ehci_td @ l | ptr l)

fun ehci_td_set_next_td {v: agz} (!ehci_td, ehci_td_ptr v, physaddr_t v): void = "mac#ehci_td_set_next_td"
overload .set_next_td with ehci_td_set_next_td
// since set_next consumes ehci_td_ptr, then responsibility for freeing must be chained





fun _ehci_td_pool_alloc (): ehci_td_ptr0 = "mac#ehci_td_pool_alloc"
fun _ehci_td_pool_free (ehci_td_ptr1): void = "mac#ehci_td_pool_free"
macdef _ehci_td_null_ptr = $extval(ehci_td_ptr null, "NULL")
prfun _ehci_td_pool_free_null (ehci_td_ptr null): void
fun _ehci_td_pool_free_ref (&ehci_td_ptr1 >> ehci_td_ptr1?): void = "mac#ehci_td_pool_free"




// QH manip
macdef EHCI_QH_PTR_T = $extval(physaddr_t null, "EHCI_QH_PTR_T")

fun usb_device_clr_next_td (!usb_device): void = "mac#usb_device_clr_next_td"
fun usb_device_set_next_td {v: agz} (ehci_td_ptr v | !usb_device, physaddr_t v): void = "mac#usb_device_set_next_td"
overload .set_next_td with usb_device_set_next_td //dev.set_next_td(paddr)

fun usb_device_clr_current_td (!usb_device): void = "mac#usb_device_clr_current_td"
fun usb_device_set_current_td (!usb_device, physaddr): void
overload .set_current_td with usb_device_set_current_td

fun usb_device_clr_alt_td (!usb_device): void = "mac#usb_device_clr_alt_td"
fun usb_device_set_alt_td (!usb_device, physaddr): void
overload .set_alt_td with usb_device_set_alt_td

fun usb_device_clear_overlay_status (!usb_device): void
overload .clear_overlay_status with usb_device_clear_overlay_status


dataprop ehci_td_chain_len (v: addr, n: int) =
  | td_chain_fail (v, 0)
  | td_chain_base (v, 1)
  | {v': addr} td_chain_cons (v, n + 1) of ehci_td_chain_len (v', n)



// USB device requests

abst@ype usb_RequestType_t = int
abst@ype usb_Request_t = int

fun usb_device_request_allocate {numTDs: nat | numTDs > 1} (
    & usb_dev_req_ptr0 ? >> usb_dev_req_optr s,
    & ehci_td_ptr0 ? >> ehci_td_ptr l2,
    int numTDs,
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int, // wIndex
    int  // wLength
  ): #[s, n': int | (s <> 0 || (n' == numTDs))]
     #[l2: agez | (~(s == 0) || l2 > null) && (~(s != 0) || l2 == null)]
     (ehci_td_chain_len (l2, n') | status s)

datatype usb_RequestType_dir = HostToDevice | DeviceToHost
datatype usb_RequestType_type = Standard | Class | Vendor
datatype usb_RequestType_receipt = Device | Iface | Endpoint | Other
fun make_RequestType (usb_RequestType_dir, usb_RequestType_type, usb_RequestType_receipt): usb_RequestType_t

datatype usb_Request =
  GetStatus | ClearFeature | _Invalid_bRequest1 | SetFeature | _Invalid_bRequest2 |
  SetAddress | GetDescriptor | SetDescriptor | GetConfiguration | SetConfiguration |
  GetInterface | SetInterface | SynchFrame
fun make_Request (usb_Request): usb_Request_t

fun _usb_device_request_fill (!usb_dev_req_ptr1, usb_RequestType_t, usb_Request_t, int, int, int): void = "mac#_usb_device_request_fill"


// #define USB_REQ_HOST_TO_DEVICE 0
// #define USB_REQ_DEVICE_TO_HOST BIT(7)
// #define USB_REQ_TYPE_STANDARD  0
// #define USB_REQ_TYPE_CLASS     BIT(5)
// #define USB_REQ_TYPE_VENDOR    BIT(6)
// #define USB_REQ_RECIP_DEVICE   0
// #define USB_REQ_RECIP_IFACE    BIT(0)
// #define USB_REQ_RECIP_ENDPT    BIT(1)
// #define USB_REQ_RECIP_OTHER    (BIT(0) | BIT(1))


//
//
// #define USB_GET_STATUS           0x00
// #define USB_CLEAR_FEATURE        0x01
// #define USB_SET_FEATURE          0x03
// #define USB_SET_ADDRESS          0x05
// #define USB_GET_DESCRIPTOR       0x06
// #define USB_SET_DESCRIPTOR       0x07
// #define USB_GET_CONFIGURATION    0x08
// #define USB_SET_CONFIGURATION    0x09
// #define USB_GET_INTERFACE        0x0A
// #define USB_SET_INTERFACE        0x0B
// #define USB_SYNCH_FRAME          0x0C


// second byte in wValue specifies descriptor type for GetDescriptor
// #define USB_TYPE_DEV_DESC      0x01
// #define USB_TYPE_CFG_DESC      0x02
// #define USB_TYPE_STR_DESC      0x03
// #define USB_TYPE_IF_DESC       0x04
// #define USB_TYPE_EPT_DESC      0x05
// #define USB_TYPE_QUA_DESC      0x06
// #define USB_TYPE_SPD_CFG_DESC  0x07
// #define USB_TYPE_IF_PWR_DESC   0x08




// attach, detach, filling TDs

fun usb_device_detach {i, nTDs: nat} (
    ! usb_device_vt (i, nTDs, false) >> usb_device_vt (i, 0, false)
  ): [l: agz] (ehci_td_chain_len (l, nTDs) | ehci_td_ptr l)

absview ehci_td_chain_filled (l: addr)
dataview usb_transfer_status (i: int) =
  | usb_transfer_ongoing (i) of ()
  | usb_transfer_aborted (i) of ()

fun usb_device_attach {i, nTDs: nat | nTDs > 1} {l: agz} (
    ehci_td_chain_len (l, nTDs), ehci_td_chain_filled l |
    ! usb_device_vt (i, 0, false) >> usb_device_vt (i, nTDs, true),
    ehci_td_ptr l
  ): (usb_transfer_status i | void)

datasort pidcode = PIDSetup | PIDIn | PIDOut
abst@ype pidcode_t (p: pidcode) = int
macdef EHCI_PIDSetup = $extval(pidcode_t PIDSetup, "EHCI_PIDCODE_SETUP")
macdef EHCI_PIDIn    = $extval(pidcode_t PIDIn,    "EHCI_PIDCODE_IN")
macdef EHCI_PIDOut   = $extval(pidcode_t PIDOut,   "EHCI_PIDCODE_OUT")

absview ehci_td_filling_v (lstart: addr, status: int)

// begin process of filling TDs
fun ehci_td_start_fill {lstart, ldata: agz} {p: pidcode} {len: nat} (
    !ehci_td_ptr lstart, &ehci_td_ptr0? >> ehci_td_optr s, pidcode_t p, &ptr ldata >> ptr ldata', &int len >> int len'
  ): #[s: int]
     #[len': nat]
     #[ldata': agz | ldata' >= ldata]
      //[p': pidcode | (p == PIDSetup && (p' == PIDOut || p' == PIDIn)) || (p == PIDIn && p' == PIDOut) || (p == PIDOut && p' == PIDIn) ]
      (ehci_td_filling_v (lstart, s) | status s)

// another step in the filling process
fun ehci_td_step_fill {lstart, lcur: agz} {p: pidcode} {len: nat} {ldata: agez | len == 0 || ldata > null} (
    !ehci_td_filling_v (lstart, 0) >> ehci_td_filling_v (lstart, s) |
    !ehci_td_ptr lstart, &ehci_td_ptr lcur >> ehci_td_optr s, pidcode_t p, &ptr ldata >> ptr ldata', &int len >> int len'
  ): #[s: int]
     #[len': nat | len' <= len]
     #[ldata': agez | ldata' >= ldata]
     //#[p': pidcode | (p == PIDIn && p' == PIDOut) || (p == PIDOut && p' == PIDIn) ]
     status s

// successful completion of fill
fun ehci_td_complete_fill {lstart, lcur: agz} (
    ehci_td_filling_v (lstart, 0) |
    !ehci_td_ptr lstart, ehci_td_ptr lcur
  ): (ehci_td_chain_filled lstart | void)

// discard filled state
prfun ehci_td_flush_fill {l: addr} (ehci_td_chain_filled l): void

// abort filling process (in case of error)
fun ehci_td_abort_fill {lstart: agz} {s: int | s <> 0} (
    ehci_td_filling_v (lstart, s) |
    !ehci_td_ptr lstart
  ): void

// USB operations

fun usb_transfer_completed {i, nTDs: nat} (usb_transfer_status i | !usb_device_vt (i, nTDs, false)): void

fun usb_transfer_chain_active {i, nTDs: nat} (
    !usb_transfer_status i |
    !usb_device_vt (i, nTDs, true) >> usb_device_vt (i, nTDs, active)
  ): #[s: int] #[active: bool | (s == 0) <> active] status s

fun usb_transfer_status {i, nTDs: nat} (
    !usb_device_vt (i, nTDs, false)
  ): [s: int] status s

// USB control operations
// control read
fun usb_begin_control_read {i, len: nat} {buf: agz} (
    !usb_device_vt (i, 0, false) >> usb_device_vt (i, nTDs, active),
    &usb_dev_req_ptr0? >> usb_dev_req_ptr ldevr,
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int, // wIndex
    int len, // wLength
    ptr buf // data
  ): #[s: int]
     #[ldevr: agez | (s <> 0 || ldevr > null) && (s == 0 || ldevr == null)]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)


// fun free_tds {n: nat} (& usb_dev_req >> usb_dev_req?, & @[ehci_td_ptr][n] >> @[ehci_td_ptr][n]?, int n): void

////
absviewtype usb_device_vt (l: addr, n: int) = ptr l

fun usb_get_device {l: addr} {n: nat} (n: int n): usb_device_vt (l, n)
fun usb_put_device {l: addr} {n: nat} (usb_device_vt (l, n)): void

abstype physaddr (l:addr)       // index physaddr by virtual address it corresponds to?

viewtypedef next_td_v (l,l': addr) = (physaddr l' @ l | ptr l)

fun usb_device_get_next_td_v {l: addr} {n: nat} (_: !usb_device_vt (l, n)): [l': addr] next_td_v l'
fun usb_device_put_next_td_v {l: addr} (_: next_td_v l): void

// alternative:
fun usb_device_set_next_td {l,l': addr} {n: nat} (_: !usb_device_vt (l, n), _: physaddr l'): void
fun usb_device_set_alt_td {l,l': addr} {n: nat} (_: !usb_device_vt (l, n), _: physaddr l'): void
fun usb_device_set_current_td {l,l': addr} {n: nat} (_: !usb_device_vt (l, n), _: physaddr l'): void

fun vmm_get_phys_addr {l,l':addr} (_: ptr l): physaddr l'


////
//status usb_control_read(usb_device_t *d, u32 bmRequestType, u32 bRequest, u32 wValue, u32 wIndex, u32 wLength, void *data)
fun usb_control_read {n, dlen: nat} {l: addr} (
    pfdata: !array_v(byte?, dlen) @ l >> option_v(array_v(byte, dlen) @ l, s == 0) |
    d: usb_device_vt (n), bmRequestType: int, bRequest: int, wValue: int, wIndex: int, wLength: int dlen, data: ptr l
  ): #[s:int] status s
