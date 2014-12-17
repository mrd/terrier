// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"
staload "ipcmem.sats"
staload "either_vt.sats"
#include "atspre_staload.hats"


fun ehci_detach_td(usbd: !usb_device): void =
let
  val td = _ehci_td_null_ptr ()
in
  usb_device_set_current_td (usbd, EHCI_QH_PTR_T);
  usb_device_set_next_td (td | usbd, EHCI_QH_PTR_T);
  usb_device_set_alt_td (usbd, EHCI_QH_PTR_T)
end

fun ehci_attach_td {l: agz} (
    usbd: !usb_device,
    td: &ehci_td_ptr l >> ehci_td_ptr l' // if successful, consumes TD -- else it does not
  ): #[s: int] #[l': agez | s == 0 || l == l'] status s =
let
  val _ = usb_device_set_current_td (usbd, EHCI_QH_PTR_T)
  val _ = usb_device_clear_overlay_status (usbd)
  var paddr: physaddr
  val s = vmm_get_phys_addr (ptrcast td, paddr)
in
  if s = 0 then begin usb_device_set_next_td (td | usbd, paddr); td := _ehci_td_null_ptr (); 0 end else 1
end

implement alloc_tds (devr, tds, n (* n = 1 for the time being *) ) =
let
  val devr0 = _usb_dev_req_pool_alloc ()
in
if ptrcast devr0 > 0 then let
    val tds0 = _ehci_td_pool_alloc ()
  in
    if ptrcast tds0 > 0 then begin
      devr := devr0;
      tds := tds0;
      0
    end else let
      prval _ = _ehci_td_pool_free_null tds0
    in
      _usb_dev_req_pool_free devr0;
      devr := _usb_dev_req_null_ptr ();
      tds := _ehci_td_null_ptr ();
      1
    end
  end else let
    prval _ = _usb_dev_req_pool_free_null devr0
  in
    devr := _usb_dev_req_null_ptr ();
    tds := _ehci_td_null_ptr ();
    1
  end
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
