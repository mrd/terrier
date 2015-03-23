// a USB interface for programs to use

#define ATS_STALOADFLAG 0 // no run-time staloading

staload "prelude/SATS/status.sats"
staload "ipcmem.sats"

%{#

#include "usb.cats"

%}


// USB device
absvtype usb_device_vt (i: int) = $extype "usb_device_t *"
vtypedef usb_device = [i: int] usb_device_vt (i)
castfn usb_dev_req_ptr2ptr (!usb_device):<> ptr
overload ptrcast with usb_dev_req_ptr2ptr
fun usb_acquire_device {i: nat} (int i): usb_device_vt (i) = "mac#usb_acquire_device"
fun usb_release_device {i: nat} (usb_device_vt (i)): void = "mac#usb_release_device"

// USB Request Block
absvtype urb_vt (i: int, nTDs: int, active: bool) = $extype "ehci_qh_t *"
vtypedef urb0 = [i: int | i >= ~1] urb_vt (i, 0, false)
vtypedef urb1 = [i: nat] urb_vt (i, 0, false)
vtypedef urb2 = [i, nTDs: nat] [active: bool] urb_vt (i, nTDs, active)
vtypedef urb1 (i: int) = urb_vt (i, 0, false)
vtypedef urb2 (i: int) = [nTDs: nat] [active: bool] urb_vt (i, nTDs, active)
fun usb_device_alloc_urb {i, endpt, maxpkt: nat | (endpt < 16 && 7 < maxpkt && maxpkt < 2048) || (endpt == 0 && maxpkt == 0)} (
    !usb_device_vt (i), int endpt, int maxpkt, &urb0? >> urb_vt (i', 0, false)
  ): #[s, i': int | (s != 0 || i' == i) && (s == 0 || i' == ~1)] status s = "mac#usb_device_alloc_urb"
fun usb_device_release_urb {i: nat} (!usb_device_vt (i), urb_vt (i, 0, false)): void = "mac#usb_device_release_urb"
prfun usb_device_release_null_urb (urb_vt (~1, 0, false)): void

//fun urb_set_control_endpoint {i: nat} (!usb_device_vt i, !urb_vt (i, 0, false)): void = "mac#urb_set_control_endpoint"
//fun urb_set_endpoint {i, endpt, maxpkt: nat | endpt < 16 && maxpkt < 2048} (
//    !urb_vt (i, 0, false), int endpt, int maxpkt
//  ): void = "mac#urb_set_endpoint"

// ** Convenient device buffers for USB operations
//absview usb_mem_v (l: addr, size: int)
absvtype usb_mem_vt (l: addr, n: int) = ptr l
vtypedef usb_mem (n: int) = [l:addr] usb_mem_vt (l,n)
castfn usb_mem_ptr2ptr {l: addr} {n: int} (!usb_mem_vt (l, n)):<> ptr l
overload ptrcast with usb_mem_ptr2ptr
//fun usb_mem_vt_split {l: addr} {n: int} (usb_mem_vt (l,n)): (usb_mem_v (l,n) | ptr l)
//fun usb_mem_vt_unsplit {l: addr} {n: int} (usb_mem_v (l,n) | ptr l): usb_mem_vt (l,n)

stadef USB_BUF_MAX_SIZE = 4096
fun usb_buf_alloc {n: nat | n <= USB_BUF_MAX_SIZE} (int n): [l: agez] usb_mem_vt (l, n) = "mac#usb_buf_alloc"
fun usb_buf_release {l: agz} {n: int} (usb_mem_vt (l, n)): void = "mac#usb_buf_release"
prfun usb_buf_release_null {n: int} (usb_mem_vt (null, n)): void
fun usb_buf_takeout {a: t@ype} {l: agz} {n: nat | sizeof a <= n} (
    usb_mem_vt (l, n)
  ): (a @ l, a @ l -<lin,prf> usb_mem_vt (l, n) | ptr l) = "mac#usb_buf_takeout"

fun usb_buf_get_uint {l: agz} {n: nat | 4 <= n} (!usb_mem_vt (l, n)): uint = "mac#usb_buf_get_uint"
fun usb_buf_get_uint_at {l: agz} {i, n: nat | 4 * i <= n} (!usb_mem_vt (l, n), int i): uint = "mac#usb_buf_get_uint_at"
fun usb_buf_set_uint {l: agz} {n: nat | 4 <= n} (!usb_mem_vt (l, n), uint): void = "mac#usb_buf_set_uint"
fun usb_buf_set_uint_at {l: agz} {i, n: nat | 4 * i <= n} (!usb_mem_vt (l, n), int i, uint): void = "mac#usb_buf_set_uint_at"
praxi lemma_sizeof_uint (): [sizeof(uint) == 4] void
fun usb_buf_get_uint16 {l: agz} {n: nat | 2 <= n} (!usb_mem_vt (l, n)): uint16 = "mac#usb_buf_get_uint16"
praxi lemma_sizeof_uint16 (): [sizeof(uint16) == 2] void
fun usb_buf_get_uint8 {l: agz} {n: nat | 1 <= n} (!usb_mem_vt (l, n)): uint8 = "mac#usb_buf_get_uint8"
praxi lemma_sizeof_uint8 (): [sizeof(uint8) == 1] void
fun{a:t@ype} usb_buf_copy_into_array {n,m: nat} {src,dst: agz} (
    !(@[a][m]) @ dst | ptr dst, !usb_mem_vt (src, n), int m
  ): void
fun{a:t@ype} usb_buf_copy_from_array {n,m: nat} {src,dst: agz} (
    !(@[a][m]) @ src | !usb_mem_vt (dst, n), ptr src, int m
  ): void


// URB wrapper
fun usb_with_urb {i, endpt, maxpkt: nat | (endpt < 16 && 7 < maxpkt && maxpkt < 2048) || (endpt == 0 && maxpkt == 0)} (
    !usb_device_vt i, int endpt, int maxpkt, f: &((!usb_device_vt i, !urb_vt (i, 0, false)) -<clo1> [s: int] status s)
  ): [s: int] status s;


(*
fun usb_with_urb_and_buf
    {n,i: nat | n < USB_BUF_SIZE}
    {endpt, maxpkt: nat | (endpt < 16 && 7 < maxpkt && maxpkt < 2048) || (endpt == 0 && maxpkt == 0)}

    (
    !usb_device_vt i, int endpt, int maxpkt, int n,
    f: &({l: agz} (!usb_device_vt i, !urb_vt (i, 0, false), !usb_mem_vt (l, n)) -<clo1> [s: int] (status s))
  ): [s: int] status s;
*)

fun urb_clr_current_td {i: nat} (!urb_vt (i, 0, false)): void = "mac#urb_clr_current_td"
overload .clr_current_td with urb_clr_current_td
fun urb_clr_alt_td {i: nat} (!urb_vt (i, 0, false)): void = "mac#urb_clr_alt_td"
overload .clr_alt_td with urb_clr_alt_td
fun urb_clr_overlay_status {i: nat} (!urb_vt (i, 0, false)): void = "mac#urb_clr_overlay_status"
overload .clr_overlay_status with urb_clr_overlay_status


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
// definition of "end of chain" according to EHCI specification.
macdef EHCI_QH_PTR_T = $extval(physaddr_t null, "EHCI_QH_PTR_T")

macdef ehci_td_null_ptr = $extval(ehci_td_ptr null, "NULL") // create a null ptr
prfun ehci_td_free_null (ehci_td_ptr null): void // consume a null ptr

// Represents a filled, ready-to-go, chain of TDs:
absview ehci_td_chain_filled (l: addr, nTDs: int)

// Represents the status of an on-going USB transfer associated with USB device i:
dataview usb_transfer_status (i: int) =
  | usb_transfer_ongoing (i) of ()
  | usb_transfer_aborted (i) of ()

// QH manip via URB: QH is part of URB
// {next_td, alt_td, current_td} are inside of QH

// Setting next TD on the QH causes it to begin the transfer
fun urb_set_next_td {i, n: nat | n > 0} {v: agz} (
    ehci_td_chain_filled (v, n) |
    !urb_vt (i, 0, false) >> urb_vt (i, n, true),
    ehci_td_ptr v,
    physaddr_t v
  ): (usb_transfer_status i | void)
  = "mac#urb_set_next_td"
overload .set_next_td with urb_set_next_td

// detach TDs from USB device, returning them
fun urb_detach {i, nTDs: nat} (
    ! urb_vt (i, nTDs, false) >> urb_vt (i, 0, false)
  ): ehci_td_ptr1 = "mac#urb_detach"

// detach TDs from USB device and free them in one step
// also return result status of transfer
fun urb_detach_and_free {i, nTDs: nat | nTDs > 0} (
    ! urb_vt (i, nTDs, false) >> urb_vt (i, 0, false)
  ): [s: int] status s
// combined URB release and detach
fun usb_device_detach_and_release_urb {i, nTDs: nat | nTDs > 0} (
    !usb_device_vt (i), urb_vt (i, nTDs, false)
  ): [s: int] status s

// version that does not return status
fun urb_detach_and_free_  {i, nTDs: nat} (
    ! urb_vt (i, nTDs, false) >> urb_vt (i, 0, false)
  ): void
fun usb_device_detach_and_release_urb_ {i, nTDs: nat} (
    !usb_device_vt (i), urb_vt (i, nTDs, false)
  ): void


// attach a chain of TDs to a QH within the USB device
fun urb_attach {i, nTDs: nat | nTDs > 0} {l: agz} (
    ehci_td_chain_filled (l, nTDs) |
    ! urb_vt (i, 0, false) >> urb_vt (i, nTDs, true),
    ehci_td_ptr l, physaddr_t l
  ): (usb_transfer_status i | void)

// //////////////////////////////////////////////////

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

// packet identifier codes
datasort pidcode = PIDSetup | PIDIn | PIDOut
abst@ype pidcode_t (p: pidcode) = int
typedef pidcode = [p: pidcode] pidcode_t p
fun pidcode_eq (pidcode, pidcode): bool = "mac#pidcode_eq"
overload = with pidcode_eq
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

// get the status of the toggle bit stored in the URB overlay area (to be used for initial TD)
fun urb_overlay_data_toggle (!urb2): bool = "mac#urb_overlay_data_toggle"
// get the toggle bit associated with the current TD in the traversal
fun ehci_td_traversal_get_toggle {l: agz} (&ehci_td_traversal1 l): bool = "mac#ehci_td_traversal_get_toggle"
// set the toggle bit associated with the current TD in the traversal
fun ehci_td_traversal_set_toggle {l: agz} (&ehci_td_traversal1 l): void = "mac#ehci_td_traversal_set_toggle"

// begin process of filling TDs
fun ehci_td_start_fill {lstart, ldata: agz} {p: pidcode} {len: nat} (
    !urb2, !ehci_td_ptr lstart, &ptr? >> ehci_td_traversal_optr (lstart, s), pidcode_t p, &ptr ldata >> ptr ldata', &int len >> int len'
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
fun urb_transfer_completed {i, nTDs: nat} (usb_transfer_status i | !urb_vt (i, nTDs, false)): void = "mac#urb_transfer_completed"
fun urb_transfer_abort {i, nTDs: nat} {active: bool} (
    usb_transfer_status i |
    !urb_vt (i, nTDs, active) >> urb_vt (i, nTDs, false)
  ): void = "mac#urb_transfer_abort"

fun urb_has_transfer_chain {i, nTDs: nat} {active: bool} (
      !urb_vt (i, nTDs, active)
    ): [nTDs != 0 || active == false] bool (nTDs > 0) = "mac#urb_has_transfer_chain"

fun urb_transfer_chain_active {i, nTDs: nat | nTDs > 0} {active: bool} (
    !usb_transfer_status i |
    !urb_vt (i, nTDs, active) >> urb_vt (i, nTDs, active')
  ): #[s: int] #[active': bool | (s == 0) == active'] status s = "mac#urb_transfer_chain_active"

fun urb_transfer_result_status {i, nTDs: nat | nTDs > 0} (
    !urb_vt (i, nTDs, false)
  ): [s: int] status s = "mac#urb_transfer_result_status"


// ** USB control operations

// control nodata
fun urb_begin_control_nodata {i: nat} (
    !urb_vt (i, 0, false) >> urb_vt (i, nTDs, active),
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int  // wIndex
  ): #[s: int]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)

// control read
fun urb_begin_control_read {i, n, len: nat | len <= n} {l: agz} (
    !urb_vt (i, 0, false) >> urb_vt (i, nTDs, active),
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int, // wIndex
    int len, // wLength
    !usb_mem_vt (l, n) // data
  ): #[s: int]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)

// control write
fun urb_begin_control_write {i, n, len: nat | len <= n} {l: agz} (
    !urb_vt (i, 0, false) >> urb_vt (i, nTDs, active),
    usb_RequestType_t,
    usb_Request_t,
    int, // wValue
    int, // wIndex
    int len, // wLength
    !usb_mem_vt (l, n) // data
  ): #[s: int]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)

// USB helper control operations
fun urb_wait_while_active {i, nTDs: nat | nTDs > 0} {active: bool} (
    xfer_v: !usb_transfer_status i |
    usbd: !urb_vt (i, nTDs, active) >> urb_vt (i, nTDs, false)
  ): void

fun urb_wait_if_active {i, nTDs: nat} {active: bool} (
    xfer_v: !usb_transfer_status i |
    usbd: !urb_vt (i, nTDs, active) >> urb_vt (i, nTDs, false)
  ): void

fun urb_wait_while_active_with_timeout {i, nTDs: nat | nTDs > 0} {active: bool} (
    xfer_v: usb_transfer_status i |
    usbd: !urb_vt (i, nTDs, active) >> urb_vt (i, nTDs, false),
    timeout: int
  ): void

fun usb_set_configuration {i: nat} (!usb_device_vt (i), int): [s: int] status s
fun usb_get_configuration {i: nat} (!usb_device_vt (i), &uint8? >> uint8): [s: int] status s
fun usb_get_endpoint_status {i: nat} (!usb_device_vt (i), int, &uint? >> uint): [s: int] status s
fun usb_clear_endpoint {i: nat} (!usb_device_vt (i), int): [s: int] status s

// ** USB bulk operations
// bulk read
fun urb_begin_bulk_read {i, n, len: nat | len <= n} {l: agz} (
    !urb_vt (i, 0, false) >> urb_vt (i, nTDs, active),
    int len, // number of elements
    !usb_mem_vt (l, n) // data
  ): #[s: int]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)

// bulk write
fun urb_begin_bulk_write {i, n, len: nat | len <= n} {l: agz} (
    !urb_vt (i, 0, false) >> urb_vt (i, nTDs, active),
    int len, // number of elements
    !usb_mem_vt (l, n) // data
  ): #[s: int]
     #[nTDs: int | (s == 0 || nTDs == 0) && (s <> 0 || nTDs > 0)]
     #[active: bool | (s == 0) == active]
     (usb_transfer_status i | status s)

// USB specification defined data structures
abst@ype usb_dev_desc_t = $extype "usb_dev_desc_t"
praxi lemma_sizeof_usb_dev_desc_t (): [sizeof(usb_dev_desc_t) == 18] void
macdef sizeof_usb_dev_desc_t = 18
macdef usb_dev_desc_empty = $extval (usb_dev_desc_t, "((usb_dev_desc_t) {0})")
fun usb_device_num_configurations (!usb_device): int = "mac#usb_device_num_configurations"
overload .num_configurations with usb_device_num_configurations
fun usb_device_id_vendor (!usb_device): int = "mac#usb_device_id_vendor"
overload .id_vendor with usb_device_id_vendor
fun usb_device_id_product (!usb_device): int = "mac#usb_device_id_product"
overload .id_product with usb_device_id_product

abst@ype usb_cfg_desc_t = $extype "usb_cfg_desc_t"
praxi lemma_sizeof_usb_cfg_desc_t (): [sizeof(usb_cfg_desc_t) == 9] void
macdef sizeof_usb_cfg_desc_t = 9
macdef usb_cfg_desc_empty = $extval (usb_cfg_desc_t, "((usb_cfg_desc_t) {0})")

abst@ype usb_if_desc_t  = $extype "usb_if_desc_t"
abst@ype usb_ept_desc_t = $extype "usb_ept_desc_t"
abst@ype usb_str_desc_t = $extype "usb_str_desc_t"
