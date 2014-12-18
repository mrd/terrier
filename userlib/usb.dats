// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"
staload "ipcmem.sats"
staload "either_vt.sats"
#include "atspre_staload.hats"

fun ehci_detach_td (usbd: !usb_device): void = begin
  usb_device_clr_current_td usbd;
  usb_device_clr_next_td usbd;
  usb_device_clr_alt_td usbd
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
  if s = 0 then begin
    usb_device_set_next_td (td | usbd, paddr);
    td := _ehci_td_null_ptr ();
    0
  end else 1
end

fun _alloc_td_chain {v: agz} {n: nat | n > 0} (start: &ehci_td_ptr v, n: int n): [n': int] (td_chain_len (v, n') | int n') =
  if n < 2 then (td_chain_base | 1) else let
    var td0 = _ehci_td_pool_alloc ()
  in
    if ptrcast td0 > 0 then
      let
        val (pf | n') = _alloc_td_chain (td0, n - 1)
      in
        if n' <> (n - 1) then
          begin
            _ehci_td_pool_free td0;
            (td_chain_fail | 0)
          end
        else
          let
            var paddr: physaddr
            val s = vmm_get_phys_addr (ptrcast td0, paddr)
          in
            if s = 0 then
              let
                val (pf_s | start_td) = ehci_td_unfold start
              in
                (!start_td).set_next_td (td0, paddr);
                start := ehci_td_fold (pf_s | start_td);
                (td_chain_cons pf | n' + 1)
              end
            else
              begin
                _ehci_td_pool_free td0; // get_phys_addr failed
                (td_chain_base | 1)
              end
          end
      end
    else
      let
        prval _ = _ehci_td_pool_free_null td0 // alloc failed
      in
        (td_chain_base | 1)
      end
  end

// Allocate a device request and a chain of TDs. Returns OK if success, EINVALID otherwise, and cleans up.
implement alloc_tds (devr, tds, n) =
let
  val devr0 = _usb_dev_req_pool_alloc ()
in
  if ptrcast devr0 > 0 then
    let
      var tds0 = _ehci_td_pool_alloc ()
    in
      if ptrcast tds0 > 0 then
        let
          val (pf_chain | n') = _alloc_td_chain (tds0, n)
        in
          if n = n' then
            begin // success
              devr := devr0;
              tds := tds0;
              (pf_chain | 0)
            end
          else // alloc_td_chain failed
            begin
              _usb_dev_req_pool_free devr0;
              _ehci_td_pool_free tds0;  // note: I initially forgot this line; ATS caught it
              devr := _usb_dev_req_null_ptr ();
              tds := _ehci_td_null_ptr ();
              (td_chain_fail | 1)
            end
        end
      else // _ehci_td_pool_alloc failed
        let
          prval _ = _ehci_td_pool_free_null tds0
        in
          _usb_dev_req_pool_free devr0;
          devr := _usb_dev_req_null_ptr ();
          tds := _ehci_td_null_ptr ();
          (td_chain_fail | 1)
        end
    end
  else // _usb_dev_req_pool_alloc failed
    let
      prval _ = _usb_dev_req_pool_free_null devr0
    in
      devr := _usb_dev_req_null_ptr ();
      tds := _ehci_td_null_ptr ();
      (td_chain_fail | 1)
    end
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
