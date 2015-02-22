// ASIX USB Ethernet driver

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
staload _ = "prelude/DATS/integer.dats"
staload "usb.sats"
staload _ = "usb.dats"
staload "asix.sats"
#include "atspre_staload.hats"

extern fun atsmain (): int = "ext#atsmain"

// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// iff
infix <==>
stadef <==> (p: bool, q: bool) = (~p || q) && (p || ~q)

macdef byte0 = $extval(uint8, "0")
macdef word0 = $extval(uint16, "0")

fun{a:vt0p} sizeofGEZ (): [s: nat] int s = $UN.cast{intGte(0)}(g1ofg0 (sizeof<a>))

// //////////////////////////////////////////////////

#define AX_CMD_SET_SW_MII		(g1int2uint 0x06)
#define AX_CMD_READ_MII_REG		(g1int2uint 0x07)
#define AX_CMD_WRITE_MII_REG		(g1int2uint 0x08)
#define AX_CMD_SET_HW_MII		(g1int2uint 0x0a)
#define AX_CMD_READ_EEPROM		(g1int2uint 0x0b)
#define AX_CMD_WRITE_EEPROM		(g1int2uint 0x0c)
#define AX_CMD_WRITE_ENABLE		(g1int2uint 0x0d)
#define AX_CMD_WRITE_DISABLE		(g1int2uint 0x0e)
#define AX_CMD_READ_RX_CTL		(g1int2uint 0x0f)
#define AX_CMD_WRITE_RX_CTL		(g1int2uint 0x10)
#define AX_CMD_READ_IPG012		(g1int2uint 0x11)
#define AX_CMD_WRITE_IPG0		(g1int2uint 0x12)
#define AX_CMD_WRITE_IPG1		(g1int2uint 0x13)
#define AX_CMD_READ_NODE_ID		(g1int2uint 0x13)
#define AX_CMD_WRITE_NODE_ID		(g1int2uint 0x14)
#define AX_CMD_WRITE_IPG2		(g1int2uint 0x14)
#define AX_CMD_WRITE_MULTI_FILTER	(g1int2uint 0x16)
#define AX88172_CMD_READ_NODE_ID	(g1int2uint 0x17)
#define AX_CMD_READ_PHY_ID		(g1int2uint 0x19)
#define AX_CMD_READ_MEDIUM_STATUS	(g1int2uint 0x1a)
#define AX_CMD_WRITE_MEDIUM_MODE	(g1int2uint 0x1b)
#define AX_CMD_READ_MONITOR_MODE	(g1int2uint 0x1c)
#define AX_CMD_WRITE_MONITOR_MODE	(g1int2uint 0x1d)
#define AX_CMD_READ_GPIOS		(g1int2uint 0x1e)
#define AX_CMD_WRITE_GPIOS		(g1int2uint 0x1f)
#define AX_CMD_SW_RESET			(g1int2uint 0x20)
#define AX_CMD_SW_PHY_STATUS		(g1int2uint 0x21)
#define AX_CMD_SW_PHY_SELECT		(g1int2uint 0x22)

// //////////////////////////////////////////////////

// /* GPIO 0 .. 2 toggles */
#define AX_GPIO_GPO0EN		(g1int2uint 0x01) /* GPIO0 Output enable */
#define AX_GPIO_GPO_0		(g1int2uint 0x02) /* GPIO0 Output value */
#define AX_GPIO_GPO1EN		(g1int2uint 0x04) /* GPIO1 Output enable */
#define AX_GPIO_GPO_1		(g1int2uint 0x08) /* GPIO1 Output value */
#define AX_GPIO_GPO2EN		(g1int2uint 0x10) /* GPIO2 Output enable */
#define AX_GPIO_GPO_2		(g1int2uint 0x20) /* GPIO2 Output value */
#define AX_GPIO_RESERVED	(g1int2uint 0x40) /* Reserved */
#define AX_GPIO_RSE		(g1int2uint 0x80) /* Reload serial EEPROM */

// //////////////////////////////////////////////////

#define AX_SWRESET_CLEAR		(g1int2uint 0x00)
#define AX_SWRESET_RR			(g1int2uint 0x01)
#define AX_SWRESET_RT			(g1int2uint 0x02)
#define AX_SWRESET_PRTE			(g1int2uint 0x04)
#define AX_SWRESET_PRL			(g1int2uint 0x08)
#define AX_SWRESET_BZ			(g1int2uint 0x10)
#define AX_SWRESET_IPRL			(g1int2uint 0x20)
#define AX_SWRESET_IPPD			(g1int2uint 0x40)

