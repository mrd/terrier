// SMSC LAN USB ethernet driver module

// (currently testing IPC mechanisms)
// (and testing USB)

#define ATS_DYNLOADFLAG 0

//staload _ = "prelude/DATS/basics.dats"
//staload _ = "prelude/DATS/integer.dats"

staload UN = "prelude/SATS/unsafe.sats"
staload "ipcmem.sats"
staload "multireader.sats"
staload _ = "multireader.dats"
staload "fixedslot.sats"
staload _ = "fixedslot.dats"
staload "prelude/SATS/status.sats"
staload "usb.sats"
staload "smsclan.sats"
// staload _ = "usb.dats"
#include "atspre_staload.hats"


extern fun atsmain (): int = "ext#atsmain"
extern fun mydelay (): void = "mac#mydelay"

extern fun printnum (_: int): void = "mac#printnum"

typedef ehci_info_t = int

fun test_fixedslot():int = let
  fun do_nothing (): void = do_nothing ()
  fun loop (fs: !fixedslot >> _, p: int): void = let
    val x = fixedslot_read<int> fs
  in
    if x > 40 then printnum x else
      begin
        (if x = p then () else printnum x);
        loop (fs, x)
      end
  end // end [loop]

  var pages: int?
  val (opt_pf | ms) = ipcmem_get_view (0, pages)
in
  if ms > 0 then
    let
      prval Some_v pf_ipcmem = opt_pf
      // fixedslot version
      val fs = fixedslot_initialize_reader (pf_ipcmem | ms, pages)
      val _  = loop (fs, 0)
      val (pf_ipcmem | ms) = fixedslot_free fs
      prval _ = ipcmem_put_view pf_ipcmem
    in
      do_nothing (); 0
    end else 1 where {
    // failed to acquire ipcmem
    prval None_v () = opt_pf
    val _ = do_nothing ()
  }
end // [fun test_fixedslot]

implement atsmain () = let

  val _ = test_fixedslot ()

(*
  fun loop {l: addr} {n, i: nat} (
        pf_ms: !mslot_v (l, n, ehci_info_t, false, i) |
        ms: ptr l, i: int i
      ): void = let
    val x = multireader_read (pf_ms | ms, i)
  in
    if x > 40 then printnum x else loop (pf_ms | ms, i)
  end // end [loop]
*)



(*    // multireader version
      var i: int?
      val (e_pf | s) = multireader_initialize_reader (pf_ipcmem | ms, pages, i)
    in
      if s = 0 then 0 where {
        prval Right_v pf_ms = e_pf


        val _ = loop (pf_ms | ms, i)
        val _ = do_nothing ()

        prval pf_ipcmem = multireader_release pf_ms
        prval _ = ipcmem_put_view pf_ipcmem

      } else 1 where {
        // failed to initialize
        prval Left_v pf_ipcmem = e_pf
        prval _ = ipcmem_put_view pf_ipcmem
        val _ = do_nothing ()
      }

*)
in 0
end // end [atsmain]

abst@ype usb_dev_desc_t = $extype "usb_dev_desc_t"
extern fun dump_usb_dev_desc (&usb_dev_desc_t): void = "mac#dump_usb_dev_desc"
extern fun get_usb_dev_desc_vendor(&usb_dev_desc_t): int = "mac#get_usb_dev_desc_vendor"

extern fun test_usb (): void = "test_usb"

fun wait_loop {i, nTDs: nat | nTDs > 0} (
    xfer_v: !usb_transfer_status i |
    usbd: !usb_device_vt (i, nTDs, true) >> usb_device_vt (i, nTDs, false)
  ): void =
let val s = usb_transfer_chain_active (xfer_v | usbd) in
    if s = OK then wait_loop (xfer_v | usbd) else ()
end
// end [wait_loop]

implement test_usb () =
let
  val usbd = usb_acquire_device 1
  var devr: usb_dev_req_ptr0?
  var devdesc: usb_dev_desc_t
  val (xfer_v | s) =
    usb_begin_control_read (usbd, devr, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                            USB_TYPE_DEV_DESC * 256, 0,
                            $UN.cast{intGte(0)}(g1ofg0 (sizeof<usb_dev_desc_t>)), addr@ devdesc)
in
  if s = OK then begin
    wait_loop (xfer_v | usbd);
    usb_transfer_completed (xfer_v | usbd);
    usb_release_device usbd;
    usb_dev_req_pool_free_ref devr;
    let var dd = ($UN.cast{usb_dev_desc_t}(devdesc)) in dump_usb_dev_desc dd end
  end else begin
    usb_transfer_completed (xfer_v | usbd);
    usb_release_device usbd;
    let prval _ = usb_dev_req_pool_free_null devr in end
  end
end


%{$

int main(void)
{
  svc3("SMSCLAN\n", 9, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);


  //while(*((volatile u32 *) (_ipcmappings[1].address)) == 0)   ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  while(__atomic_load_n((u32 *) _ipcmappings[1].address, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  DLOG(1, "smsclan ****************** &usbdevices[0]=%#x\n", &usbd[0]);
  //while(*((volatile u32 *) (&usbd[1].flags)) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[1].flags));
  while(__atomic_load_n(&usbd[1].flags, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[1].flags));
  DLOG(1, "smsclan ****************** usbd->qh.capabilities=%#x\n", usbd->qh.capabilities);
  DLOG(1, "smsclan ****************** invoking test_usb\n");
  //test(); // Test USB
  test_usb (); // Test ATS version of USB code
  DLOG(1, "smsclan ****************** invoking atsmain\n");
  return atsmain();
}

%}

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
