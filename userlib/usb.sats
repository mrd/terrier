// a USB interface for programs to use

#define ATS_STALOADFLAG 0 // no run-time staloading

staload "ipcmem.sats"
staload "prelude/SATS/status.sats"

%{#

#include "usb.cats"

%}


// USB device
absvtype usb_device_vt (i: int, nTDs: int, active: bool) = $extype "usb_device_t *"
vtypedef usb_device = [i, nTDs: int] [active: bool] usb_device_vt (i, nTDs, active)
castfn usb_dev_req_ptr2ptr (!usb_device):<> ptr
overload ptrcast with usb_dev_req_ptr2ptr
fun usb_acquire_device {i: nat} (int i): usb_device_vt (i, 0, false) = "mac#usb_acquire_device"
fun usb_release_device {i: nat} (usb_device_vt (i, 0, false)): void = "mac#usb_release_device"

// phys addresses
abst@ype physaddr_t (v: addr) = $extype "physaddr" // (v = associated virtual address). (could be invalidated).
typedef physaddr = [v: agez] physaddr_t v
typedef physaddr0 = [v: agez] physaddr_t v
typedef physaddr1 = [v: agz]  physaddr_t v
fun vmm_get_phys_addr {v: addr} (!ptr v, &physaddr? >> physaddr_t v'): #[s: int] #[v': agez | s <> 0 || v == v'] status s = "mac#vmm_get_phys_addr"


// USB device request viewtype
absvt@ype usb_dev_req_vt = $extype "usb_dev_req_t"
vtypedef usb_dev_req = usb_dev_req_vt


absvtype usb_dev_req_ptr (l:addr) = ptr l
vtypedef usb_dev_req_ptr0 = [l:agez] usb_dev_req_ptr l
vtypedef usb_dev_req_ptr1 = [l:agz]  usb_dev_req_ptr l
vtypedef usb_dev_req_optr (s: int) = [l: agez | (~(s == 0) || l > null) && (~(s != 0) || l == null)] usb_dev_req_ptr l
castfn usb_dev_req_ptr2ptr {l:addr} (!usb_dev_req_ptr l):<> ptr l
overload ptrcast with usb_dev_req_ptr2ptr

fun usb_dev_req_pool_alloc (): usb_dev_req_ptr0 = "mac#usb_dev_req_pool_alloc"
fun usb_dev_req_pool_free (usb_dev_req_ptr1): void = "mac#usb_dev_req_pool_free"
fun usb_dev_req_pool_free_ref (!usb_dev_req_ptr1 >> usb_dev_req_ptr1?): void = "mac#usb_dev_req_pool_free"
macdef usb_dev_req_null_ptr = $extval(usb_dev_req_ptr null, "NULL")
prfun usb_dev_req_pool_free_null (usb_dev_req_ptr null): void


// EHCI TD viewtype
absvt@ype ehci_td_vt = $extype "ehci_td_t"
vtypedef ehci_td = ehci_td_vt

// EHCI TD pointer viewtype.
absvtype ehci_td_ptr (l:addr) = $extype "ehci_td_t *"
vtypedef ehci_td_ptr0 = [l:agez] ehci_td_ptr l
vtypedef ehci_td_ptr1 = [l:agz]  ehci_td_ptr l
vtypedef ehci_td_optr (s: int) = [l: agez | (~(s == 0) || l > null) && (~(s != 0) || l == null)] ehci_td_ptr l
castfn ehci_td_ptr2ptr {l:addr} (!ehci_td_ptr l):<> ptr l
overload ptrcast with ehci_td_ptr2ptr

castfn ehci_td_fold {l: agz} (ehci_td @ l | ptr l): ehci_td_ptr l
castfn ehci_td_unfold {l: agz} (ehci_td_ptr l):<> (ehci_td @ l | ptr l)

// EHCI TD "set next TD" in chain. Consumes pointer, so responsibility is with ehci_td_chain_free.
fun ehci_td_set_next_td {v: agz} (!ehci_td, ehci_td_ptr v, physaddr_t v): void = "mac#ehci_td_set_next_td"
overload .set_next_td with ehci_td_set_next_td

// EHCI TD memory management
fun ehci_td_pool_alloc (): ehci_td_ptr0 = "mac#ehci_td_pool_alloc"
fun ehci_td_chain_free (ehci_td_ptr1): void = "mac#ehci_td_chain_free" // free TD and remaining chain of TDs
fun ehci_td_pool_free_ref (&ehci_td_ptr1 >> ehci_td_ptr1?): void = "mac#ehci_td_chain_free" // reference version

macdef ehci_td_null_ptr = $extval(ehci_td_ptr null, "NULL") // create a null ptr
prfun ehci_td_free_null (ehci_td_ptr null): void // consume a null ptr

// Represents a filled, ready-to-go, chain of TDs:
absview ehci_td_chain_filled (l: addr, nTDs: int)

// Represents the status of an on-going USB transfer associated with USB device i:
dataview usb_transfer_status (i: int) =
  | usb_transfer_ongoing (i) of ()
  | usb_transfer_aborted (i) of ()

// QH manip via USB device: QH is inside of usb_device
// {next_td, alt_td, current_td} are inside of QH
fun usb_device_clr_next_td (!usb_device): void = "mac#usb_device_clr_next_td"

// Setting next TD on the QH causes it to begin the transfer
fun usb_device_set_next_td {v: agz} {i, n: nat | n > 0} (
    ehci_td_chain_filled (v, n) |
    !usb_device_vt (i, 0, false) >> usb_device_vt (i, n, true),
    ehci_td_ptr v,
    physaddr_t v
  ): (usb_transfer_status i | void)
  = "mac#usb_device_set_next_td"
overload .set_next_td with usb_device_set_next_td

fun usb_device_clr_current_td (!usb_device): void = "mac#usb_device_clr_current_td"
overload .clr_current_td with usb_device_clr_current_td
//fun usb_device_set_current_td (!usb_device, physaddr): void
//overload .set_current_td with usb_device_set_current_td

fun usb_device_clr_alt_td (!usb_device): void = "mac#usb_device_clr_alt_td"
//fun usb_device_set_alt_td (!usb_device, physaddr): void
//overload .set_alt_td with usb_device_set_alt_td

fun usb_device_clr_overlay_status (!usb_device): void = "mac#usb_device_clr_overlay_status"
overload .clr_overlay_status with usb_device_clr_overlay_status

// definition of "end of chain" according to EHCI specification.
macdef EHCI_QH_PTR_T = $extval(physaddr_t null, "EHCI_QH_PTR_T")


// USB device requests

abst@ype usb_RequestType_t = int
abst@ype usb_Request_t = int

fun usb_device_request_allocate (
    & usb_dev_req_ptr0 ? >> usb_dev_req_optr s,
    & ehci_td_ptr0 ? >> ehci_td_ptr l2,
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int, // wIndex
    int  // wLength
  ): #[s: int]
     #[l2: agez | (~(s == 0) || l2 > null) && (~(s != 0) || l2 == null)]
     status s

datatype usb_RequestType_dir = HostToDevice | DeviceToHost
datatype usb_RequestType_type = Standard | Class | Vendor
datatype usb_RequestType_receipt = Device | Iface | Endpoint | Other
fun make_RequestType (usb_RequestType_dir, usb_RequestType_type, usb_RequestType_receipt): usb_RequestType_t = "mac#make_RequestType"

datatype usb_Request =
  GetStatus | ClearFeature | _Invalid_bRequest1 | SetFeature | _Invalid_bRequest2 |
  SetAddress | GetDescriptor | SetDescriptor | GetConfiguration | SetConfiguration |
  GetInterface | SetInterface | SynchFrame
fun make_Request (usb_Request): usb_Request_t = "mac#make_Request"

// second byte in wValue specifies descriptor type for GetDescriptor
macdef USB_TYPE_DEV_DESC = 1
macdef USB_TYPE_CFG_DESC = 2
macdef USB_TYPE_STR_DESC = 3
macdef USB_TYPE_IF_DESC = 4
macdef USB_TYPE_EPT_DESC = 5
macdef USB_TYPE_QUA_DESC = 6
macdef USB_TYPE_SPD_CFG_DESC = 7
macdef USB_TYPE_IF_PWR_DESC = 8

// filling out device request structs for configuration transfers
fun usb_device_request_fill (!usb_dev_req_ptr1, usb_RequestType_t, usb_Request_t, int, int, int): void
  = "mac#_usb_device_request_fill"

// detach, attach, filling TDs
fun usb_device_detach {i, nTDs: nat} (
    ! usb_device_vt (i, nTDs, false) >> usb_device_vt (i, 0, false)
  ): ehci_td_ptr1 = "mac#usb_device_detach"

fun usb_device_attach {i, nTDs: nat | nTDs > 0} {l: agz} (
    ehci_td_chain_filled (l, nTDs) |
    ! usb_device_vt (i, 0, false) >> usb_device_vt (i, nTDs, true),
    ehci_td_ptr l, physaddr_t l
  ): (usb_transfer_status i | void)

// packet identifier codes
datasort pidcode = PIDSetup | PIDIn | PIDOut
abst@ype pidcode_t (p: pidcode) = int
typedef pidcode = [p: pidcode] pidcode_t p
macdef EHCI_PIDSetup = $extval(pidcode_t PIDSetup, "EHCI_PIDCODE_SETUP")
macdef EHCI_PIDIn    = $extval(pidcode_t PIDIn,    "EHCI_PIDCODE_IN")
macdef EHCI_PIDOut   = $extval(pidcode_t PIDOut,   "EHCI_PIDCODE_OUT")

// tracking process of filling TDs
dataview ehci_td_filling_v (lstart: addr, status: int, nTDs: int) =
  | {s: int} ehci_td_filling_abort (lstart, status, nTDs) of ehci_td_filling_v (lstart, s, nTDs)
  | ehci_td_filling_start (lstart, 0, 1)
  | {n: nat} ehci_td_filling_step (lstart, 0, n + 1) of ehci_td_filling_v (lstart, 0, n)

// tracking the traversal of the TD chain
absvtype ehci_td_traversal_vt (lstart: addr, lcur: addr) = ptr lcur
vtypedef ehci_td_traversal_optr (lstart: addr, s: int) =
  [l: agez | (~(s == 0) || l > null) && (~(s != 0) || l == null)] ehci_td_traversal_vt (lstart, l)
vtypedef ehci_td_traversal1 (lstart: addr) = [l: agz] ehci_td_traversal_vt (lstart, l)
castfn ehci_td_traversal_ptr2ptr {l1,l2:addr} (!ehci_td_traversal_vt (l1,l2)):<> ptr l2
overload ptrcast with ehci_td_traversal_ptr2ptr
fun ehci_td_traversal_null0 {l: agz} (!ehci_td_ptr l): (ehci_td_traversal_vt (l, null))
  = "mac#ehci_td_traversal_null0"
fun ehci_td_traversal_null1 {l: agz} (!ehci_td_ptr l, ehci_td_traversal1 l): (ehci_td_traversal_vt (l, null))
  = "mac#ehci_td_traversal_null1"
fun ehci_td_traversal_start {lstart: agz} (!ehci_td_ptr lstart): ehci_td_traversal_vt (lstart, lstart)
  = "mac#ehci_td_traversal_start"
fun ehci_td_traversal_next {lstart, lcur: agz} (
    &ehci_td_traversal1 lstart >> ehci_td_traversal_vt (lstart, lcur),
    ehci_td_ptr lcur,
    physaddr_t lcur
  ): void
  = "mac#ehci_td_traversal_next"
prfun ehci_td_traversal_free_null {lstart: agz} (ehci_td_traversal_vt (lstart, null)): void

// begin process of filling TDs
fun ehci_td_start_fill {lstart, ldata: agz} {p: pidcode} {len: nat} (
    !ehci_td_ptr lstart, &ptr? >> ehci_td_traversal_optr (lstart, s), pidcode_t p, &ptr ldata >> ptr ldata', &int len >> int len'
  ): #[s: int]
     #[len': nat]
     #[ldata': agz | ldata' >= ldata]
      (ehci_td_filling_v (lstart, s, 1) | status s)

// another step in the filling process
fun ehci_td_step_fill {lstart: agz} {p: pidcode} {nTDs, len: nat} {ldata: agez | len == 0 || ldata > null} (
    !ehci_td_filling_v (lstart, 0, nTDs) >> ehci_td_filling_v (lstart, s, nTDs') |
    !ehci_td_ptr lstart,
    &ehci_td_traversal1 lstart >> ehci_td_traversal_optr (lstart, s),
    pidcode_t p,
    &ptr ldata >> ptr ldata',
    &int len >> int len'
  ): #[s: int]
     #[nTDs': int | (s <> 0 || nTDs' == nTDs + 1) && (s == 0 || nTDs' == nTDs)]
     #[len': nat | len' <= len]
     #[ldata': agez | ldata' >= ldata]
     status s

// successful completion of fill
prfun ehci_td_complete_fill {lstart: agz} {nTDs: nat} (
    ehci_td_filling_v (lstart, 0, nTDs), !ehci_td_ptr lstart, ehci_td_traversal1 lstart
  ): (ehci_td_chain_filled (lstart, nTDs))

// discard filled state
prfun ehci_td_flush_fill {l: addr} {n: int} (ehci_td_chain_filled (l, n)): void

// abort filling process (in case of error)
prfun ehci_td_abort_fill {lstart: agz} {nTDs: nat} {s: int | s <> 0} (
    ehci_td_filling_v (lstart, s, nTDs), !ehci_td_ptr lstart
  ): void

// USB operations
fun usb_transfer_completed {i, nTDs: nat} (usb_transfer_status i | !usb_device_vt (i, nTDs, false)): void = "mac#usb_transfer_completed"

fun usb_transfer_chain_active {i, nTDs: nat | nTDs > 0} (
    !usb_transfer_status i |
    !usb_device_vt (i, nTDs, true) >> usb_device_vt (i, nTDs, active)
  ): #[s: int] #[active: bool | (s == 0) == active] status s = "mac#usb_transfer_chain_active"

fun usb_transfer_status {i, nTDs: nat | nTDs > 0} (
    !usb_device_vt (i, nTDs, false)
  ): [s: int] status s = "mac#usb_transfer_status"

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
