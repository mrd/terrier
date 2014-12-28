// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"
staload "ipcmem.sats"
staload "prelude/SATS/status.sats"
staload UN = "prelude/SATS/unsafe.sats"
#include "atspre_staload.hats"

%{^

#include "usb-internals.cats"

%}


// Attach and activate a TD chain
implement usb_device_attach (filled_v | usbd, td, paddr) =
begin
  usbd.clr_current_td ();
  usbd.clr_overlay_status ();
  usbd.set_next_td (filled_v | td, paddr)
end

// Allocate a device request and a starting TDs. Returns OK if success, ENOSPACE otherwise, and cleans up.
implement usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, wLength) =
let
  val devr0 = usb_dev_req_pool_alloc ()
in
  if ptrcast devr0 > 0 then
    let
      var td0 = ehci_td_pool_alloc ()
    in
      if ptrcast td0 > 0 then begin
        usb_device_request_fill (devr0, bmRequestType, bRequest, wValue, wIndex, wLength);
        devr := devr0;
        startTD := td0;
        OK
      end else let // ehci_td_pool_alloc failed
        prval _ = ehci_td_free_null td0
      in
        usb_dev_req_pool_free devr0;
        devr := usb_dev_req_null_ptr;
        startTD := ehci_td_null_ptr;
        ENOSPACE
      end
    end
  else // usb_dev_req_pool_alloc failed
    let
      prval _ = usb_dev_req_pool_free_null devr0
    in
      devr := usb_dev_req_null_ptr;
      startTD := ehci_td_null_ptr;
      ENOSPACE
    end
end
// end [usb_device_request_allocate]

// helper functions defined in macros or inline
extern fun clear_td (!ehci_td_ptr1): void = "mac#_clear_td"
extern fun setup_td {cpage, errcnt: nat | cpage < 5 && errcnt < 4} (
    !ehci_td_ptr1, bytes: int, ioc: bool, cpage: int cpage, errcnt: int errcnt, pid: pidcode, active: bool
  ): void = "mac#_setup_td"
extern fun set_td_buf {i: nat} (!ehci_td_ptr1, int i, physaddr): void = "mac#_set_td_buf"
overload .set_buf with set_td_buf
extern fun incr_td_page {l: agz} {n: nat} (
    &ptr l >> ptr l', &int n >> int n'
  ): #[l': agz | l' >= l] #[n': int | 0 <= n' && n' <= n] void = "mac#_incr_td_page"

// 5 pages = 20480 bytes is max for single TD
fun setup_td_buffers {i, n: nat | i < 6} {l: agez | (n == 0 || l > null)} (
    td: !ehci_td_ptr1, cpage: int i, data: &ptr l >> ptr l', len: &int n >> int n'
  ): #[s: int] #[l': agez | l' >= l] #[n': int | 0 <= n' && n' <= n] status s =
if len <= 0 then OK else if cpage >= 5 then OK else let
  var pdata: physaddr
  val s = vmm_get_phys_addr (data, pdata)
in
  if s != OK then s else begin
    td.set_buf (cpage, pdata);
    incr_td_page (data, len);
    setup_td_buffers (td, cpage + 1, data, len)
  end
end
// end [setup_td_buffers]

implement ehci_td_start_fill (startTD, trav, pid, data, len) =
let
  val len0 = len
  val () = clear_td startTD
  val s = setup_td_buffers (startTD, 0, data, len)
  val bytes = len0 - len
in
  if s != OK then begin
    trav := ehci_td_traversal_null0 (startTD);
    (ehci_td_filling_abort ehci_td_filling_start | s)
  end else begin
    setup_td (startTD, bytes, true, 0, 3, pid, true);
    trav := ehci_td_traversal_start startTD;
    (ehci_td_filling_start | OK)
  end
end
// end [ehci_td_start_fill]

implement ehci_td_step_fill (fill_v | startTD, trav, pid, data, len) =
let
  // writing in ATS gives me confidence that allocating on the fly like this
  // won't result in dangling pointers or memory leaks
  val td = ehci_td_pool_alloc ()
