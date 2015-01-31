// A USB module for programs to use

#define ATS_DYNLOADFLAG 0

staload "usb.sats"
staload "ipcmem.sats"
staload "prelude/SATS/status.sats"
staload UN = "prelude/SATS/unsafe.sats"
staload _ = "prelude/DATS/unsafe.dats"
staload "prelude/SATS/array.sats"
staload _ = "prelude/DATS/array.dats"
staload _ = "prelude/DATS/integer.dats"
staload _ = "prelude/DATS/integer_fixed.dats"
#include "atspre_staload.hats"


%{^

#include "usb-internals.cats"

%}


// Attach and activate a TD chain
implement urb_attach (filled_v | urb, td, paddr) =
begin
  urb.clr_current_td ();
  urb.clr_overlay_status ();
  urb.set_next_td (filled_v | td, paddr)
end

implement urb_detach_and_free (urb) =
let
  val s = urb_transfer_result_status (urb)
  val tds = urb_detach (urb)
in
  ehci_td_chain_free tds; s
end

implement usb_device_detach_and_release_urb (usbd, urb) =
let
  val s = urb_transfer_result_status (urb)
  val tds = urb_detach (urb)
in
  usb_device_release_urb (usbd, urb);
  ehci_td_chain_free tds; s
end

implement urb_detach_and_free_ (urb) =
let
  val tds = urb_detach urb
in
  ehci_td_chain_free tds
end

implement usb_device_detach_and_release_urb_ (usbd, urb) =
let
  val tds = urb_detach urb
in
  usb_device_release_urb (usbd, urb);
  ehci_td_chain_free tds
end

// Allocate a device request and a starting TDs. Returns OK if success, ENOSPACE otherwise, and cleans up.
implement{} usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, wLength) =
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
    int, &ptr l >> ptr l', &int n >> int n'
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
    //if cpage > 0 then $extfcall (void, "debuglog", 0, 1, "setup_td_buffers\n") else (); // cpage=%d pdata=%#x\n",cpage,pdata) else ();
    td.set_buf (cpage, pdata); // FIXME: something's not right
    incr_td_page (g0ofg1 cpage, data, len);
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

