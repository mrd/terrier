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
    td := _ehci_td_null_ptr;
    0
  end else 1
end

fun _alloc_td_chain {v: agz} {n: nat | n > 0} (start: &ehci_td_ptr v, n: int n): [n': int] (ehci_td_chain_len (v, n') | int n') =
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
implement usb_device_request_allocate (devr, tds, n, bmRequestType, bRequest, wValue, wIndex, wLength) =
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
              _usb_device_request_fill (devr0, bmRequestType, bRequest, wValue, wIndex, wLength);
              devr := devr0;
              tds := tds0;
              (pf_chain | 0)
            end
          else // alloc_td_chain failed
            begin
              _usb_dev_req_pool_free devr0;
              _ehci_td_pool_free tds0;  // note: I initially forgot this line; ATS caught it
              devr := _usb_dev_req_null_ptr;
              tds := _ehci_td_null_ptr;
              (td_chain_fail | 1)
            end
        end
      else // _ehci_td_pool_alloc failed
        let
          prval _ = _ehci_td_pool_free_null tds0
        in
          _usb_dev_req_pool_free devr0;
          devr := _usb_dev_req_null_ptr;
          tds := _ehci_td_null_ptr;
          (td_chain_fail | 1)
        end
    end
  else // _usb_dev_req_pool_alloc failed
    let
      prval _ = _usb_dev_req_pool_free_null devr0
    in
      devr := _usb_dev_req_null_ptr;
      tds := _ehci_td_null_ptr;
      (td_chain_fail | 1)
    end
end

macdef OK = $extval(status 0, "OK")
macdef EINVALID = $extval(status 1, "EINVALID")
macdef EUNDEFINED = $extval(status 2, "EUNDEFINED")
macdef ENOSPACE = $extval(status 3, "ENOSPACE")
macdef EINCOMPLETE = $extval(status 4, "EINCOMPLETE")
macdef EDATA = $extval(status 5, "EDATA")

// _ehci_td_pool_free should also free TDs in ->next pointer?


// ATS type checker made me realize that 'devr' needed to be a return
// value (by ref) from this function, unlike C version, because otherwise
// where does resource go?:

implement usb_begin_control_read (usbd, devr, bmRequestType, bRequest, wValue, wIndex, wLength, data) =
let
  var startTD: ehci_td_ptr0?
  val (chain_p | s) = usb_device_request_allocate (devr, startTD, 3, bmRequestType, bRequest, wValue, wIndex, wLength)
in
  // flattened if-then-else statements to avoid nesting
  if s != OK then begin
    _ehci_td_pool_free_null startTD;
    (usb_transfer_aborted | s)
  end else let
    var pdata: physaddr?
    val s = vmm_get_phys_addr (data, pdata)
  in if s != OK then begin
    _usb_dev_req_pool_free devr;
    devr := _usb_dev_req_null_ptr;
    _ehci_td_pool_free startTD;
    (usb_transfer_aborted | s)
  end else let
    var curTD: ehci_td_ptr0?
    var remaining = wLength
    val (fill_v | s) = ehci_td_start_fill (startTD, curTD, EHCI_PIDSetup, pdata, remaining) // FIXME supposed to be devr
  in if s != OK then begin
    ehci_td_abort_fill (fill_v | startTD);
    _usb_dev_req_pool_free devr;
    devr := _usb_dev_req_null_ptr;
    _ehci_td_pool_free startTD;
    _ehci_td_pool_free_null curTD;
    (usb_transfer_aborted | s)
  end else let
    val s = ehci_td_step_fill (fill_v | startTD, curTD, EHCI_PIDIn, pdata, remaining)
  in if s != OK then begin
    ehci_td_abort_fill (fill_v | startTD);
    _usb_dev_req_pool_free devr;
    devr := _usb_dev_req_null_ptr;
    _ehci_td_pool_free startTD;
    _ehci_td_pool_free_null curTD;
    (usb_transfer_aborted | s)
  end else let // FIXME how to handle STATUS stage?
    val (filled_v | ()) = ehci_td_complete_fill (fill_v | startTD, curTD)
    val (xfer_v | ()) = usb_device_attach (chain_p, filled_v | usbd, startTD)
  in
    (xfer_v | OK)
  end end end end // flattened
end


(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
