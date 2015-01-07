// SMSC LAN USB ethernet driver module

// (currently testing IPC mechanisms)
// (and testing USB)

#define ATS_DYNLOADFLAG 0

staload UN = "prelude/SATS/unsafe.sats"
staload _ = "prelude/DATS/unsafe.dats"
staload "prelude/SATS/array.sats"
staload _ = "prelude/DATS/array.dats"
staload "ipcmem.sats"
staload "multireader.sats"
staload _ = "multireader.dats"
staload "fixedslot.sats"
staload _ = "fixedslot.dats"
staload "prelude/SATS/status.sats"
staload "prelude/SATS/bits.sats"
staload "usb.sats"
staload _ = "usb.dats"
staload "smsclan.sats"
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

in 0
end // end [atsmain]

extern fun dump_usb_dev_desc (&usb_dev_desc_t): void = "mac#dump_usb_dev_desc"
extern fun dump_usb_cfg_desc (&usb_cfg_desc_t): void = "mac#dump_usb_cfg_desc"
extern fun get_usb_dev_desc_vendor(&usb_dev_desc_t): int = "mac#get_usb_dev_desc_vendor"

extern fun test_usb (): void = "test_usb"

fun wait_loop {i, nTDs: nat | nTDs > 0} (
    xfer_v: !usb_transfer_status i |
    usbd: !usb_device_vt (i, nTDs, true) >> usb_device_vt (i, nTDs, false)
  ): void =
let val s = usb_transfer_chain_active (xfer_v | usbd) in
    if s = OK then wait_loop (xfer_v | usbd) else ()
end // [wait_loop]

fun print_dev_desc {i: nat} (usbd: !usb_device_vt (i, 0, false)): void =
let
  var devdesc = @[usb_dev_desc_t][1](usb_dev_desc_empty)
  val (xfer_v | s) =
    usb_begin_control_read (view@ devdesc |
                            usbd, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                            USB_TYPE_DEV_DESC << 8, 0,
                            1, addr@ devdesc)
in
  if s = OK then begin
    wait_loop (xfer_v | usbd);
    usb_transfer_completed (xfer_v | usbd);
    let var dd = array_get_at (devdesc, 0) in dump_usb_dev_desc dd end;
    usb_device_detach_and_free usbd
  end else begin
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd
  end
end // [print_dev_desc]

fun print_cfg_desc {i: nat} (usbd: !usb_device_vt (i, 0, false), cfgidx: int): void =
let
  var cfgdesc = @[usb_cfg_desc_t][1](usb_cfg_desc_empty)
  val (xfer_v | s) =
    usb_begin_control_read (view@ cfgdesc |
                            usbd, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                            USB_TYPE_CFG_DESC * 256 + cfgidx, 0,
                            1, addr@ cfgdesc)
in
  if s = OK then begin
    wait_loop (xfer_v | usbd);
    usb_transfer_completed (xfer_v | usbd);
    let var d = array_get_at (cfgdesc, 0) in dump_usb_cfg_desc d end;
    usb_device_detach_and_free usbd
  end else begin
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd
  end
end // [print_cfg_desc]


//  USB Vendor Requests
#define USB_VENDOR_REQUEST_WRITE_REGISTER       (g1int2uint 0xA0)
#define USB_VENDOR_REQUEST_READ_REGISTER        (g1int2uint 0xA1)
#define USB_VENDOR_REQUEST_GET_STATS            (g1int2uint 0xA2)

(*
/* SCSRs */
#define ID_REV                          (0x00)
#define ID_REV_CHIP_ID_MASK_            (0xFFFF0000)
#define ID_REV_CHIP_REV_MASK_           (0x0000FFFF)
#define ID_REV_CHIP_ID_9500_            (0x9500)
#define ID_REV_CHIP_ID_9500A_           (0x9E00)
#define ID_REV_CHIP_ID_9512_            (0xEC00)
#define ID_REV_CHIP_ID_9530_            (0x9530)
#define ID_REV_CHIP_ID_89530_           (0x9E08)
#define ID_REV_CHIP_ID_9730_            (0x9730)

*)

extern fun make_VendorRequest (i:uint): usb_Request_t = "mac#make_VendorRequest"

