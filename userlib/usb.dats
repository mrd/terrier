// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"

fun ehci_detach_td(usbd: !usb_device): void = begin
  usbd.set_current_td(EHCI_QH_PTR_T);
  usb_device_set_current_td(usbd, EHCI_QH_PTR_T);
  usb_device_set_next_td(usbd, EHCI_QH_PTR_T);
  usb_device_set_alt_td(usbd, EHCI_QH_PTR_T)  
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
