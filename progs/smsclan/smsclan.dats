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

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// iff
infix <==>
stadef <==> (p: bool, q: bool) = (~p || q) && (p || ~q)


extern fun atsmain (): int = "ext#atsmain"
extern fun mydelay (): void = "mac#mydelay"

extern fun printnum (_: int): void = "mac#printnum"

typedef ehci_info_t = int

extern fun wait_for_ehci_info (): void = "ext#wait_for_ehci_info"
implement wait_for_ehci_info (): void = let
  fun loop (fs: !fixedslotr int >> _): void = let
    val x = fixedslot_read<int> fs
  in
    if x > 0 then () else loop fs
  end // end [loop]

  var pages: int?
  val (opt_pf | ms) = ipcmem_get_view (0, pages)
in
  if ms > 0 then
    let
      prval Some_v pf_ipcmem = opt_pf
      // fixedslot version
      val fs = fixedslot_initialize_reader (pf_ipcmem | ms, pages)
      val _  = loop (fs)
      val (pf_ipcmem | ms) = fixedslot_free fs
      prval _ = ipcmem_put_view pf_ipcmem
    in
      ()
    end else () where {
    // failed to acquire ipcmem
    prval None_v () = opt_pf
  }
end // [fun wait_for_ehci_info]

extern fun wait_for_ehci_info2 (): void = "ext#wait_for_ehci_info2"
implement wait_for_ehci_info2 (): void = let
  fun loop {l: addr} {n, i: nat}
           (pf_ms: ! mslot_v (l, n, int, false, i) | ms: ptr l, i: int i): void = let
    val x = multireader_read<int> (pf_ms | ms, i)
  in
    if x > 0 then () else loop (pf_ms | ms, i)
  end // end [loop]

  var pages: int?
  val (opt_pf | ms) = ipcmem_get_view (0, pages)
in
  if ms > 0 then
    let
      prval Some_v pf_ipcmem = opt_pf
      var index: int
      val (e_pf | s) = multireader_initialize_reader<int> (pf_ipcmem | ms, pages, index)
    in
      if s = 0 then () where {
        prval Right_v (pf_ms) = e_pf

        val _ = loop (pf_ms | ms, index)

        prval pf_ipcmem = multireader_release pf_ms
        prval _         = ipcmem_put_view pf_ipcmem
      } else () where {
        prval Left_v pf_ipcmem = e_pf
        prval _                = ipcmem_put_view pf_ipcmem
      }
    end else () where {
    // failed to acquire ipcmem
    prval None_v () = opt_pf
  }
end // [fun wait_for_ehci_info]

fun test_fixedslot():int = let
  fun do_nothing (): void = do_nothing ()
  fun loop (fs: !fixedslotr int >> _, p: int): void = let
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
extern fun dump_usb_dev_qh (!usb_device): void = "mac#dump_usb_dev_qh"
extern fun dump_buf {n,m: nat | n <= m} (& @[uint8][m], int n): void = "mac#dump_buf"
extern fun dump_bufptr {n,m: nat | n <= m} {l: agz} (!((@[uint8][m]) @ l) | ptr l, int n): void = "mac#dump_buf"

extern fun swap_endian32 (uint): uint = "mac#swap_endian32"

extern fun smsclan_operate (): void = "smsclan_operate"

fun print_dev_desc {i: nat} (usbd: !usb_device_vt (i)): void = () where { val _ = usb_with_urb (usbd, 0, 0, f) }
where {
  val size = sizeof_usb_dev_desc_t
  var f = lam@ (usbd: !usb_device_vt i, urb: !urb_vt (i, 0, false)): [s: int] status s =<clo1> let
    val devdesc = usb_buf_alloc size
  in if ptrcast devdesc = 0 then ENOSPACE where { prval _ = usb_buf_release_null devdesc } else let
    val (xfer_v | s) =
      urb_begin_control_read (urb, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                              USB_TYPE_DEV_DESC << 8, 0,
                              size, devdesc)
  in
    if s = OK then begin
      urb_wait_while_active (xfer_v | urb);
      urb_transfer_completed (xfer_v | urb);
      let
        prval _ = lemma_sizeof_usb_dev_desc_t ()
        val (pf, fpf | p) = usb_buf_takeout devdesc
        val _ = dump_usb_dev_desc (!p)
        prval _ = devdesc := fpf pf
      in end;
      usb_buf_release devdesc;
      urb_detach_and_free urb
    end else begin
      urb_transfer_completed (xfer_v | urb);
      usb_buf_release devdesc;
      urb_detach_and_free_ urb; s
    end
  end end // [print_dev_desc]
}

fun print_cfg_desc {i: nat} (usbd: !usb_device_vt (i), cfgidx: int): void = () where { val _ = usb_with_urb (usbd, 0, 0, f) }
where {
  val size = sizeof_usb_dev_desc_t
  var f = lam@ (usbd: !usb_device_vt i, urb: !urb_vt (i, 0, false)): [s: int] status s =<clo1> let
    val desc = usb_buf_alloc size
  in if ptrcast desc = 0 then ENOSPACE where { prval _ = usb_buf_release_null desc } else let
    val (xfer_v | s) =
      urb_begin_control_read (urb, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                              (USB_TYPE_CFG_DESC << 8) + cfgidx, 0,
                              size, desc)
  in
    if s = OK then begin
      urb_wait_while_active (xfer_v | urb);
      urb_transfer_completed (xfer_v | urb);
      let
        prval _ = lemma_sizeof_usb_cfg_desc_t ()
        val (pf, fpf | p) = usb_buf_takeout desc
        val _ = dump_usb_cfg_desc (!p)
        prval _ = desc := fpf pf
      in end;
      usb_buf_release desc;
      urb_detach_and_free urb
    end else begin
      urb_transfer_completed (xfer_v | urb);
      usb_buf_release desc;
      urb_detach_and_free_ urb; s
    end
  end end // [print_cfg_desc]
}

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

fun _smsclan_read_reg {i: nat} (urb: !urb_vt (i, 0, false), idx: uint, value: &uint? >> uint): [s: int] status s =
let
  val buf = usb_buf_alloc 4
