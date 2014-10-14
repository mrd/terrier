// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"
staload "ipcmem.sats"
staload "either_vt.sats"

fun ehci_detach_td(usbd: !usb_device): void = begin
  usb_device_set_current_td (usbd, EHCI_QH_PTR_T);
  usb_device_set_next_td (usbd, EHCI_QH_PTR_T);
  usb_device_set_alt_td (usbd, EHCI_QH_PTR_T)
end

fun ehci_attach_td(usbd: !usb_device, td: !ehci_td): [s: int] status s = let
  val _ = usb_device_set_current_td (usbd, EHCI_QH_PTR_T)
  val _ = usb_device_clear_overlay_status (usbd)
  val paddr = vmm_get_phys_addr (vptr_of_ehci_td td) // FIXME: handle failure
in
  usb_device_set_next_td (usbd, paddr); 0
end

implement alloc_tds (devr, tds, n) = let
  val m_dr = _usb_dev_req_pool_alloc ()
in
  case+ m_dr of
  | ~None_vt () => begin
      devr := Left_vt devr;
      tds := Left_vt tds; 1
    end
  | ~Some_vt dr => let
      val m_td = _ehci_td_pool_alloc ()
    in
      case+ m_td of
      | ~None_vt () => begin
        _usb_dev_req_pool_free dr;
        devr := Left_vt devr;
        tds := Left_vt tds; 1
      end
      | ~Some_vt td => begin
        devr := Right_vt dr;
        tds := Right_vt td; 0
      end
    end // [let val m_td]
  // end [case+ m_dr]
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