fun smsclan_read_reg {i: nat} (usbd: !usb_device_vt (i, 0, false), idx: uint, value: &uint? >> uint): [s: int] status s =
let
  var buf = @[uint][1](g1int2uint 0)
  val (xfer_v | s) =
    usb_begin_control_read (view@ buf | usbd, make_RequestType (DeviceToHost, Vendor, Device),
                            make_VendorRequest USB_VENDOR_REQUEST_READ_REGISTER,
                            0, g0uint2int idx, 1, addr@ buf)
in
  if s = OK then begin
    usb_wait_while_active (xfer_v | usbd);
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd;
    value := array_get_at (buf, 0);
    OK
  end else begin
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd;
    value := g1int2uint 0;
    s
  end
end // [smsclan_read_reg]

fun smsclan_write_reg {i: nat} (usbd: !usb_device_vt (i, 0, false), idx: uint, value: uint): [s: int] status s =
let
  var buf = @[uint][1](value)
  val (xfer_v | s) =
    usb_begin_control_write (view@ buf | usbd, make_RequestType (HostToDevice, Vendor, Device),
                             make_VendorRequest USB_VENDOR_REQUEST_WRITE_REGISTER,
                             0, g0uint2int idx, 1, addr@ buf)
in
  if s = OK then begin
    usb_wait_while_active (xfer_v | usbd);
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd;
    OK
  end else begin
    usb_transfer_completed (xfer_v | usbd);
    usb_device_detach_and_free usbd;
    s
  end
end // [smsclan_write_reg]

fun smsclan_get_all_regs {i: nat} (usbd: !usb_device_vt (i, 0, false), idx: uint): [s: int] status s =
if idx < 305 then let
  var reg: uint
  val s = smsclan_read_reg (usbd, idx, reg)
in
  if s = OK then begin
    $extfcall (void, "debuglog", 0, 1, "SMSC[%#x]=%#x\n", idx, reg);
    smsclan_get_all_regs (usbd, idx + g1int2uint 4)
  end else s
end else OK

#define E2P_CMD                         (g1int2uint 0x30)
#define E2P_CMD_BUSY_                   (g1int2uint 0x80000000)
#define E2P_CMD_MASK_                   (g1int2uint 0x70000000)
#define E2P_CMD_READ_                   (g1int2uint 0x00000000)
#define E2P_CMD_EWDS_                   (g1int2uint 0x10000000)
#define E2P_CMD_EWEN_                   (g1int2uint 0x20000000)
#define E2P_CMD_WRITE_                  (g1int2uint 0x30000000)
#define E2P_CMD_WRAL_                   (g1int2uint 0x40000000)
#define E2P_CMD_ERASE_                  (g1int2uint 0x50000000)
#define E2P_CMD_ERAL_                   (g1int2uint 0x60000000)
#define E2P_CMD_RELOAD_                 (g1int2uint 0x70000000)
#define E2P_CMD_TIMEOUT_                (g1int2uint 0x00000400)
#define E2P_CMD_LOADED_                 (g1int2uint 0x00000200)
#define E2P_CMD_ADDR_                   (g1int2uint 0x000001FF)
#define MAX_EEPROM_SIZE                 (g1int2uint 512)
#define E2P_DATA                        (g1int2uint 0x34)
#define E2P_DATA_MASK_                  (g1int2uint 0x000000FF)

#define EEPROM_MAC_OFFSET               (g1int2uint 0x1)

fun smsclan_eeprom_confirm_not_busy {i: nat} (usbd: !usb_device_vt (i, 0, false)): [s: int] status s =
let fun loop {i: nat} (usbd: !usb_device_vt (i, 0, false), timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, E2P_CMD, reg)
      val s' = if s != 0 then s else if (reg land E2P_CMD_BUSY_) = 0 then OK else
               if timeout > 100 then ETIMEOUT else loop (usbd, timeout + 1)
    }
in
  loop (usbd, 0)
end
// [smsclan_eeprom_confirm_not_busy]