// //////////////////////////////////////////////////

#define MII_BMCR (g1int2uint 0x00)
#define MII_BMSR (g1int2uint 0x01)
#define MII_PHYSID1 (g1int2uint 0x02)
#define MII_PHYSID2 (g1int2uint 0x03)
#define MII_ADVERTISE (g1int2uint 0x04)

#define BMCR_RESET (g1int2uint 0x8000)
#define BMCR_ANRESTART (g1int2uint 0x0200)
#define BMCR_ANENABLE (g1int2uint 0x1000)
#define BMCR_LOOPBACK (g1int2uint 0x4000)

#define ADVERTISE_CSMA (g1int2uint 0x0001)
#define ADVERTISE_10HALF (g1int2uint  0x0020)
#define ADVERTISE_100HALF (g1int2uint 0x0080)
#define ADVERTISE_10FULL (g1int2uint 0x0040)
#define ADVERTISE_100FULL (g1int2uint 0x100)
#define ADVERTISE_ALL (ADVERTISE_10HALF lor ADVERTISE_10FULL lor ADVERTISE_100HALF lor ADVERTISE_100FULL)

#define MEDIUM_PF (g1int2uint   0x0080)
#define MEDIUM_JFE (g1int2uint  0x0040)
#define MEDIUM_TFC (g1int2uint  0x0020)
#define MEDIUM_RFC (g1int2uint  0x0010)
#define MEDIUM_ENCK (g1int2uint 0x0008)
#define MEDIUM_AC (g1int2uint   0x0004)
#define MEDIUM_FD (g1int2uint   0x0002)
#define MEDIUM_GM (g1int2uint   0x0001)
#define MEDIUM_SM (g1int2uint   0x1000)
#define MEDIUM_SBP (g1int2uint  0x0800)
#define MEDIUM_PS (g1int2uint   0x0200)
#define MEDIUM_RE (g1int2uint   0x0100)

#define AX88772_MEDIUM_DEFAULT (MEDIUM_FD lor MEDIUM_RFC lor MEDIUM_TFC lor MEDIUM_PS lor MEDIUM_AC lor MEDIUM_RE )

#define AX88772_IPG0_DEFAULT (g1int2uint 0x15)
#define AX88772_IPG1_DEFAULT (g1int2uint 0x0C)
#define AX88772_IPG2_DEFAULT (g1int2uint 0x12)

// //////////////////////////////////////////////////

extern fun wait_for_ehci_info (): void = "ext#wait_for_ehci_info"
implement wait_for_ehci_info (): void = let
  fun loop (fs: !fixedslot >> _): void = let
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

// //////////////////////////////////////////////////

extern fun dump_usb_dev_desc (&usb_dev_desc_t): void = "mac#dump_usb_dev_desc"
extern fun dump_buf {n,m: nat | n <= m} (& @[uint8][m], int n): void = "mac#dump_buf"
extern fun dump_bufptr {n,m: nat | n <= m} {l: agz} (!((@[uint8][m]) @ l) | ptr l, int n): void = "mac#dump_buf"
extern fun uintarray2uint_le {n: nat | n >= 4} (!(@[uint8][n])): uint = "mac#get4bytes_le"
extern fun uintarrayptr2uint_le {n: nat | n >= 4} {l: agz} (!array_v (uint8, l, n) | ptr l): uint = "mac#get4bytes_le"

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

// //////////////////////////////////////////////////

extern fun make_VendorRequest (i:uint): usb_Request_t = "mac#make_VendorRequest"

extern fun dump_urb (!urb2, int): void = "mac#dump_urb"

fun{a:t@ype} asix_write_cmd {i, n: nat} {l: agz} (
    pf: !(@[a][n]) @ l |
    usbd: !usb_device_vt (i), cmd: uint, wValue: int, wIndex: int, count: int n, data: ptr l
  ): [s: int] status s =
let
  val wLength = $UN.cast{natLt USB_BUF_SIZE}(sizeofGEZ<a>() * count)
  val buf = usb_buf_alloc (wLength)
in if ptrcast buf = 0 then ENOSPACE where { prval _ = usb_buf_release_null buf } else
let
  val _ = usb_buf_copy_from_array (pf | buf, data, count)
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb
            val   _ = usb_buf_release buf }