in if ptrcast buf = 0 then ENOSPACE where { prval _ = usb_buf_release_null buf
                                            val _   = value := g1int2uint 0 } else let
  val (xfer_v | s) =
    urb_begin_control_read (urb, make_RequestType (DeviceToHost, Vendor, Device),
                            make_VendorRequest USB_VENDOR_REQUEST_READ_REGISTER,
                            0, g0uint2int idx, 4, buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    let val s = urb_detach_and_free urb in
      if s = OK then begin value := usb_buf_get_uint buf; usb_buf_release buf; OK end
                else begin value := g1int2uint 0; usb_buf_release buf; s end
    end
  end else begin
    urb_transfer_completed (xfer_v | urb);
    urb_detach_and_free_ urb;
    value := g1int2uint 0;
    usb_buf_release buf;
    s
  end
end end

fun smsclan_read_reg {i: nat} (usbd: !usb_device_vt (i), idx: uint, value: &uint? >> uint): [s: int] status s =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in
  if s != OK then
    s where { prval _ = usb_device_release_null_urb urb
              val   _ = value := g1int2uint 0 }
  else let
    val s = _smsclan_read_reg (urb, idx, value)
  in
    usb_device_release_urb (usbd, urb); s
  end
end // [smsclan_read_reg]

extern fun smsclan_write_wait (): void = "mac#smsclan_write_wait"
fun smsclan_write_reg {i: nat} (usbd: !usb_device_vt (i), idx: uint, value: uint): [s: int] status s =
let
  val buf = usb_buf_alloc 4
in if ptrcast buf = 0 then ENOSPACE where { prval _ = usb_buf_release_null buf } else let
  val _ = usb_buf_set_uint (buf, value)
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb
            val   _ = usb_buf_release buf }
else let
  val (xfer_v | s) =
    urb_begin_control_write (urb, make_RequestType (HostToDevice, Vendor, Device),
                             make_VendorRequest USB_VENDOR_REQUEST_WRITE_REGISTER,
                             0, g0uint2int idx, 4, buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    // for some reason, the transmitter is much happier if I put a slight delay at this point:
    smsclan_write_wait ();
    usb_buf_release buf;
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_buf_release buf;
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end end // [smsclan_write_reg]

fun smsclan_get_all_regs {i: nat} (usbd: !usb_device_vt (i), idx: uint): [s: int] status s =
if idx < 305 then let
  var reg: uint
  val s = smsclan_read_reg (usbd, idx, reg)
in
  if s = OK then begin
    $extfcall (void, "debuglog", 0, 1, "SMSC[%#x]=%#x\n", idx, reg);
    smsclan_get_all_regs (usbd, idx + g1int2uint 4)
  end else s
end else OK

///* Rx status word */
#define RX_STS_FF_			(g1int2uint 0x40000000)	///* Filter Fail */
#define RX_STS_FL_			(g1int2uint 0x3FFF0000)	///* Frame Length */
#define RX_STS_ES_			(g1int2uint 0x00008000)	///* Error Summary */
#define RX_STS_BF_			(g1int2uint 0x00002000)	///* Broadcast Frame */
#define RX_STS_LE_			(g1int2uint 0x00001000)	///* Length Error */
#define RX_STS_RF_			(g1int2uint 0x00000800)	///* Runt Frame */
#define RX_STS_MF_			(g1int2uint 0x00000400)	///* Multicast Frame */
#define RX_STS_TL_			(g1int2uint 0x00000080)	///* Frame too long */
#define RX_STS_CS_			(g1int2uint 0x00000040)	///* Collision Seen */
#define RX_STS_FT_			(g1int2uint 0x00000020)	///* Frame Type */
#define RX_STS_RW_			(g1int2uint 0x00000010)	///* Receive Watchdog */
#define RX_STS_ME_			(g1int2uint 0x00000008)	///* Mii Error */
#define RX_STS_DB_			(g1int2uint 0x00000004)	///* Dribbling */
#define RX_STS_CRC_			(g1int2uint 0x00000002)	///* CRC Error */

// Tx command words
#define TX_CMD_A_DATA_OFFSET_		(g1int2uint 0x001F0000)
#define TX_CMD_A_FIRST_SEG_		(g1int2uint 0x00002000)
#define TX_CMD_A_LAST_SEG_		(g1int2uint 0x00001000)
#define TX_CMD_A_BUF_SIZE_		(g1int2uint 0x000007FF)

#define TX_CMD_B_CSUM_ENABLE		(g1int2uint 0x00004000)
#define TX_CMD_B_ADD_CRC_DISABLE_	(g1int2uint 0x00002000)
#define TX_CMD_B_DISABLE_PADDING_	(g1int2uint 0x00001000)
#define TX_CMD_B_PKT_BYTE_LENGTH_	(g1int2uint 0x000007FF)


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

fun smsclan_eeprom_confirm_not_busy {i: nat} (usbd: !usb_device_vt (i)): [s: int] status s =
let fun loop {i: nat} (usbd: !usb_device_vt (i), timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, E2P_CMD, reg)
      val s' = if s != 0 then s else if (reg land E2P_CMD_BUSY_) = 0 then OK else
               if timeout > 100 then ETIMEOUT else loop (usbd, timeout + 1)
    }
in
  loop (usbd, 0)
end
// [smsclan_eeprom_confirm_not_busy]

fun smsclan_eeprom_wait {i: nat} (usbd: !usb_device_vt (i)): [s: int] status s =
let fun loop {i: nat} (usbd: !usb_device_vt (i), timeout: int): [s: int] status s = s' where {
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
    usbd: !usb_device_vt (i), offset: uint, n: int n, data: &(@[uint][n])
  ): [s: int] status s =
let val s = smsclan_eeprom_confirm_not_busy usbd in if s != OK then s else
let fun loop {i, j: nat} (usbd: !usb_device_vt (i), j: int j, data: &(@[uint][n])): [s: int] status s =
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

fun smsclan_get_mac_addr {i: nat} (usbd: !usb_device_vt (i), lo: &uint? >> uint, hi: &uint? >> uint): [s: int] status s =
let
  val s1 = smsclan_read_reg (usbd, ADDRL, lo)
  val s2 = smsclan_read_reg (usbd, ADDRH, hi)
in
  if s1 = OK then if s2 = OK then OK else s2 else s1
end

fun smsclan_set_mac_addr {i: nat} (usbd: !usb_device_vt (i), lo: uint, hi: uint): [s: int] status s =
let
  val s1 = smsclan_write_reg (usbd, ADDRL, lo)
  val s2 = smsclan_write_reg (usbd, ADDRH, hi)
in
  if s1 = OK then if s2 = OK then OK else s2 else s1
end

(*
fun smsclan_get_rx_statistics {i, n: nat | n >= 8} {l: agz} (pf: !(@[uint][n]) @ l | usbd: !usb_device_vt i, stats: ptr l): [s: int] status s =
let
  val buf = usb_buf_alloc 32
in if ptrcast buf = 0 then ENOSPACE where { prval _ = usb_buf_release_null buf } else let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb
            val   _ = usb_buf_release buf }