in
  if ptrcast td = 0 then // alloc failed
    let
      prval _ = ehci_td_free_null td
      prval _ = fill_v := ehci_td_filling_abort fill_v
    in
      trav := ehci_td_traversal_null1 (startTD, trav);
      ENOSPACE
    end
  else let
    val len0 = len
    val () = clear_td td
    val s = setup_td_buffers (td, 0, data, len)
    val bytes = len0 - len
  in
    if s != OK then begin // setup td buffers failed
      ehci_td_chain_free td;
      trav := ehci_td_traversal_null1 (startTD, trav);
      fill_v := ehci_td_filling_abort fill_v;
      s
    end else let
      var paddr: physaddr
      val s = vmm_get_phys_addr (ptrcast td, paddr)
    in
      if s != OK then begin // vmm_get_phys_addr failed
        ehci_td_chain_free td;
        trav := ehci_td_traversal_null1 (startTD, trav);
        fill_v := ehci_td_filling_abort fill_v;
        s
      end else begin // success
        setup_td (td, bytes, true, 0, 3, pid, true);
        ehci_td_traversal_next (trav, td, paddr);
        fill_v := ehci_td_filling_step fill_v;
        OK
      end
    end
  end
end
// end [ehci_td_step_fill]

// iteratively fills TDs from a given buffer of data
fun data_stage {lstart: agz} {nTDs, len: nat} {ldata: agez | len == 0 || ldata > null} {p: pidcode} (
    fill_v: !ehci_td_filling_v (lstart, 0, nTDs) >> ehci_td_filling_v (lstart, s, nTDs') |
    startTD: !ehci_td_ptr lstart,
    trav: &ehci_td_traversal1 lstart >> ehci_td_traversal_optr (lstart, s),
    pid: pidcode_t p,
    data: &ptr ldata >> ptr ldata',
    len: &int len >> int len'
  ): #[s: int]
     #[nTDs': int | nTDs' >= nTDs]
     #[ldata': agez | ldata' >= ldata]
     #[len': nat | len' <= len]
     status s =
let
  val s = ehci_td_step_fill (fill_v | startTD, trav, pid, data, len)
in
  if s != OK then s
  else if len > 0 then data_stage (fill_v | startTD, trav, pid, data, len)
  else s
end
// end [data_stage]

// ATS type checker made me realize that 'devr' needed to be a return
// value (by ref) from this function, unlike C version, because otherwise
// where does resource go?:

implement usb_begin_control_read (usbd, devr, bmRequestType, bRequest, wValue, wIndex, wLength, data) =
let
  var startTD: ehci_td_ptr0?
  val s = usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, wLength)
in
  // flattened if-then-else statements to avoid nesting
  if s != OK then begin
    let prval _ = ehci_td_free_null startTD in end;
    (usb_transfer_aborted | s)
  end else let // REQUEST
    var p1: ptr = ptrcast devr
    var p1len: int = $UN.cast{intGte(0)}(g1ofg0(sizeof<usb_dev_req_vt>)) // assert that size >= 0
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDSetup, p1, p1len)
  in if s != OK then begin
    let prval _ = ehci_td_abort_fill (fill_v, startTD) in end;
    usb_dev_req_pool_free devr;
    devr := usb_dev_req_null_ptr;
    ehci_td_chain_free startTD;
    let prval _ = ehci_td_traversal_free_null trav in end;
    (usb_transfer_aborted | s)
  end else let // DATA
    var p2: ptr = data
    var p2len: int = wLength
    val s = data_stage (fill_v | startTD, trav, EHCI_PIDIn, p2, p2len)
  in if s != OK then begin
    let prval _ = ehci_td_abort_fill (fill_v, startTD) in end;
    usb_dev_req_pool_free devr;
    devr := usb_dev_req_null_ptr;
    ehci_td_chain_free startTD;
    let prval _ = ehci_td_traversal_free_null trav in end;
    (usb_transfer_aborted | s)
  end else let // STATUS
    var p3: ptr = the_null_ptr
    var p3len: int = 0
    val s = ehci_td_step_fill (fill_v | startTD, trav, EHCI_PIDOut, p3, p3len)
  in if s != OK then begin
    let prval _ = ehci_td_abort_fill (fill_v, startTD) in end;
    usb_dev_req_pool_free devr;
    devr := usb_dev_req_null_ptr;
    ehci_td_chain_free startTD;
    let prval _ = ehci_td_traversal_free_null trav in end;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then begin // failed phys addr lookup
    let prval _ = ehci_td_flush_fill (filled_v) in end;
    usb_dev_req_pool_free devr;
    devr := usb_dev_req_null_ptr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    val (xfer_v | ()) = usb_device_attach (filled_v | usbd, startTD, paddr)
  in
    (xfer_v | OK)
  end end end end end // flattened
end

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
