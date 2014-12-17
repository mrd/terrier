// a USB interface for programs to use

#define ATS_STALOADFLAG 0 // no run-time staloading

staload "ipcmem.sats"
staload "either_vt.sats"

%{#

#include "usb.cats"
#define EHCI_QH_PTR_T 1

%}


// USB device
absvtype usb_device_vt
vtypedef usb_device = usb_device_vt
fun usb_acquire_device (int): usb_device
fun usb_release_device (usb_device): void

// phys addresses
abstype physaddr // (v: addr) // could be invalidated
fun vmm_get_phys_addr {v: addr} (ptr v): physaddr

// QH is inside of usb_device
// {next_td, alt_td, current_td} are inside of QH


// QH manip
macdef EHCI_QH_PTR_T = $extval(physaddr, "EHCI_QH_PTR_T")

fun usb_device_set_next_td (!usb_device, physaddr): void
overload .set_next_td with usb_device_set_next_td //dev.set_next_td(paddr)

fun usb_device_set_current_td (!usb_device, physaddr): void
overload .set_current_td with usb_device_set_current_td

fun usb_device_set_alt_td (!usb_device, physaddr): void
overload .set_alt_td with usb_device_set_alt_td

fun usb_device_clear_overlay_status (!usb_device): void
overload .clear_overlay_status with usb_device_clear_overlay_status

// USB device request
absvt@ype usb_dev_req_vt = $extype "usb_dev_req_t"
vtypedef usb_dev_req = usb_dev_req_vt
vtypedef usb_dev_req_ptr = [v: addr] (usb_dev_req @ v | ptr v)
vtypedef usb_dev_req_ptr (v: addr) = (usb_dev_req @ v | ptr v)

// TD manip
absvt@ype ehci_td_vt = $extype "ehci_td_t"
vtypedef ehci_td = ehci_td_vt
vtypedef ehci_td_ptr = [v: addr] (ehci_td @ v | ptr v)
vtypedef ehci_td_ptr (v:addr) = (ehci_td @ v | ptr v)

fun vptr_of_ehci_td (!ehci_td): [v: addr] ptr v
//
// linked list of TDs using views ... cons/uncons?
//

fun _ehci_td_pool_alloc (): [v: addr] (option_vt (ehci_td_ptr v, v > null)) = "mac#ehci_td_pool_alloc"
fun _ehci_td_pool_free {v: addr} (ehci_td_ptr v): void = "mac#ehci_td_pool_free"
fun _usb_dev_req_pool_alloc (): [v: addr] (option_vt (usb_dev_req_ptr v, v > null)) = "mac#usb_dev_req_pool_alloc"
fun _usb_dev_req_pool_free {v: addr} (usb_dev_req_ptr v): void = "mac#usb_dev_req_pool_free"

fun alloc_tds {n: nat} (& usb_dev_req_ptr? >> either_vt(usb_dev_req_ptr?, usb_dev_req_ptr, s==0),
                        & ehci_td_ptr? >> either_vt(ehci_td_ptr?, ehci_td_ptr, s==0),
                        int 1
                       ): #[s: int] status s

fun free_tds {n: nat} (& usb_dev_req >> usb_dev_req?, & @[ehci_td_ptr][n] >> @[ehci_td_ptr][n]?, int n): void

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