// another step in the filling process (but only if need_empty = true)
extern fun ehci_td_step_fill1 {lstart: agz} {p: pidcode} {nTDs, len: nat} {ldata: agez | len == 0 || ldata > null} (
    !ehci_td_filling_v (lstart, 0, nTDs) >> ehci_td_filling_v (lstart, s, nTDs') |
    bool,
    !ehci_td_ptr lstart,
    &ehci_td_traversal1 lstart >> ehci_td_traversal_optr (lstart, s),
    pidcode_t p,
    &ptr ldata >> ptr ldata',
    &int len >> int len'
  ): #[s: int]
     #[nTDs': int | (s <> 0 || nTDs' >= nTDs) && (s == 0 || nTDs' == nTDs)]
     #[len': nat | len' <= len]
     #[ldata': agez | ldata' >= ldata]
     status s

implement ehci_td_step_fill1 (fill_v | need_empty, startTD, trav, pid, data, len) =
  if need_empty then ehci_td_step_fill (fill_v | startTD, trav, pid, data, len)
  else OK

// iteratively fills TDs from a given buffer of data
fun{} data_stage {lstart: agz} {nTDs, len: nat} {ldata: agez | len == 0 || ldata > null} {p: pidcode} (
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
if len = 0 then OK else
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

fun{a:vt0p} sizeofGEZ (): [s: nat] int s = $UN.cast{intGte(0)}(g1ofg0 (sizeof<a>))

extern fun dump_td (!ehci_td_ptr0, int): void = "mac#dump_td"
extern fun dump_urb (!urb2, int): void = "mac#dump_urb"
extern fun dump_usb_dev_req (!usb_dev_req_ptr0): void = "mac#dump_usb_dev_req"

implement{a} urb_begin_control_read (pf | urb, bmRequestType, bRequest, wValue, wIndex, numElems, data) =
let
  var startTD: ehci_td_ptr0?
  var devr: usb_dev_req_ptr0?
  val wLength = $UN.cast{intGte(0)}(sizeofGEZ<a>() * numElems) // nat * nat is a nat
  val s = usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, wLength)
  //val _ = dump_usb_dev_req devr
in
  // flattened if-then-else statements to avoid nesting
  if s != OK then let
    prval _ = ehci_td_free_null startTD
    prval _ = usb_dev_req_pool_free_null devr
  in
    (usb_transfer_aborted | s)
  end else let // REQUEST
    var p1: ptr = ptrcast devr
    var p1len: int = sizeofGEZ<usb_dev_req_vt>() // assert that size >= 0
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDSetup, p1, p1len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let // DATA
    var p2: ptr = data
    var p2len: int = wLength
    val s = data_stage (fill_v | startTD, trav, EHCI_PIDIn, p2, p2len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let // STATUS
    var p3: ptr = the_null_ptr
    var p3len: int = 0
    val s = ehci_td_step_fill (fill_v | startTD, trav, EHCI_PIDOut, p3, p3len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then let // failed phys addr lookup
    prval _ = ehci_td_flush_fill (filled_v)
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    //val _ = dump_td (startTD,0)
    val (xfer_v | ()) = urb_attach (filled_v | urb, startTD, paddr)
  in
    usb_dev_req_pool_free devr;
    (xfer_v | OK)
  end end end end end // flattened
end

implement{a} urb_begin_control_write (pf | urb, bmRequestType, bRequest, wValue, wIndex, numElems, data) =
let
  var startTD: ehci_td_ptr0?
  var devr: usb_dev_req_ptr0?
  val wLength = $UN.cast{intGte(0)}(sizeofGEZ<a>() * numElems) // FIXME: why is the cast necessary?
  val s = usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, wLength)
  //val _ = dump_usb_dev_req devr
in
  // flattened if-then-else statements to avoid nesting
  if s != OK then let
    prval _ = ehci_td_free_null startTD
    prval _ = usb_dev_req_pool_free_null devr
  in
    (usb_transfer_aborted | s)
  end else let // REQUEST
    var p1: ptr = ptrcast devr
    var p1len: int = sizeofGEZ<usb_dev_req_vt>()
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDSetup, p1, p1len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let // DATA
    var p2: ptr = data
    var p2len: int = wLength
    val s = data_stage (fill_v | startTD, trav, EHCI_PIDOut, p2, p2len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let // STATUS
    var p3: ptr = the_null_ptr
    var p3len: int = 0
    val s = ehci_td_step_fill (fill_v | startTD, trav, EHCI_PIDIn, p3, p3len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then let // failed phys addr lookup
    prval _ = ehci_td_flush_fill (filled_v)
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    //val _ = dump_td (startTD,0)
    val (xfer_v | ()) = urb_attach (filled_v | urb, startTD, paddr)
  in
    usb_dev_req_pool_free devr;
    (xfer_v | OK)
  end end end end end // flattened
end

implement{} urb_begin_control_nodata (urb, bmRequestType, bRequest, wValue, wIndex) =
let
  var startTD: ehci_td_ptr0?
  var devr: usb_dev_req_ptr0?
  val s = usb_device_request_allocate (devr, startTD, bmRequestType, bRequest, wValue, wIndex, 0)
  //val _ = dump_usb_dev_req devr
in
  // flattened if-then-else statements to avoid nesting
  if s != OK then let
    prval _ = ehci_td_free_null startTD
    prval _ = usb_dev_req_pool_free_null devr
  in
    (usb_transfer_aborted | s)
  end else let // REQUEST
    var p1: ptr = ptrcast devr
    var p1len: int = sizeofGEZ<usb_dev_req_vt>()
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDSetup, p1, p1len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let // STATUS
    var p3: ptr = the_null_ptr
    var p3len: int = 0
    val s = ehci_td_step_fill (fill_v | startTD, trav, EHCI_PIDIn, p3, p3len)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then let // failed phys addr lookup
    prval _ = ehci_td_flush_fill (filled_v)
  in
    usb_dev_req_pool_free devr;
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    //val _ = dump_td (startTD,0)
    val (xfer_v | ()) = urb_attach (filled_v | urb, startTD, paddr)
  in
    usb_dev_req_pool_free devr;
    (xfer_v | OK)
  end end end end // flattened
end

implement urb_wait_while_active (xfer_v | urb) =
let val s = urb_transfer_chain_active (xfer_v | urb) in
    if s = OK then urb_wait_while_active (xfer_v | urb) else ()
end // [usb_wait_while_active]

implement usb_with_urb (usbd, f) =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, urb)
in
  if s != OK
  then s where { prval _ = usb_device_release_null_urb urb }
  else s where { val s = f (usbd, urb)
                 val _ = usb_device_release_urb (usbd, urb) }
end

implement usb_set_configuration {i} (usbd, cfgval) = usb_with_urb (usbd, f)
where {
  var f = lam@ (usbd: !usb_device_vt i, urb: !urb_vt (i, 0, false)): [s: int] status s =<clo1> let
    val _ = urb_set_control_endpoint (usbd, urb)
    val (xfer_v | s) =
        urb_begin_control_nodata (urb, make_RequestType (HostToDevice, Standard, Device),
                                  make_Request SetConfiguration,
                                  cfgval, 0)
  in
    if s = OK then begin
      urb_wait_while_active (xfer_v | urb);
      urb_transfer_completed (xfer_v | urb);
      urb_detach_and_free urb
    end else begin
      urb_transfer_completed (xfer_v | urb);
      urb_detach_and_free_ urb;
      s
    end
  end
}

implement usb_get_configuration {i} (usbd, cfgval) = let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb
            val   _ = cfgval := $UN.cast{uint8}(0) }
else let
  val _ = urb_set_control_endpoint (usbd, urb)
  var buf = @[uint8][1]($UN.cast{uint8}(0))
  val (xfer_v | s) =
    urb_begin_control_read (view@ buf | urb, make_RequestType (DeviceToHost, Standard, Device),
                            make_Request GetConfiguration,
                            0, 0, 1, addr@ buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    cfgval := array_get_at (buf, 0);
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_device_detach_and_release_urb_ (usbd, urb);
    cfgval := $UN.cast{uint8}(0);
    s
  end
end end // [usb_get_configuration]

// note: worked first time
implement{a} urb_begin_bulk_read (pf | urb, numElems, data) =
let
  val packet_size = 512 // FIXME
  var startTD: ehci_td_ptr0? = ehci_td_pool_alloc ()
// flattened if-then-else statements to avoid nesting
in if ptrcast startTD = 0 then let
    prval _ = ehci_td_free_null startTD
  in
    (usb_transfer_aborted | ENOSPACE)
  end else let
    var plen = $UN.cast{intGte(0)}(sizeofGEZ<a>() * numElems) // nat * nat is a nat
    val need_empty = false //(plen mod packet_size) = 0
    var p: ptr = data
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDIn, p, plen)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    val _ = ehci_td_traversal_set_toggle trav
    val s = data_stage (fill_v | startTD, trav, EHCI_PIDIn, p, plen)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    var empty: ptr null = the_null_ptr
    var emptylen: int 0 = 0
    // empty packet if partial packet was last
    typedef status0 = [s: int] status s
    val s = ehci_td_step_fill1 (fill_v | need_empty, startTD, trav, EHCI_PIDIn, empty, emptylen)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then let // failed phys addr lookup
    prval _ = ehci_td_flush_fill (filled_v)
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    //val _ = dump_urb (urb,0)
    //val _ = dump_td (startTD,0)
    val (xfer_v | ()) = urb_attach (filled_v | urb, startTD, paddr)
  in
    (xfer_v | OK)
  end end end end end // flattened
end

implement{a} urb_begin_bulk_write (pf | urb, numElems, data) =
let
  val packet_size = 512 // FIXME
  var startTD: ehci_td_ptr0? = ehci_td_pool_alloc ()
// flattened if-then-else statements to avoid nesting
in if ptrcast startTD = 0 then let
    prval _ = ehci_td_free_null startTD
  in
    (usb_transfer_aborted | ENOSPACE)
  end else let
    var plen = $UN.cast{intGte(0)}(sizeofGEZ<a>() * numElems) // nat * nat is a nat
    val need_empty = true //(plen mod packet_size) != 0
    var p: ptr = data
    var trav: ptr?
    val (fill_v | s) = ehci_td_start_fill (startTD, trav, EHCI_PIDOut, p, plen) // FIXME: PING protocol
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    val s = data_stage (fill_v | startTD, trav, EHCI_PIDOut, p, plen)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    var empty: ptr null = the_null_ptr
    var emptylen: int 0 = 0
    // empty packet if partial packet was last
    typedef status0 = [s: int] status s
    val s = ehci_td_step_fill1 (fill_v | need_empty, startTD, trav, EHCI_PIDOut, empty, emptylen)
  in if s != OK then let
    prval _ = ehci_td_abort_fill (fill_v, startTD)
    prval _ = ehci_td_traversal_free_null trav
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    prval filled_v = ehci_td_complete_fill (fill_v, startTD, trav)
    var paddr: physaddr
    val s = vmm_get_phys_addr (ptrcast startTD, paddr)
  in if s != OK then let // failed phys addr lookup
    prval _ = ehci_td_flush_fill (filled_v)
  in
    ehci_td_chain_free startTD;
    (usb_transfer_aborted | s)
  end else let
    // OK and ready-to-go
    //val _ = dump_urb (urb,0)
    //val _ = dump_td (startTD, 0)
    val (xfer_v | ()) = urb_attach (filled_v | urb, startTD, paddr)
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