else let
  val (xfer_v | s) =
    urb_begin_control_read (urb, make_RequestType (DeviceToHost, Vendor, Device),
                            make_VendorRequest USB_VENDOR_REQUEST_GET_STATS,
                            0, 0, 32, buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    // FIXME: memcpy buf into stats
    usb_buf_release buf;
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_buf_release buf;
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end end // [smsclan_get_rx_statistics]

fun smsclan_get_tx_statistics {i, n: nat | n >= 10} {l: agz} (pf: !(@[uint][n]) @ l | usbd: !usb_device_vt i, stats: ptr l): [s: int] status s =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb }
else let
  val (xfer_v | s) =
    urb_begin_control_read (pf | urb, make_RequestType (DeviceToHost, Vendor, Device),
                            make_VendorRequest USB_VENDOR_REQUEST_GET_STATS,
                            0, 1, 10, stats)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end // [smsclan_get_tx_statistics]
*)

#define ID_REV                          (g1int2uint 0x00)

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

#define BURST_CAP			(g1int2uint 0x38)
#define BULK_IN_DLY			(g1int2uint 0x6C)
#define DEFAULT_BULK_IN_DELAY		(g1int2uint 0x800) // 0x00002000)
#define NET_IP_ALIGN                    (g1int2uint 0) // let's just do 0

#define INT_STS				(g1int2uint 0x08)
#define INT_STS_TX_STOP_		(g1int2uint 0x00020000)
#define INT_STS_RX_STOP_		(g1int2uint 0x00010000)
#define INT_STS_PHY_INT_		(g1int2uint 0x00008000)
#define INT_STS_TXE_			(g1int2uint 0x00004000)
#define INT_STS_TDFU_			(g1int2uint 0x00002000)
#define INT_STS_TDFO_			(g1int2uint 0x00001000)
#define INT_STS_RXDF_			(g1int2uint 0x00000800)
#define INT_STS_GPIOS_			(g1int2uint 0x000007FF)
#define INT_STS_CLEAR_ALL_		(g1int2uint 0xFFFFFFFF)

#define LED_GPIO_CFG			(g1int2uint 0x24)
#define LED_GPIO_CFG_SPD_LED		(g1int2uint 0x01000000)
#define LED_GPIO_CFG_LNK_LED		(g1int2uint 0x00100000)
#define LED_GPIO_CFG_FDX_LED		(g1int2uint 0x00010000)

#define FLOW				(g1int2uint 0x11C)
#define FLOW_FCPT_			(g1int2uint 0xFFFF0000)
#define FLOW_FCPASS_			(g1int2uint 0x00000004)
#define FLOW_FCEN_			(g1int2uint 0x00000002)
#define FLOW_FCBSY_			(g1int2uint 0x00000001)

#define AFC_CFG				(g1int2uint 0x2C)
// Hi watermark = 15.5Kb (~10 mtu pkts)
// low watermark = 3k (~2 mtu pkts)
// backpressure duration = ~ 350us
// Apply FC on any frame.
#define AFC_CFG_DEFAULT			(g1int2uint 0x00F830A1)

#define VLAN1				(g1int2uint 0x120)
#define VLAN2				(g1int2uint 0x124)

// 802.1Q VLAN Extended Header
#define ETH_P_8021Q                     (g1int2uint 0x8100)

#define COE_CR				(g1int2uint 0x130)
#define Tx_COE_EN_			(g1int2uint 0x00010000)
#define Rx_COE_MODE_			(g1int2uint 0x00000002)
#define Rx_COE_EN_			(g1int2uint 0x00000001)

#define INT_EP_CTL			(g1int2uint 0x68)
#define INT_EP_CTL_INTEP_		(g1int2uint 0x80000000)
#define INT_EP_CTL_MACRTO_		(g1int2uint 0x00080000)
#define INT_EP_CTL_TX_STOP_		(g1int2uint 0x00020000)
#define INT_EP_CTL_RX_STOP_		(g1int2uint 0x00010000)
#define INT_EP_CTL_PHY_INT_		(g1int2uint 0x00008000)
#define INT_EP_CTL_TXE_			(g1int2uint 0x00004000)
#define INT_EP_CTL_TDFU_		(g1int2uint 0x00002000)
#define INT_EP_CTL_TDFO_		(g1int2uint 0x00001000)
#define INT_EP_CTL_RXDF_		(g1int2uint 0x00000800)
#define INT_EP_CTL_GPIOS_		(g1int2uint 0x000007FF)

#define MAC_CR				(g1int2uint 0x100)
#define MAC_CR_RXALL_			(g1int2uint 0x80000000)
#define MAC_CR_RCVOWN_			(g1int2uint 0x00800000)
#define MAC_CR_LOOPBK_			(g1int2uint 0x00200000)
#define MAC_CR_FDPX_			(g1int2uint 0x00100000)
#define MAC_CR_MCPAS_			(g1int2uint 0x00080000)
#define MAC_CR_PRMS_			(g1int2uint 0x00040000)
#define MAC_CR_INVFILT_			(g1int2uint 0x00020000)
#define MAC_CR_PASSBAD_			(g1int2uint 0x00010000)
#define MAC_CR_HFILT_			(g1int2uint 0x00008000)
#define MAC_CR_HPFILT_			(g1int2uint 0x00002000)
#define MAC_CR_LCOLL_			(g1int2uint 0x00001000)
#define MAC_CR_BCAST_			(g1int2uint 0x00000800)
#define MAC_CR_DISRTY_			(g1int2uint 0x00000400)
#define MAC_CR_PADSTR_			(g1int2uint 0x00000100)
#define MAC_CR_BOLMT_MASK		(g1int2uint 0x000000C0)
#define MAC_CR_DFCHK_			(g1int2uint 0x00000020)
#define MAC_CR_TXEN_			(g1int2uint 0x00000008)
#define MAC_CR_RXEN_			(g1int2uint 0x00000004)

#define TX_CFG				(g1int2uint 0x10)
#define TX_CFG_ON_			(g1int2uint 0x00000004)
#define TX_CFG_STOP_			(g1int2uint 0x00000002)
#define TX_CFG_FIFO_FLUSH_		(g1int2uint 0x00000001)