fun smsclan_eeprom_wait {i: nat} (usbd: !usb_device_vt (i, 0, false)): [s: int] status s =
let fun loop {i: nat} (usbd: !usb_device_vt (i, 0, false), timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, E2P_CMD, reg)
      val _ =         $extfcall (void, "debuglog", 0, 1, "reg=%#x\n", reg)
      val s' = if s != 0 then s else if ((reg land E2P_CMD_BUSY_) = 0) || (reg land E2P_CMD_TIMEOUT_) > 0 then
               (if (reg land (E2P_CMD_BUSY_ lor E2P_CMD_TIMEOUT_)) > g1int2uint 0 then ETIMEOUT else OK)
               else if timeout > 100 then ETIMEOUT else loop (usbd, timeout + 1)
    }
in
  loop (usbd, 0)
end
// [smsclan_eeprom_wait]

fun smsclan_read_eeprom {i, n: nat} (
    usbd: !usb_device_vt (i, 0, false), offset: uint, n: int n, data: &(@[uint][n])
  ): [s: int] status s =
let val s = smsclan_eeprom_confirm_not_busy usbd in if s != OK then s else
let fun loop {i, j: nat} (usbd: !usb_device_vt (i, 0, false), j: int j, data: &(@[uint][n])): [s: int] status s =
      if j >= n then OK else
      let val v = E2P_CMD_BUSY_ lor E2P_CMD_READ_ lor ((offset + g1int2uint j) land E2P_CMD_ADDR_)
          val s = smsclan_write_reg (usbd, E2P_CMD, v) in if s != OK then s else
      let val s = smsclan_eeprom_wait usbd in if s != OK then s else
      let var v: uint
          val s = smsclan_read_reg (usbd, E2P_DATA, v) in if s != OK then s else
      begin
        array_set_at (data, j, v land (g1int2uint 0xFF));
        loop (usbd, j + 1, data)
      end end end end // flattened
    // end [loop]
in
  loop (usbd, 0, data)
end end // flattened
// end [smsclan_read_eeprom]

#define ADDRH                           (g1int2uint 0x104)
#define ADDRL                           (g1int2uint 0x108)

fun smsclan_get_mac_addr {i: nat} (usbd: !usb_device_vt (i, 0, false), lo: &uint? >> uint, hi: &uint? >> uint): [s: int] status s =
let
  val s1 = smsclan_read_reg (usbd, ADDRL, lo)
  val s2 = smsclan_read_reg (usbd, ADDRH, hi)
in
  if s1 = OK then if s2 = OK then OK else s2 else s1
end

fun smsclan_set_mac_addr {i: nat} (usbd: !usb_device_vt (i, 0, false), lo: uint, hi: uint): [s: int] status s =
let
  val s1 = smsclan_write_reg (usbd, ADDRL, lo)
  val s2 = smsclan_write_reg (usbd, ADDRH, hi)
in
  if s1 = OK then if s2 = OK then OK else s2 else s1
end

#define HW_CFG                          (g1int2uint 0x14)
#define HW_CFG_BIR_                     (g1int2uint 0x00001000)
#define HW_CFG_LEDB_                    (g1int2uint 0x00000800)
#define HW_CFG_RXDOFF_                  (g1int2uint 0x00000600)
#define HW_CFG_DRP_                     (g1int2uint 0x00000040)
#define HW_CFG_MEF_                     (g1int2uint 0x00000020)
#define HW_CFG_LRST_                    (g1int2uint 0x00000008)
#define HW_CFG_PSEL_                    (g1int2uint 0x00000004)
#define HW_CFG_BCE_                     (g1int2uint 0x00000002)
#define HW_CFG_SRST_                    (g1int2uint 0x00000001)

#define PM_CTRL                         (g1int2uint 0x20)
#define PM_CTL_RES_CLR_WKP_STS          (g1int2uint 0x00000200)
#define PM_CTL_DEV_RDY_                 (g1int2uint 0x00000080)
#define PM_CTL_SUS_MODE_                (g1int2uint 0x00000060)
#define PM_CTL_SUS_MODE_0               (g1int2uint 0x00000000)
#define PM_CTL_SUS_MODE_1               (g1int2uint 0x00000020)
#define PM_CTL_SUS_MODE_2               (g1int2uint 0x00000040)
#define PM_CTL_SUS_MODE_3               (g1int2uint 0x00000060)
#define PM_CTL_PHY_RST_                 (g1int2uint 0x00000010)
#define PM_CTL_WOL_EN_                  (g1int2uint 0x00000008)
#define PM_CTL_ED_EN_                   (g1int2uint 0x00000004)
#define PM_CTL_WUPS_                    (g1int2uint 0x00000003)
#define PM_CTL_WUPS_NO_                 (g1int2uint 0x00000000)
#define PM_CTL_WUPS_ED_                 (g1int2uint 0x00000001)
#define PM_CTL_WUPS_WOL_                (g1int2uint 0x00000002)
#define PM_CTL_WUPS_MULTI_              (g1int2uint 0x00000003)