else let
  val (xfer_v | s) =
    urb_begin_control_write (urb, make_RequestType (HostToDevice, Vendor, Device),
                             make_VendorRequest cmd,
                             wValue, wIndex, wLength, buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    usb_buf_release buf;
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_buf_release buf;
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end end // [asix_write_cmd]

fun{a:t@ype} asix_read_cmd {i, n: nat} {l: agz} (
    pf: !(@[a][n]) @ l |
    usbd: !usb_device_vt (i), cmd: uint, wValue: int, wIndex: int, count: int n, data: ptr l
  ): [s: int] status s =
let
  val wLength = $UN.cast{natLt USB_BUF_SIZE}(sizeofGEZ<a>() * count)
  val buf = usb_buf_alloc wLength
in if ptrcast buf = 0 then ENOSPACE where { prval _ = usb_buf_release_null buf } else
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb
            val   _ = usb_buf_release buf }
else let
  val (xfer_v | s) =
    urb_begin_control_read (urb, make_RequestType (DeviceToHost, Vendor, Device),
                            make_VendorRequest cmd,
                            wValue, wIndex, wLength, buf)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    usb_buf_copy_into_array (pf | data, buf, count);
    usb_buf_release buf;
    usb_device_detach_and_release_urb (usbd, urb)
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_buf_release buf;
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end end // [asix_read_cmd]

fun asix_write_cmd_nodata {i: nat} (usbd: !usb_device_vt (i), cmd: uint, wValue: int, wIndex: int): [s: int] status s =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb }
else let
  val (xfer_v | s) =
    urb_begin_control_nodata (urb, make_RequestType (HostToDevice, Vendor, Device),
                              make_VendorRequest cmd, wValue, wIndex)
in
  if s = OK then begin
    urb_wait_while_active (xfer_v | urb);
    urb_transfer_completed (xfer_v | urb);
    usb_device_detach_and_release_urb (usbd, urb);
  end else begin
    urb_transfer_completed (xfer_v | urb);
    usb_device_detach_and_release_urb_ (usbd, urb);
    s
  end
end end // [asix_write_cmd_nodata]

extern fun msleep (int):void = "mac#msleep"

fun asix_write_gpio {i: nat} (usbd: !usb_device_vt i, value: uint, sleep: int): [s: int] status s =
let val s = asix_write_cmd_nodata (usbd, AX_CMD_WRITE_GPIOS, $UN.cast{int}(value), 0) in
  msleep sleep; s
end // [asix_write_gpio]

fun asix_sw_reset {i: nat} (usbd: !usb_device_vt i, flags: uint): [s: int] status s =
  asix_write_cmd_nodata (usbd, AX_CMD_SW_RESET, $UN.cast{int}(flags), 0)

fun asix_mdio_write {i: nat} (usbd: !usb_device_vt i, phy_id: uint, loc: uint, value: uint): [s: int] status s =
let
  var v = @[uint16][1]($UN.cast{uint16}(value))
  val s = asix_write_cmd_nodata (usbd, AX_CMD_SET_SW_MII, 0, 0)
  //val _ = $extfcall (void, "debuglog", 0, 1, "SET_SW_MII returned %d\n", s)
  val s = asix_write_cmd (view@ v | usbd, AX_CMD_WRITE_MII_REG, $UN.cast{int}(phy_id), $UN.cast{int}(loc), 1, addr@ v)
  val _ = asix_write_cmd_nodata (usbd, AX_CMD_SET_HW_MII, 0, 0)
in
  s
end

fun asix_mdio_read {i: nat} (usbd: !usb_device_vt i, phy_id: uint, loc: uint, value: &uint? >> uint): [s: int] status s =
let
  var v = @[uint16][1]($UN.cast{uint16}(value))
  val _ = asix_write_cmd_nodata (usbd, AX_CMD_SET_SW_MII, 0, 0)
  val s = asix_read_cmd (view@ v | usbd, AX_CMD_READ_MII_REG, $UN.cast{int}(phy_id), $UN.cast{int}(loc), 1, addr@ v)
  val _ = asix_write_cmd_nodata (usbd, AX_CMD_SET_HW_MII, 0, 0)
in
  value := $UN.cast{uint}(array_get_at(v, 0)); s
end

// //////////////////////////////////////////////////

fun asix_reset {i: nat} (dev: !usb_device_vt i): [s: int] status s = let
  val s = asix_write_gpio (dev, AX_GPIO_RSE lor AX_GPIO_GPO_2 lor AX_GPIO_GPO2EN, 5)
  val _ = $extfcall (void, "debuglog", 0, 1, "asix_write_gpio returned %d\n", s)
  val _ = msleep 5