fun smsclan_reset {i: nat} (usbd: !usb_device_vt (i)): [s: int] status s =
// software reset -- seems to cause all kinds of problems
// let val s = smsclan_write_reg (usbd, HW_CFG, HW_CFG_SRST_) in if s != OK then s else
//let val () = print_dev_desc usbd in
// check for de-assert
let fun loop {i: nat} (usbd: !usb_device_vt (i), regnum: uint, test: uint, timeout: int): [s: int] status s = s' where {
      var reg: uint
      val s = smsclan_read_reg (usbd, regnum, reg)
      val s' = if s != 0 then s else if (reg land test) = 0 then OK else
               if timeout > 100 then ETIMEOUT else loop (usbd, regnum, test, timeout + 1)
    }
    in
//    val s = loop (usbd, HW_CFG, HW_CFG_SRST_, 0)     in if s != OK then s else

// PM PHY reset
// let val s = smsclan_write_reg (usbd, PM_CTRL, PM_CTL_PHY_RST_) in if s != OK then s else
// check for de-assert
// let val s = loop (usbd, PM_CTRL, PM_CTL_PHY_RST_, 0) in if s != OK then s else

let val s = smsclan_set_mac_addr (usbd, g1int2uint 0x6f3a0ddf, g1int2uint 0x02ac) in if s != OK then s else
let var regbuf: uint
    val s = smsclan_read_reg (usbd, HW_CFG, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x\n", regbuf);
let val s = smsclan_write_reg (usbd, HW_CFG, regbuf lor HW_CFG_BIR_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, HW_CFG, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x after HW_CFG_BIR_\n", regbuf);

//	if (!turbo_mode) {
//		burst_cap = 0;
//		dev->rx_urb_size = MAX_SINGLE_PACKET_SIZE;
//	} else if (dev->udev->speed == USB_SPEED_HIGH) {
//		burst_cap = DEFAULT_HS_BURST_CAP_SIZE / HS_USB_PKT_SIZE;
//		dev->rx_urb_size = DEFAULT_HS_BURST_CAP_SIZE;
//	} else {
//		burst_cap = DEFAULT_FS_BURST_CAP_SIZE / FS_USB_PKT_SIZE;
//		dev->rx_urb_size = DEFAULT_FS_BURST_CAP_SIZE;
//	}

let val s = smsclan_write_reg (usbd, BURST_CAP, g1int2uint 0) in if s != OK then s else // no turbo mode
let val s = smsclan_read_reg (usbd, BURST_CAP, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[BURST_CAP]=%#x\n", regbuf);
let val s = smsclan_write_reg (usbd, BULK_IN_DLY, DEFAULT_BULK_IN_DELAY) in if s != OK then s else // no turbo mode
let val s = smsclan_read_reg (usbd, BULK_IN_DLY, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[BULK_IN_DLY]=%#x\n", regbuf);
let val s = smsclan_read_reg (usbd, HW_CFG, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x\n", regbuf);
//	if (turbo_mode)
//		read_buf |= (HW_CFG_MEF_ | HW_CFG_BCE_);
let val s = smsclan_write_reg (usbd, HW_CFG, (regbuf land (lnot HW_CFG_RXDOFF_)) lor (NET_IP_ALIGN << 9)) in if s != OK then s else
let val s = smsclan_read_reg (usbd, HW_CFG, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[HW_CFG]=%#x\n", regbuf);
let val s = smsclan_write_reg (usbd, INT_STS, INT_STS_CLEAR_ALL_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, ID_REV, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[ID_REV]=%#x\n", regbuf);
// Configure GPIO pins as LED outputs
let val s = smsclan_write_reg (usbd, LED_GPIO_CFG, LED_GPIO_CFG_SPD_LED lor LED_GPIO_CFG_LNK_LED lor LED_GPIO_CFG_FDX_LED)
in if s != OK then s else

// Init Tx
let val s = smsclan_write_reg (usbd, FLOW, FLOW_FCEN_) in if s != OK then s else
let val s = smsclan_write_reg (usbd, AFC_CFG, AFC_CFG_DEFAULT) in if s != OK then s else

let var mac_cr: uint
    val s = smsclan_read_reg (usbd, MAC_CR, mac_cr) in if s != OK then s else
let val s = smsclan_write_reg (usbd, VLAN1, ETH_P_8021Q) in if s != OK then s else
// features = NETIF_F_HW_CSUM | NETIF_F_RXCSUM;
let val s = smsclan_read_reg (usbd, COE_CR, regbuf) in if s != OK then s else
let val s = smsclan_write_reg (usbd, COE_CR, regbuf lor Tx_COE_EN_ lor Rx_COE_EN_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, COE_CR, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[COE_CR]=%#x\n", regbuf);
// smsc95xx_set_multicast(dev->net); // skip for now
// smsc95xx_phy_initialize(dev); // skip for now
let val s = smsclan_read_reg (usbd, INT_EP_CTL, regbuf) in if s != OK then s else
let val s = smsclan_write_reg (usbd, INT_EP_CTL, regbuf lor INT_EP_CTL_PHY_INT_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, INT_EP_CTL, regbuf) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[INT_EP_CTL]=%#x\n", regbuf);

// start tx path

// Enable Tx at MAC
let val s = smsclan_write_reg (usbd, MAC_CR, mac_cr lor MAC_CR_TXEN_ lor MAC_CR_FDPX_) in if s != OK then s else
// Enable Tx at SCSRs
let val s = smsclan_write_reg (usbd, TX_CFG, TX_CFG_ON_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, MAC_CR, mac_cr) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[MAC_CR]=%#x\n", mac_cr);


// start rx path
let val s = smsclan_write_reg (usbd, MAC_CR, mac_cr lor MAC_CR_RXEN_) in if s != OK then s else
let val s = smsclan_read_reg (usbd, MAC_CR, mac_cr) in if s != OK then s else
begin $extfcall (void, "debuglog", 0, 1, "smsclan[MAC_CR]=%#x\n", mac_cr);


   OK
end end end end end end end end end end end end end end end end //end
end end end end end end end end end end end end end end end end end end end end end end end end end // flattened let-statements
// end [smsclan_reset]


extern fun uintarray2uint_le {n: nat | n >= 4} (!(@[uint8][n])): uint = "mac#get4bytes_le"
extern fun uintarrayptr2uint_le {n: nat | n >= 4} {l: agz} (!array_v (uint8, l, n) | ptr l): uint = "mac#get4bytes_le"
extern fun dump_urb (!urb2, int): void = "mac#dump_urb"

// should match value in uip-conf.h for BUFSIZE
stadef UIP_BUFSIZE = 1500
macdef UIP_BUFSIZE = $extval(uint UIP_BUFSIZE, "UIP_BUFSIZE")

stadef USBNET_BUFSIZE = 1600
macdef USBNET_BUFSIZE = 1600

vtypedef buf_vt (l: addr, m: int) = ((@[uint8][m]) @ l | ptr l)

extern fun copy_into_uip_buf {n, m: nat | n <= m && n <= UIP_BUFSIZE} {l: agz} (
    !usb_mem_vt (l, m), uint n
  ): void = "mac#copy_into_uip_buf"
extern fun copy_from_uip_buf {m: nat} {l: agz} (
    !usb_mem_vt (l, m)
  ): [n: nat | n <= m && n <= UIP_BUFSIZE] uint n = "mac#copy_from_uip_buf"

extern fun get_uip_len (): int = "mac#get_uip_len"
extern fun put_uip_len (int): void = "mac#put_uip_len"

fun dump_int_sts {i:nat} (usbd: !usb_device_vt i): void =
  let var reg: uint
  in if smsclan_read_reg (usbd, INT_STS, reg) != OK then () else
    $extfcall (void, "debuglog", 0, 1, "INT_STS=%#x\n", reg)
  end

(*
fun dump_statistics {i: nat} (usbd: !usb_device_vt i): void =
  let var rxstats = @[uint][8](g1int2uint 0) var txstats = @[uint][10](g1int2uint 0) in
    begin if smsclan_get_rx_statistics (view@ rxstats | usbd, addr@ rxstats) != OK then () else
    $extfcall (void, "debuglog", 0, 1, "RX STATS=%d %d %d %d %d %d %d %d\n",
               array_get_at (rxstats, 0),
               array_get_at (rxstats, 1),
               array_get_at (rxstats, 2),
               array_get_at (rxstats, 3),
               array_get_at (rxstats, 4),
               array_get_at (rxstats, 5),
               array_get_at (rxstats, 6),
               array_get_at (rxstats, 7))
    end;
    begin if smsclan_get_tx_statistics (view@ txstats | usbd, addr@ txstats) != OK then () else
    $extfcall (void, "debuglog", 0, 1, "TX STATS=%d %d %d %d %d %d %d %d %d %d\n",
               array_get_at (txstats, 0),
               array_get_at (txstats, 1),
               array_get_at (txstats, 2),
               array_get_at (txstats, 3),
               array_get_at (txstats, 4),
               array_get_at (txstats, 5),
               array_get_at (txstats, 6),
               array_get_at (txstats, 7),
               array_get_at (txstats, 8),
               array_get_at (txstats, 9))
    end
  end
*)

extern castfn g1uint2int {x: int} (uint x): int x

extern fun do_one_send {i, ntTDs: nat} {tactive: bool} {tl: agz} (
    txfer_v: !usb_transfer_status i |
    usbd: !usb_device_vt (i),
    turb: !urb_vt (i, ntTDs, tactive) >> urb_vt (i, ntTDs', tactive'),
    tbuf: !usb_mem_vt (tl, USBNET_BUFSIZE)
  ): #[ntTDs': nat] #[tactive': bool] void = "ext#do_one_send"

implement do_one_send (txfer_v | usbd, turb, tbuf) =
let
  val _ = urb_wait_if_active (txfer_v | turb);
  val _ = urb_transfer_completed (txfer_v | turb);
  val _ = urb_detach_and_free_ turb
  val txlen = copy_from_uip_buf (tbuf) // will leave 8 bytes for Tx Cmd A & B
  // txlen does not include command word lengths
  val tx_cmd_a = (txlen lor TX_CMD_A_LAST_SEG_) lor TX_CMD_A_FIRST_SEG_
  val tx_cmd_b = txlen
  val _ = usb_buf_set_uint_at (tbuf, 0, tx_cmd_a);
  val _ = usb_buf_set_uint_at (tbuf, 1, tx_cmd_b);
  //dump_buf (txbuf, g1uint2int (txlen + g1int2uint 8));
  val (xfer_v | s) = urb_begin_bulk_write (turb, g1uint2int (txlen + g1int2uint 8), tbuf)
  prval _ = txfer_v := xfer_v
in
  ()
end

//fun check_register {i: nat} (usbd: !usb_device_vt i, int, string)
fun check_registers {i: nat} (usbd: !usb_device_vt i): void =
let var buf: uint
    val s = smsclan_read_reg (usbd, MAC_CR, buf) in if s != OK then () else
let val _ = $extfcall (void, "debuglog", 0, 1, "MAC_CR=%#x\n", buf)
    val s = smsclan_read_reg (usbd, COE_CR, buf) in if s != OK then () else
let val _ = $extfcall (void, "debuglog", 0, 1, "COE_CR=%#x\n", buf)
    val s = smsclan_read_reg (usbd, HW_CFG, buf) in if s != OK then () else
let val _ = $extfcall (void, "debuglog", 0, 1, "HW_CFG=%#x\n", buf)
    val s = smsclan_read_reg (usbd, FLOW, buf) in if s != OK then () else
let val _ = $extfcall (void, "debuglog", 0, 1, "FLOW=%#x\n", buf)
    val s = smsclan_read_reg (usbd, AFC_CFG, buf) in if s != OK then () else
let val _ = $extfcall (void, "debuglog", 0, 1, "AFG_CFG=%#x\n", buf)
in () end end end end end end

extern fun smsclan_uip_loop {i: nat} (usbd: !usb_device_vt i): [s: int] status s = "ext#smsclan_uip_loop"
fun do_nothing (): void = do_nothing ()

implement smsclan_operate () =
let
  val usbd = usb_acquire_device 1
  //val () = print_dev_desc usbd
  //val () = print_cfg_desc (usbd, 0)

  val s = usb_set_configuration (usbd, 1)
  val () = $extfcall (void, "debuglog", 0, 1, "usb_set_configuration returned %d\n", s)
  var buf8: uint8
  val s = usb_get_configuration (usbd, buf8)
  val () = $extfcall (void, "debuglog", 0, 1, "usb_get_configuration returned (%d, %d)\n", s, buf8)

  val s = smsclan_reset usbd
  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_reset returned %d\n", s)

  val s = smsclan_set_mac_addr (usbd, g1int2uint 0x6f3a0ddf, g1int2uint 0x02ac)
  val () = $extfcall (void, "debuglog", 0, 1, "smsclan_set_mac_addr returned %d\n", s)

  var lo: uint
  var hi: uint
  val s = smsclan_get_mac_addr (usbd, lo, hi)
  val _ = $extfcall (void, "debuglog", 0, 1, "get_mac_addr: hi=%.4x lo=%.8x\n", hi, lo)

  val _ = $extfcall (void, "debuglog", 0, 1, "invoking smsclan_uip_loop\n")
  val _ = smsclan_uip_loop usbd
in
  usb_release_device usbd
end // [smsclan_operate]

// //////////////////////////////////////////////////
// UIP interface
abst@ype uip_timer_t = $extype "struct timer"
extern fun timer_set (&uip_timer_t? >> uip_timer_t, int): void = "mac#timer_set"
extern fun timer_reset (&uip_timer_t): void = "mac#timer_reset"
extern fun timer_expired (&uip_timer_t): bool = "mac#timer_expired"

extern fun uip_init (): void = "mac#uip_init"
abst@ype uip_eth_addr_t = $extype "struct uip_eth_addr"
extern fun uip_eth_addr (&uip_eth_addr_t? >> uip_eth_addr_t, int, int, int, int, int, int): void = "mac#uip_eth_addr"
extern fun uip_setethaddr (!uip_eth_addr_t): void = "mac#uip_setethaddr"
abst@ype uip_ipaddr_t = $extype "uip_ipaddr_t"
extern fun uip_ipaddr (!uip_ipaddr_t? >> uip_ipaddr_t, int, int, int, int): void = "mac#uip_ipaddr"
extern fun uip_sethostaddr(!uip_ipaddr_t): void = "mac#uip_sethostaddr"
extern fun uip_setdraddr(!uip_ipaddr_t): void = "mac#uip_setdraddr"
extern fun uip_setnetmask(!uip_ipaddr_t): void = "mac#uip_setnetmask"

extern fun uip_periodic(int): void = "mac#uip_periodic"
extern fun uip_arp_out(): void = "mac#uip_arp_out"
extern fun uip_arp_ipin(): void = "mac#uip_arp_ipin"
extern fun uip_arp_arpin(): void = "mac#uip_arp_arpin"
extern fun uip_arp_timer(): void = "mac#uip_arp_timer"
extern fun uip_udp_periodic(int): void = "mac#uip_udp_periodic"

extern fun uip_input(): void = "mac#uip_input"

extern fun get_uip_buftype (): uint = "mac#get_uip_buftype"

macdef UIP_ETHTYPE_IP = $extval(uint, "UIP_ETHTYPE_IP")
macdef UIP_ETHTYPE_ARP = $extval(uint, "UIP_ETHTYPE_ARP")

extern fun htons (uint): uint = "mac#htons" // just use uint for simplicity, should be ok

macdef CLOCK_SECOND = $extval(int, "CLOCK_SECOND")
macdef UIP_CONNS = $extval(int, "UIP_CONNS")
extern fun clock_time (): int = "mac#clock_time"

// //////////////////////////////////////////////////

implement smsclan_uip_loop {i} (usbd) = let
  fun uip_preamble (periodic_timer: &uip_timer_t? >> uip_timer_t, arp_timer: &uip_timer_t? >> uip_timer_t): void =
  let
    var ethaddr: uip_eth_addr_t
    var ipaddr: uip_ipaddr_t
  in
    timer_set (periodic_timer, CLOCK_SECOND / 2);
    timer_set (arp_timer, CLOCK_SECOND * 10);
    uip_init ();

    uip_eth_addr (ethaddr,0x02,0xac,0x6f,0x3a,0x0d,0xdf);
    uip_setethaddr ethaddr;

    uip_ipaddr (ipaddr, 10,0,0,2);
    uip_sethostaddr ipaddr;
    uip_ipaddr (ipaddr, 10,0,0,1);
    uip_setdraddr ipaddr;
    uip_ipaddr (ipaddr, 255,255,255,0);
    uip_setnetmask ipaddr
  end


  fun uip_periodic_check {i, ntTDs: nat} {tactive: bool} {tl: agz} (
      txfer_v: !usb_transfer_status i |
      usbd: !usb_device_vt i,
      turb: !urb_vt (i, ntTDs, tactive) >> urb_vt (i, ntTDs', tactive'),
      tbuf: !usb_mem_vt (tl, USBNET_BUFSIZE),
      periodic_timer: &uip_timer_t,
      arp_timer: &uip_timer_t
    ): #[ntTDs': nat] #[tactive': bool] void =
    if timer_expired periodic_timer then
      let
        fun loop1 {i, ntTDs: nat} {tactive: bool} {tl: agz} (
            txfer_v: !usb_transfer_status i |
            usbd: !usb_device_vt i,
            turb: !urb_vt (i, ntTDs, tactive) >> urb_vt (i, ntTDs', tactive'),
            tbuf: !usb_mem_vt (tl, USBNET_BUFSIZE),
            j: int
          ): #[ntTDs': nat] #[tactive': bool] void =
          if j >= UIP_CONNS then () else begin
            uip_periodic j;
            if get_uip_len () > 0 then begin
              uip_arp_out ();
              do_one_send (txfer_v | usbd, turb, tbuf)
            end else ();
            loop1 (txfer_v | usbd, turb, tbuf, j + 1)
          end
(*
        // UDP
        fun loop2 {i: nat} (usbd: !usb_device_vt i, j: int): void =
          if j >= UIP_CONNS then () else begin
            uip_udp_periodic j;
            if get_uip_len () > 0 then
              do_one_send (txfer_v | usbd, turb, tbuf) where { val _ = uip_arp_out () }
            else ();
            loop2 (usbd, j + 1)
          end
*)
      in
        timer_reset periodic_timer;
        loop1 (txfer_v | usbd, turb, tbuf, 0);
        //loop2 (usbd, 0);
        if timer_expired arp_timer then begin
          timer_reset arp_timer;
          uip_arp_timer ();
        end else ()
      end
    else ()

  fun loop_body {i, ntTDs, nrTDs: nat | nrTDs > 0} {rl, tl: agz} {tactive: bool} (
        txfer_v: !usb_transfer_status i |
        usbd: !usb_device_vt i,
        rurb: !urb_vt (i, nrTDs, false) >> urb_vt (i, nrTDs', ractive'),
        turb: !urb_vt (i, ntTDs, tactive) >> urb_vt (i, ntTDs', tactive'),
        rbuf: !usb_mem_vt (rl, USBNET_BUFSIZE),
        tbuf: !usb_mem_vt (tl, USBNET_BUFSIZE)
      ): #[s: int] #[ntTDs', nrTDs': nat | (s == 0) <==> (nrTDs' > 0)] #[tactive', ractive': bool | ractive' == (s == 0)]
         (usb_transfer_status i | status s) =
  let val s = urb_detach_and_free rurb in if s != OK then (usb_transfer_aborted | s) else
    let
      val header = usb_buf_get_uint rbuf
      val size = g1ofg0 ((header land RX_STS_FL_) >> 16)
    in
      if size > UIP_BUFSIZE
      then (usb_transfer_aborted | EINVALID)
      else let
        val _ = copy_into_uip_buf (rbuf, size);
        val (rxfer_v' | s) = urb_begin_bulk_read (rurb, 1536, rbuf); // begin next read
      in
        // process current read
        if get_uip_buftype () = htons (UIP_ETHTYPE_IP) then begin
          uip_arp_ipin ();
          uip_input ();
          if get_uip_len () > 0 then begin
            uip_arp_out ();
            do_one_send (txfer_v | usbd, turb, tbuf)
          end else ()
        end else if get_uip_buftype () = htons (UIP_ETHTYPE_ARP) then begin
          uip_arp_arpin ();
          if get_uip_len () > 0 then
            do_one_send (txfer_v | usbd, turb, tbuf)
          else ()
        end;
        (rxfer_v' | s)
      end
    end
  end

  // ATS helped sort out the resource alloc & clean-up for these functions
  fun loop {i, nrTDs, ntTDs: nat | nrTDs > 0} {rl, tl: agz} {tactive: bool} (
        rxfer_v: usb_transfer_status i,
        txfer_v: usb_transfer_status i |
        usbd: !usb_device_vt i,
        rurb: !urb_vt (i, nrTDs, true) >> urb_vt (i, 0, false),
        turb: !urb_vt (i, ntTDs, tactive) >> urb_vt (i, 0, false),
        rbuf: !usb_mem_vt (rl, USBNET_BUFSIZE),
        tbuf: !usb_mem_vt (tl, USBNET_BUFSIZE),
        periodic_timer: &uip_timer_t,
        arp_timer: &uip_timer_t,
        cyc_prev: int
      ): #[s: int | s != 0] status s =
  let
    val s = urb_transfer_chain_active (rxfer_v | rurb)
  in
    if s = OK then begin // No Rx
      uip_periodic_check (txfer_v | usbd, turb, tbuf, periodic_timer, arp_timer);
      loop (rxfer_v, txfer_v | usbd, rurb, turb, rbuf, tbuf, periodic_timer, arp_timer, cyc_prev)
    end else // Rx one packet
      let
        val lbody_start = clock_time ()
        val cyc_start = $extfcall (int, "arm_read_cycle_counter", ())
        val _ = urb_transfer_completed (rxfer_v | rurb)
        val (rxfer_v' | s) = loop_body (txfer_v | usbd, rurb, turb, rbuf, tbuf)
        val lbody_end = clock_time ()
        val cyc_end = $extfcall (int, "arm_read_cycle_counter", ())
        val lbody_dur = lbody_end - lbody_start
        val cyc_dur = cyc_end - cyc_start
        //val _ = $extfcall (void, "debuglog", 0, 1, "lbody_dur=%d cyc_dur=%d cyc_loop=%d\n", lbody_dur, cyc_dur, cyc_end - cyc_prev)
        val cyc_next = $extfcall (int, "arm_read_cycle_counter", ())
      in
        if s != 0 then s where { val _ = urb_transfer_abort (rxfer_v' | rurb)
                                 val _ = urb_transfer_abort (txfer_v | turb)
                                 val _ = urb_detach_and_free_ rurb
                                 val _ = urb_detach_and_free_ turb }
        else loop (rxfer_v', txfer_v | usbd, rurb, turb, rbuf, tbuf, periodic_timer, arp_timer, cyc_next)
      end
  end

  var start_loop = lam@ (usbd: !usb_device_vt i, rurb: !urb_vt (i, 0, false)): [s: int] status s =<clo1> let
    var turb: urb0?
    val s = usb_device_alloc_urb (usbd, 2, 512, turb)
  in if s != OK then s where { prval _ = usb_device_release_null_urb turb } else let
    val rxbuf = usb_buf_alloc USBNET_BUFSIZE
  in if ptrcast rxbuf = 0 then ENOSPACE where { prval _ = usb_buf_release_null rxbuf
                                                val   _ = usb_device_release_urb (usbd, turb) } else let
    val txbuf = usb_buf_alloc USBNET_BUFSIZE
  in if ptrcast txbuf = 0 then ENOSPACE where { prval _ = usb_buf_release_null txbuf
                                                val   _ = usb_buf_release rxbuf
                                                val   _ = usb_device_release_urb (usbd, turb) } else let
    var periodic_timer: uip_timer_t
    var arp_timer: uip_timer_t
    val _ = uip_preamble (periodic_timer, arp_timer)
    val (rxfer_v | s) = urb_begin_bulk_read (rurb, 1536, rxbuf)
  in
    if s != OK then
      s where { val _ = urb_transfer_abort (rxfer_v | rurb)
                val _ = usb_device_detach_and_release_urb_ (usbd, turb)
                val _ = usb_buf_release rxbuf
                val _ = usb_buf_release txbuf }
    else let
      prval txfer_v = usb_transfer_aborted
      val s = loop (rxfer_v, txfer_v | usbd, rurb, turb, rxbuf, txbuf, periodic_timer, arp_timer,
                    $extfcall (int, "arm_read_cycle_counter", ()))
    in
      usb_buf_release rxbuf; usb_buf_release txbuf; usb_device_detach_and_release_urb_ (usbd, turb); s
    end
  end end end end
in
  usb_with_urb (usbd, 1, 512, start_loop)
end



%{$

void appcall(void)
{
}

void uip_log(char *m)
{
  DLOG(1, "uIP log message: %s\n", m);
}

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

u8 buf1[1600], buf2[1600];

void network_setup (void)
{
  uip_ipaddr_t ipaddr;
  struct uip_eth_addr eaddr = {{0x02,0xac,0x6f,0x3a,0x0d,0xdf}};
  uip_setethaddr(eaddr);

  uip_ipaddr(ipaddr, 10,0,0,2);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, 10,0,0,1);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, 255,255,255,0);
  uip_setnetmask(ipaddr);
}

int main(void)
{
  svc3("SMSCLAN\n", 9, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);


  //while(*((volatile u32 *) (_ipcmappings[1].address)) == 0)   ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  //while(__atomic_load_n((u32 *) _ipcmappings[1].address, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  //usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  //DLOG(1, "smsclan ****************** &usbdevices[0]=%#x\n", &usbd[0]);
  //while(*((volatile u32 *) (&usbd[1].flags)) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[1].flags));
  //while(__atomic_load_n(&usbd[1].flags, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[1].flags));
  //DLOG(1, "smsclan ****************** usbd->qh.capabilities=%#x\n", QH( *usbd,0).capabilities);
  //test(); // Test USB
  //DLOG(1, "SMSCLAN timer=%d\n", clock_time());
  //DLOG(1, "smsclan ****************** invoking smsclan_operate\n");
  //DLOG(1, "smsclan ****************** &__ehci_td_pool_array=%#x &__ehci_td_pool_bitmap=%#x\n", get_ehci_td_pool_addr (), get_ehci_td_bitmap_addr ());

#if 0
  // Test memory reads
  u32 *ptr = (u32 *) 0x8007a040;
  for(;;) {
    //__atomic_load_n(&usbd[2].qh[0].capabilities, __ATOMIC_SEQ_CST);
    //__atomic_load_n(&usbd[2].qh[1].capabilities, __ATOMIC_SEQ_CST);
    //__atomic_load_n(&usbd[2].qh[2].capabilities, __ATOMIC_SEQ_CST);
    //__atomic_load_n(&usbd[2].qh[3].capabilities, __ATOMIC_SEQ_CST);
    __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
    __atomic_load_n(ptr+8, __ATOMIC_SEQ_CST);
    __atomic_load_n(ptr+16, __ATOMIC_SEQ_CST);
    __atomic_load_n(ptr+24, __ATOMIC_SEQ_CST);
  }
#endif

  DLOG(1, "smsclan ****************** wait_for_ehci_info\n");
  wait_for_ehci_info ();

  usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  DLOG(1, "smsclan ****************** &usbdevices[0]=%#x\n", &usbd[0]);
  DLOG(1, "smsclan ****************** usbd->qh.capabilities=%#x\n", QH( *usbd,0).capabilities);
  DLOG(1, "smsclan ****************** usbd->dev_desc.idVendor=%#x .idProduct=%#x\n", usbd->dev_desc.idVendor, usbd->dev_desc.idProduct);

  DLOG(1, "smsclan ****************** smsclan_operate\n");
  smsclan_operate (); // Run uIP with SMSCLAN


  //DLOG(1, "SMSCLAN timer=%d\n", clock_time());
  //DLOG(1, "smsclan ****************** invoking atsmain\n");
  //return atsmain();
  return 0;
}

%}

(*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: ATS
 * End:
 *)

(* vi: set et sw=2 sts=2: *)
////
implement smsclan_uip_loop (usbd) = let
  val (pfbuf1 | buf1) = alloc_rx_buf ()
  val (pfbuf2 | buf2) = alloc_rx_buf ()

  fun loop_body {i, nTDs: nat | nTDs > 0} {l: agz} (
        pfbuf: !rxbuf_v l |
        usbd: !usb_device_vt i,
        urb: !urb_vt (i, nTDs, false) >> urb_vt (i, nTDs', active'),
        buf: ptr l
      ): #[s: int] #[nTDs': nat | (s == 0) <==> (nTDs' > 0)] #[active': bool | active' == (s == 0)]
         (usb_transfer_status i | status s) =
  let val s = urb_detach_and_free urb in if s != OK then (usb_transfer_aborted | s) else
    let var bufref = $UN.cast{@[uint8][1600]}(buf) val _ = dump_buf (bufref, 64) in
      urb_begin_bulk_read (pfbuf | urb, 1536, buf)
    end
  end


  fun loop {i, nTDs1, nTDs2: nat | nTDs1 > 0 && nTDs2 > 0} {l1, l2: agz} (
        xfer1_v: usb_transfer_status i,
        xfer2_v: usb_transfer_status i,
        pfbuf1: !rxbuf_v l1,
        pfbuf2: !rxbuf_v l2 |
        usbd: !usb_device_vt i,
        urb1: !urb_vt (i, nTDs1, true) >> urb_vt (i, nTDs1', active1),
        urb2: !urb_vt (i, nTDs2, true) >> urb_vt (i, nTDs2', active2),
        buf1: ptr l1,
        buf2: ptr l2
      ): #[nTDs1', nTDs2': nat | nTDs1 > 0 && nTDs2 > 0] #[active1, active2: bool] void =
  let val s = urb_transfer_chain_active (xfer1_v | urb1) in
    if s = OK then let val s = urb_transfer_chain_active (xfer2_v | urb2) in
      if s = OK then loop (xfer1_v, xfer2_v, pfbuf1, pfbuf2 | usbd, urb1, urb2, buf1, buf2) else begin
        urb_transfer_completed (xfer2_v | urb2);
        let val (xfer2_v' | s) = loop_body (pfbuf2 | usbd, urb2, buf2) in
          if s != 0 then begin
            urb_transfer_abort (xfer2_v' | urb2);
            urb_transfer_abort (xfer1_v | urb1)
          end else loop (xfer1_v, xfer2_v', pfbuf1, pfbuf2 | usbd, urb1, urb2, buf1, buf2)
        end
      end
    end else begin
      urb_transfer_completed (xfer1_v | urb1);
      let val (xfer1_v' | s) = loop_body (pfbuf1 | usbd, urb1, buf1) in
        if s != 0 then begin
          urb_transfer_abort (xfer1_v' | urb1);
          urb_transfer_abort (xfer2_v | urb2)
        end else loop (xfer1_v', xfer2_v, pfbuf1, pfbuf2 | usbd, urb1, urb2, buf1, buf2)
      end
    end
  end

  fun start_loop {i: nat} {l1,l2: agz} (
      pfbuf1: !rxbuf_v l1,
      pfbuf2: !rxbuf_v l2 |
      usbd: !usb_device_vt i,
      buf1: ptr l1, buf2: ptr l2
    ): void = let
      var urb1: urb0?
      val s = usb_device_alloc_urb (usbd, urb1)
    in
      if s != OK then
        () where { prval _ = usb_device_release_null_urb urb1 }
      else let
        val _ = urb_set_endpoint (urb1, 1, 512)
        var urb2: urb0?
        val s = usb_device_alloc_urb (usbd, urb2)
      in
        if s != OK then
          () where { prval _ = usb_device_release_null_urb urb1 }
        else let
          val _ = urb_set_endpoint (urb1, 1, 512)
          val s = urb_begin_bulk_read (pfbuf1 | urb1, 1536, buf1)
        in if s != OK then

          usb_device_release_urb (usbd, urb);

  start_loop (pfbuf1, pfbuf2...
  release_rx_buf (pfbuf1 | buf1);
  release_rx_buf (pfbuf2 | buf2)
end