fun smsclan_reset {i: nat} (usbd: !usb_device_vt (i, 0, false)): [s: int] status s =
// software reset
let val s = smsclan_write_reg (usbd, HW_CFG, HW_CFG_SRST_) in if s != OK then s else
// check for de-assert
let fun loop {i: nat} (usbd: !usb_device_vt (i, 0, false), timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, HW_CFG, reg)
      val s' = if s != 0 then s else if (reg land HW_CFG_SRST_) = 0 then OK else
               if timeout > 100 then ETIMEOUT else loop (usbd, timeout + 1)
    }
    val s = loop (usbd, 0) in if s != OK then s else

// PM PHY reset
let val s = smsclan_write_reg (usbd, PM_CTRL, PM_CTL_PHY_RST_) in if s != OK then s else
// check for de-assert
let fun loop {i: nat} (usbd: !usb_device_vt (i, 0, false), timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, PM_CTRL, reg)
      val s' = if s != 0 then s else if (reg land PM_CTL_PHY_RST_) = 0 then OK else
               if timeout > 100 then ETIMEOUT else loop (usbd, timeout + 1)
    }
    val s = loop (usbd, 0) in if s != OK then s else

let val s = smsclan_set_mac_addr (usbd, g1int2uint 0x6f3a0ddf, g1int2uint 0x02ac) in if s != OK then s else
let var hwcfg: uint
    val s = smsclan_read_reg (usbd, HW_CFG, hwcfg) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x\n", hwcfg);
let val s = smsclan_write_reg (usbd, HW_CFG, hwcfg lor HW_CFG_BIR_) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x after HW_CFG_BIR_\n", hwcfg);


   OK

end end end end end end end end end // flattened let-statements
// end [smsclan_reset]

implement test_usb () =
let
  val usbd = usb_acquire_device 1
  val () = print_dev_desc usbd
  val () = print_cfg_desc (usbd, 0)
  val s = usb_set_configuration (usbd, 1)
  val () = $extfcall (void, "debuglog", 0, 1, "usb_set_configuration returned %d\n", s)

  //val s = smsclan_get_all_regs (usbd, g1int2uint 0)

  val s = smsclan_eeprom_confirm_not_busy usbd
  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_eeprom_confirm_not_busy returned %d\n", s)

  var mac = @[uint][6](g1int2uint 0xff)
  val s = smsclan_read_eeprom (usbd, EEPROM_MAC_OFFSET, 6, mac)
  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_read_eeprom returned %d\n", s)
  val () = $extfcall (void, "debuglog", 0, 1, "mac=%#x:%#x:%#x:%#x:%#x:%#x\n",
                      array_get_at (mac, 0), array_get_at (mac, 1), array_get_at (mac, 2),
                      array_get_at (mac, 3), array_get_at (mac, 4), array_get_at (mac, 5))

  val s = smsclan_set_mac_addr (usbd, g1int2uint 0x6f3a0ddf, g1int2uint 0x02ac)
  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_set_mac_addr returned %d\n", s)

  //val s = smsclan_reset usbd
  //val () = $extfcall (void, "debuglog", 0, 1, "smsclan_reset returned %d\n", s)


//  var idrev: int
//  val s = smsclan_read_reg (usbd, 0, idrev)
//  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_read_reg returned (%d, %#x)\n", s, idrev)
  // val s = smsclan_get_all_regs (usbd, g1int2uint 0)

  var lo: uint
  var hi: uint
  val s = smsclan_get_mac_addr (usbd, lo, hi)
  val _ = $extfcall (void, "debuglog", 0, 1, "get_mac_addr = %x%x\n", hi, lo)

in
  usb_release_device usbd
end // [test_usb]


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
