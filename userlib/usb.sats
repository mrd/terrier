// a USB interface for programs to use

#define ATS_STALOADFLAG 0 // no run-time staloading

staload "ipcmem.sats"
staload "either_vt.sats"

%{#

#include "usb.cats"

%}


// USB device
absvtype usb_device_vt = $extype "usb_device_t *"
vtypedef usb_device = usb_device_vt
fun usb_acquire_device (int): usb_device
fun usb_release_device (usb_device): void

// phys addresses
abst@ype physaddr_t (v: addr) = $extype "physaddr" // (v = associated virtual address). (could be invalidated).
typedef physaddr = [v: agez] physaddr_t v
typedef physaddr0 = [v: agez] physaddr_t v
typedef physaddr1 = [v: agz]  physaddr_t v
fun vmm_get_phys_addr {v: addr} (!ptr v, &physaddr? >> physaddr_t v'): #[s: int] #[v': agez | s <> 0 || v == v'] status s = "mac#vmm_get_phys_addr"

// QH is inside of usb_device
// {next_td, alt_td, current_td} are inside of QH


// USB device request pointer
absvt@ype usb_dev_req_vt = $extype "usb_dev_req_t"
vtypedef usb_dev_req = usb_dev_req_vt

absvtype usb_dev_req_ptr (l:addr) = ptr l
vtypedef usb_dev_req_ptr0 = [l:agez] usb_dev_req_ptr l
vtypedef usb_dev_req_ptr1 = [l:agz]  usb_dev_req_ptr l
castfn usb_dev_req_ptr2ptr {l:addr} (!usb_dev_req_ptr l):<> ptr l
overload ptrcast with usb_dev_req_ptr2ptr

// TD manip
absvt@ype ehci_td_vt = $extype "ehci_td_t"
vtypedef ehci_td = ehci_td_vt

absvtype ehci_td_ptr (l:addr) = ptr l
vtypedef ehci_td_ptr0 = [l:agez] ehci_td_ptr l
vtypedef ehci_td_ptr1 = [l:agz]  ehci_td_ptr l
castfn ehci_td_ptr2ptr {l:addr} (!ehci_td_ptr l):<> ptr l
overload ptrcast with ehci_td_ptr2ptr

castfn ehci_td_fold {l: agz} (ehci_td @ l | ptr l): ehci_td_ptr l
castfn ehci_td_unfold {l: agz} (ehci_td_ptr l):<> (ehci_td @ l | ptr l)

fun ehci_td_set_next_td {v: agz} (!ehci_td, ehci_td_ptr v, physaddr_t v): void = "mac#ehci_td_set_next_td"
overload .set_next_td with ehci_td_set_next_td

//
// linked list of TDs using views ... cons/uncons?
//

fun _usb_dev_req_pool_alloc (): usb_dev_req_ptr0 = "mac#usb_dev_req_pool_alloc"
fun _usb_dev_req_pool_free (usb_dev_req_ptr1): void = "mac#usb_dev_req_pool_free"
fun _usb_dev_req_null_ptr (): usb_dev_req_ptr null
prfun _usb_dev_req_pool_free_null (usb_dev_req_ptr null): void = "mac#usb_dev_req_pool_free"

fun _ehci_td_pool_alloc (): ehci_td_ptr0 = "mac#ehci_td_pool_alloc"
fun _ehci_td_pool_free (ehci_td_ptr1): void = "mac#ehci_td_pool_free"
fun _ehci_td_null_ptr (): ehci_td_ptr null
prfun _ehci_td_pool_free_null (ehci_td_ptr null): void = "mac#ehci_td_pool_free"





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


dataprop td_chain_len (v: addr, n: int) =
  | td_chain_fail (v, 0)
  | td_chain_base (v, 1)
  | {v': addr} td_chain_cons (v, n + 1) of td_chain_len (v', n)

fun alloc_tds {n: nat | n > 1} (
    & usb_dev_req_ptr0 ? >> usb_dev_req_ptr l1,
    & ehci_td_ptr0 ? >> ehci_td_ptr l2,
    int n
  ): #[s, n': int] #[l1, l2: agez | s <> 0 || (l1 > null && l2 > null && n' == n)] (td_chain_len (l2, n') | status s)


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