in if s != OK then s else let
  var v = @[uint16][1](word0)
  val s = asix_read_cmd (view@ v | dev, AX_CMD_READ_PHY_ID, 0, 0, 1, addr@ v)
in if s != OK then s else let
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_PHY_ID returned %d (v=%#x)\n", s, array_get_at (v, 0))
  val phy_id0 = $UN.cast{uint}(array_get_at (v, 0))
  val phy_id1 = if (phy_id0 land (g1int2uint 0xe0)) = g1int2uint 0xe0 then phy_id0 >> 8 else phy_id0
in if phy_id1 = (g0int2uint 0xe0) then begin
  $extfcall (void, "debuglog", 0, 1, "no supported phy_id (%#x)\n", phy_id0); EINVALID
end else let
  val phy_id = phy_id1 land g1int2uint 0x1f
  val _ = $extfcall (void, "debuglog", 0, 1, "phy_id=%#x\n", phy_id)
  val embd_phy = if (phy_id land (g1int2uint 0x1f)) = (g1int2uint 0x10) then 1 else 0
  val s = asix_write_cmd_nodata (dev, AX_CMD_SW_PHY_SELECT, embd_phy, 0);
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_SW_PHY_SELECT returned %d\n", s)
in if s != OK then s else let
  val s = asix_sw_reset (dev, AX_SWRESET_IPPD lor AX_SWRESET_PRL)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_SWRESET_IPPD returned %d\n", s)
in if s != OK then s else let
  val _ = msleep(150)
  val s = asix_sw_reset(dev, AX_SWRESET_CLEAR)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_SWRESET_CLEAR returned %d\n", s)
in if s != OK then s else let
  val _ = msleep(150);
  val s = asix_sw_reset(dev, if eq_g0int_int(embd_phy, 0) then AX_SWRESET_IPRL else AX_SWRESET_PRTE)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_SWRESET (IPRL or PRTE) returned %d\n", s)
in if s != OK then s else let
  val _ = msleep(150)
  var rx = @[uint16][1](word0)
  val s = asix_read_cmd (view@ rx | dev, AX_CMD_READ_RX_CTL, 0, 0, 1, addr@ rx)
in if s != OK then s else let
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_RX_CTL returned %d (rx=%#x)\n", s, array_get_at (v, 0))
  val _ = msleep(150)
  val _ = asix_write_cmd_nodata (dev, AX_CMD_WRITE_RX_CTL, 0, 0)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_WRITE_RX_CTL returned %d\n", s)
in if s != OK then s else let
  var mac = @[uint8][6](byte0)
  // Get the MAC address
  val s = asix_read_cmd (view@ mac | dev, AX_CMD_READ_NODE_ID, 0, 0, 6, addr@ mac)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_NODE_ID returned %d (%.2x:%.2x:%.2x:%.2x:%.2x:%.2x)\n", s,
                     array_get_at (mac, 0),
                     array_get_at (mac, 1),
                     array_get_at (mac, 2),
                     array_get_at (mac, 3),
                     array_get_at (mac, 4),
                     array_get_at (mac, 5))
  var phy_reg1: uint
  var phy_reg2: uint
  val s = asix_mdio_read (dev, phy_id, MII_PHYSID1, phy_reg1)
  val s = asix_mdio_read (dev, phy_id, MII_PHYSID2, phy_reg2)
  val _ = $extfcall (void, "debuglog", 0, 1, "MII_PHYSID returned %d (reg=%#x %#x)\n", s, phy_reg1, phy_reg2)

  // reset card again
  val s = asix_sw_reset (dev, AX_SWRESET_PRL)
  val _ = $extfcall (void, "debuglog", 0, 1, "SW_RESET_PRL returned %d\n", s)
  val _ = msleep 150

  val s = asix_sw_reset (dev, AX_SWRESET_IPRL lor AX_SWRESET_PRL)
  val _ = $extfcall (void, "debuglog", 0, 1, "SW_RESET_IPRL|SW_RESET_PRL returned %d\n", s)
  val _ = msleep 150

  val s = asix_mdio_write (dev, phy_id, MII_BMCR, BMCR_RESET)
  val _ = $extfcall (void, "debuglog", 0, 1, "MII_BMCR BMCR_RESET returned %d\n", s)

  val s = asix_mdio_write (dev, phy_id, MII_ADVERTISE, ADVERTISE_ALL lor ADVERTISE_CSMA)
  val _ = $extfcall (void, "debuglog", 0, 1, "MII_ADVERTISE returned %d\n", s)

  var bmcr: uint
  val s = asix_mdio_read (dev, phy_id, MII_BMCR, bmcr)
  val _ = $extfcall (void, "debuglog", 0, 1, "read MII_BMCR returned %d (bmcr=%#x)\n", s, bmcr)
in if (bmcr land BMCR_ANENABLE) = g1int2uint 0 then EINVALID else let
  val _ = bmcr := bmcr lor BMCR_ANRESTART
  val s = asix_mdio_write (dev, phy_id, MII_BMCR, bmcr)
  val _ = $extfcall (void, "debuglog", 0, 1, "write MII_BMCR returned %d (bmcr=%#x)\n", s, bmcr)

  val m:uint = (AX88772_MEDIUM_DEFAULT)
  val s = asix_write_cmd_nodata (dev, AX_CMD_WRITE_MEDIUM_MODE, $UN.cast{int}(m), 0)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_WRITE_MEDIUM_MODE returned %d\n", s)

  val ipg_v:uint = AX88772_IPG0_DEFAULT lor (AX88772_IPG1_DEFAULT << 8)
  val ipg_i:uint = AX88772_IPG2_DEFAULT
  val s = asix_write_cmd_nodata (dev, AX_CMD_WRITE_IPG0, $UN.cast{int}(ipg_v), $UN.cast{int}(ipg_v))
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_WRITE_IPG0 returned %d\n", s)

  val s = asix_write_cmd_nodata (dev, AX_CMD_WRITE_RX_CTL, 0x88, 0) // AB | SO
  val _ = $extfcall (void, "debuglog", 0, 1, "Accepting broadcasts, starting operation (s=%d)\n", s)

  val s = asix_mdio_read (dev, phy_id, MII_BMCR, bmcr)
  val _ = $extfcall (void, "debuglog", 0, 1, "read MII_BMCR returned %d (bmcr=%#x)\n", s, bmcr)

  val s = asix_mdio_read (dev, phy_id, MII_BMSR, bmcr)
  val _ = $extfcall (void, "debuglog", 0, 1, "read MII_BMSR returned %d (bmcr=%#x)\n", s, bmcr)

  var medium = @[uint16][1](word0)
  val s = asix_read_cmd (view@ medium | dev, AX_CMD_READ_MEDIUM_STATUS, 0, 0, 1, addr@ medium)
  val _ = $extfcall (void, "debuglog", 0, 1, "read MEDIUM STATUS returned %d (m=%#x)\n", s, array_get_at(medium, 0))
in
  $extfcall (void, "debuglog", 0, 1, "reset complete\n"); OK
end end end end end end end end end end end // flattened lets

fun asix_write_mac {i: nat} (usbd: !usb_device_vt i): void = let
  var mac = @[uint8][6](byte0)
  val _ = array_set_at (mac, 0, $UN.cast{uint8}(0x01))
  val _ = array_set_at (mac, 1, $UN.cast{uint8}(0x02))
  val _ = array_set_at (mac, 2, $UN.cast{uint8}(0x03))
  val _ = array_set_at (mac, 3, $UN.cast{uint8}(0x04))
  val _ = array_set_at (mac, 4, $UN.cast{uint8}(0x05))
  val _ = array_set_at (mac, 5, $UN.cast{uint8}(0x06))
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_WRITE_NODE_ID \n")
  val s = asix_write_cmd (view@ mac | usbd, AX_CMD_WRITE_NODE_ID, 0, 0, 6, addr@ mac)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_WRITE_NODE_ID %d\n", s)
in
  ()
end

extern fun counter (): int = "mac#counter" //testing

// should match value in uip-conf.h for BUFSIZE
stadef UIP_BUFSIZE = 1500
macdef UIP_BUFSIZE = $extval(uint UIP_BUFSIZE, "UIP_BUFSIZE")

stadef USBNET_BUFSIZE = 1600
macdef USBNET_BUFSIZE = 1600

extern fun copy_into_uip_buf {n, m: nat | n <= m && n <= UIP_BUFSIZE} {l: agz} (
    !usb_mem_vt (l, m), uint n
  ): void = "mac#copy_into_uip_buf"
extern fun copy_from_uip_buf {m: nat} {l: agz} (
    !usb_mem_vt (l, m)
  ): [n: nat | n <= m && n <= UIP_BUFSIZE] uint n = "mac#copy_from_uip_buf"

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
  val txlen = copy_from_uip_buf (tbuf) // will leave 4 bytes for command word
  // txlen does not include command word length
  val header = ((txlen lxor (g1int2uint 0xffff)) << 16) lor txlen
  val _ = usb_buf_set_uint_at (tbuf, 0, header);
  //dump_buf (txbuf, g1uint2int (txlen + g1int2uint 4));
  val (xfer_v | s) = urb_begin_bulk_write (turb, g1uint2int (txlen + g1int2uint 4), tbuf)
  prval _ = (txfer_v := xfer_v)
in
  ()
end

extern fun asix_uip_loop {i: nat} {l:agz} (
    pfmac: !(@[uint8][6]) @ l | usbd: !usb_device_vt i, mac: ptr l
  ): [s: int] status s = "ext#asix_uip_loop"

fun asix_init {i: nat} (usbd: !usb_device_vt i): void = let
  val s = usb_set_configuration (usbd, 1)
  val _ = $extfcall (void, "debuglog", 0, 1, "usb_set_configuration returned %d\n", s)
  var cfgval: uint8
  val s = usb_get_configuration (usbd, cfgval)
  val _ = $extfcall (void, "debuglog", 0, 1, "usb_get_configuration returned %d (cfgval=%d)\n", s, cfgval)

  val _ = asix_reset usbd

  var mac = @[uint8][6](byte0)
  // Get the MAC address
  val s = asix_read_cmd (view@ mac | usbd, AX_CMD_READ_NODE_ID, 0, 0, 6, addr@ mac)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_NODE_ID returned %d (%.2x:%.2x:%.2x:%.2x:%.2x:%.2x)\n", s,
                     array_get_at (mac, 0),
                     array_get_at (mac, 1),
                     array_get_at (mac, 2),
                     array_get_at (mac, 3),
                     array_get_at (mac, 4),
                     array_get_at (mac, 5))



  var v = @[uint16][1](word0)
  val s = asix_read_cmd (view@ v | usbd, AX_CMD_READ_RX_CTL, 0, 0, 1, addr@ v)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_RX_CTL returned %d (v=%#x)\n", s, array_get_at (v, 0))
in
  () where { val _ = asix_uip_loop (view@ mac | usbd, addr@ mac) };
end

fun asix_operate {i: nat} (usbd: !usb_device_vt i): void = begin
  asix_init usbd;
  ()
end


// //////////////////////////////////////////////////

implement atsmain () = let
  val usbd = usb_acquire_device 2
  val () = print_dev_desc usbd
in
  if usb_device_id_vendor usbd = 0xb95 then
    if usb_device_id_product usbd = 0x772a then // AX77882A
      asix_operate usbd
    else if usb_device_id_product usbd = 0x7e2b then // AX77882B
      asix_operate usbd
    else () else ();
  usb_release_device usbd;
  busyloop () where { fun busyloop (): void = busyloop () }; 0
end

// //////////////////////////////////////////////////

extern fun get_uip_len (): int = "mac#get_uip_len"

// //////////////////////////////////////////////////
// UIP interface
abst@ype uip_timer_t = $extype "struct timer"
extern fun timer_set (&uip_timer_t? >> uip_timer_t, int): void = "mac#timer_set"
extern fun timer_reset (&uip_timer_t): void = "mac#timer_reset"
extern fun timer_expired (&uip_timer_t): bool = "mac#timer_expired"

extern fun uip_init (): void = "mac#uip_init"
abst@ype uip_eth_addr_t = $extype "struct uip_eth_addr"
extern fun uip_eth_addr (&uip_eth_addr_t? >> uip_eth_addr_t, int, int, int, int, int, int): void = "mac#uip_eth_addr"
extern fun uip_eth_addr_array {l: agz} (!((@[uint8][6]) @ l) | &uip_eth_addr_t? >> uip_eth_addr_t, ptr l): void = "mac#uip_eth_addr_array"
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

implement asix_uip_loop {i} (pfmac | usbd, mac) = let
  fun uip_preamble (periodic_timer: &uip_timer_t? >> uip_timer_t, arp_timer: &uip_timer_t? >> uip_timer_t): void =
  let
    var ipaddr: uip_ipaddr_t
  in
    timer_set (periodic_timer, CLOCK_SECOND / 2);
    timer_set (arp_timer, CLOCK_SECOND * 10);
    uip_init ();

    uip_ipaddr (ipaddr, 10,0,0,3);
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
      val size = g1ofg0 (header land (g1int2uint 0x7ff))
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
    val s = usb_device_alloc_urb (usbd, 3, 512, turb)
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

  var ethaddr: uip_eth_addr_t
in
  uip_eth_addr_array (pfmac | ethaddr, mac);
  uip_setethaddr ethaddr;
  usb_with_urb (usbd, 2, 512, start_loop)
end

%{$

void appcall(void)
{
}

void uip_log(char *m)
{
  DLOG(1, "uIP log message: %s\n", m);
}

#if 0
static bool
send_cmd (bool input, uint8 cmd, uint16 val, uint16 index,
          uint16 len, uint8 *buf)
{
  USB_DEV_REQ setup_req;

  setup_req.bmRequestType = (input ? 0x80 : 0x00) | 0x40;
  setup_req.bRequest = cmd;
  setup_req.wValue = val;
  setup_req.wIndex = index;
  setup_req.wLength = len;

  {
    uint8 *ptr = (uint8 *) &setup_req;
    DLOG ("send_cmd : %.02X%.02X_%.02X%.02X_%.02X%.02X_%.02X%.02X",
          ptr[0], ptr[1], ptr[2], ptr[3],
          ptr[4], ptr[5], ptr[6], ptr[7]);
  }

  return usb_control_transfer (ethusbdev, &setup_req, sizeof (USB_DEV_REQ),
                               buf, len) == 0;
}


static bool
reset (void)
{
  u32 phy_id, phy_reg1, phy_reg2;
  u16 bmcr, medium;

  /* setup GPIO */
  if (!send_cmd (FALSE, GPIO_WRITE, GPIO_RSE | GPIO_GPO_2 | GPIO_GPO2EN,
                 0, 0, NULL))
    goto abort;
  delay (5);

  /* setup embedded or external PHY */
  if (!send_cmd (TRUE, ETH_PHY_ID, 0, 0, 2, (u8 *)&phy_id))
    goto abort;

  if ((phy_id & 0xE0) == 0xE0) {
    /* lower byte is unsupported PHY */
    phy_id >>= 8;
    if ((phy_id & 0xE0) == 0xE0) {
      DLOG ("no supported PHY");
      goto abort;
    }
  }

  phy_id &= 0x1F;               /* mask ID bits */

  DLOG ("phy_id=0x%x", phy_id);

  if (!send_cmd (FALSE, SW_PHY_SELECT,
                 (phy_id & 0x1F) == 0x10 ? 1 : 0,
                 0, 0, NULL))
    goto abort;

  DLOG ("sending SW reset");
  /* reset card */
  if (!send_cmd (FALSE, SW_RESET, SWRESET_IPPD | SWRESET_PRL, 0, 0, NULL))
    goto abort;
  delay (150);
  if (!send_cmd (FALSE, SW_RESET, SWRESET_CLEAR, 0, 0, NULL))
    goto abort;
  delay (150);
  if (!send_cmd (FALSE, SW_RESET,
                 (phy_id & 0x1F) == 0x10 ? SWRESET_IPRL : SWRESET_PRTE,
                 0, 0, NULL))
    goto abort;
  delay (150);

  /* check RX CTRL */
  DLOG ("RXCTRL=0x%x", read_rx_ctrl ());
  if (!write_rx_ctrl (0x0))
    goto abort;
  DLOG ("wrote 0x0 -- RXCTRL=0x%x", read_rx_ctrl ());

  /* get ethernet address */
  memset (ethaddr, 0, ETH_ADDR_LEN);
  if (!send_cmd (TRUE, GET_NODE_ID, 0, 0, ETH_ADDR_LEN, ethaddr))
    goto abort;

  DLOG ("ethaddr=%.02X:%.02X:%.02X:%.02X:%.02X:%.02X",
        ethaddr[0], ethaddr[1], ethaddr[2], ethaddr[3], ethaddr[4], ethaddr[5]);

  /* get PHY IDENTIFIER (vendor, model) from MII registers */
  phy_reg1 = mdio_read (phy_id, MII_PHYSID1);
  phy_reg2 = mdio_read (phy_id, MII_PHYSID2);

  DLOG ("MII said PHY IDENTIFIER=0x%x",
        ((phy_reg1 & 0xffff) << 16) | (phy_reg2 & 0xffff));


  /* reset card again */
  DLOG ("resending SW reset");
  if (!send_cmd (FALSE, SW_RESET, SWRESET_PRL, 0, 0, NULL))
    goto abort;
  delay (150);
  if (!send_cmd (FALSE, SW_RESET, SWRESET_IPRL | SWRESET_PRL, 0, 0, NULL))
    goto abort;
  delay (150);

  /* init MII */
  mdio_write (phy_id, MII_BMCR, BMCR_RESET);
  mdio_write (phy_id, MII_ADVERTISE, ADVERTISE_ALL | ADVERTISE_CSMA);

  /* autonegotiation */
  bmcr = mdio_read (phy_id, MII_BMCR);
  DLOG ("enabling autonegotiation.  BMCR=0x%x", bmcr);
  if (bmcr & BMCR_ANENABLE) {
    bmcr |= BMCR_ANRESTART;
    mdio_write (phy_id, MII_BMCR, bmcr);
  } else
    goto abort;

  DLOG ("setting medium mode=0x%x", AX88772_MEDIUM_DEFAULT);
  /* setup medium mode */
  if (!send_cmd (FALSE, MEDIUM_MODE, AX88772_MEDIUM_DEFAULT, 0, 0, NULL))
    goto abort;

  /* interpacket gap */
  DLOG ("setting IPG");
  if (!send_cmd (FALSE, WRITE_IPG,
                 AX88772_IPG0_DEFAULT | (AX88772_IPG1_DEFAULT << 8),
                 AX88772_IPG2_DEFAULT, 0, NULL))
    goto abort;

  DLOG ("accepting broadcasts, starting operation");
  /* Accept Broadcasts and Start Operation */
  if (!write_rx_ctrl (0x88))       /* AB | SO */
    goto abort;

  DLOG ("RXCTRL=0x%x at end of reset", read_rx_ctrl ());

  if (!send_cmd (TRUE, MEDIUM_STATUS, 0, 0, 2, (u8 *) &medium))
    goto abort;
  DLOG ("medium status=0x%x at end of reset", medium);

  bmcr = mdio_read (phy_id, MII_BMCR);
  DLOG ("BMCR=0x%x at end of reset", bmcr);

  DLOG ("BMSR=0x%x at end of reset", mdio_read (phy_id, MII_BMSR));

  return TRUE;
 abort:
  DLOG ("reset failed");
  return FALSE;
}
#endif

int main(void)
{
  svc3("ASIX\n", 6, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);

  DLOG(1, "asix ****************** wait_for_ehci_info\n");
  wait_for_ehci_info2 ();

  //while(*((volatile u32 *) (_ipcmappings[1].address)) == 0)   ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  //while(__atomic_load_n((u32 *) _ipcmappings[1].address, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  //usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  //DLOG(1, "asix ****************** &usbdevices[0]=%#x\n", &usbd[0]);
  //while(*((volatile u32 *) (&usbd[1].flags)) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[2].flags));
  //while(__atomic_load_n(&usbd[2].flags, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[2].flags));
  //DLOG(1, "asix ****************** usbd->qh.capabilities=%#x\n", QH(usbd[2],0).capabilities);
  DLOG(1, "asix ****************** invoking atsmain\n");
  //DLOG(1, "asix ****************** &__ehci_td_pool_array=%#x &__ehci_td_pool_bitmap=%#x\n", get_ehci_td_pool_addr (), get_ehci_td_bitmap_addr ());
  return atsmain();
}

%}

////

  idVendor           0x0b95 ASIX Electronics Corp.
  idProduct          0x772a AX88772A Fast Ethernet
  bcdDevice            0.01
  iManufacturer           1
  iProduct                2
  iSerial                 3
  bNumConfigurations      1
  Configuration Descriptor:
    bLength                 9
    bDescriptorType         2
    wTotalLength           39
    bNumInterfaces          1
    bConfigurationValue     1
    iConfiguration          4
    bmAttributes         0xe0
      Self Powered
      Remote Wakeup
    MaxPower              250mA
    Interface Descriptor:
      bLength                 9
      bDescriptorType         4
      bInterfaceNumber        0
      bAlternateSetting       0
      bNumEndpoints           3
      bInterfaceClass       255 Vendor Specific Class
      bInterfaceSubClass    255 Vendor Specific Subclass
      bInterfaceProtocol      0
      iInterface              7
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x81  EP 1 IN
        bmAttributes            3
          Transfer Type            Interrupt
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0008  1x 8 bytes
        bInterval              11
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x82  EP 2 IN
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
      Endpoint Descriptor:
        bLength                 7
        bDescriptorType         5
        bEndpointAddress     0x03  EP 3 OUT
        bmAttributes            2
          Transfer Type            Bulk
          Synch Type               None
          Usage Type               Data
        wMaxPacketSize     0x0200  1x 512 bytes
        bInterval               0
