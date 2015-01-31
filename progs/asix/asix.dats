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

extern fun dump_usb_dev_desc (&usb_dev_desc_t): void = "mac#dump_usb_dev_desc"

fun print_dev_desc {i: nat} (usbd: !usb_device_vt (i)): void = () where { val _ = usb_with_urb (usbd, 0, 0, f) }
where {
  var f = lam@ (usbd: !usb_device_vt i, urb: !urb_vt (i, 0, false)): [s: int] status s =<clo1> let
    var devdesc = @[usb_dev_desc_t][1](usb_dev_desc_empty)
    val (xfer_v | s) =
      urb_begin_control_read (view@ devdesc |
                              urb, make_RequestType (DeviceToHost, Standard, Device), make_Request GetDescriptor,
                              USB_TYPE_DEV_DESC << 8, 0,
                              1, addr@ devdesc)
  in
    if s = OK then begin
      urb_wait_while_active (xfer_v | urb);
      urb_transfer_completed (xfer_v | urb);
      dump_usb_dev_desc dd where { var dd = array_get_at (devdesc, 0) };
      urb_detach_and_free urb
    end else begin
      urb_transfer_completed (xfer_v | urb);
      urb_detach_and_free_ urb; s
    end
  end // [print_dev_desc]
}

// //////////////////////////////////////////////////

extern fun make_VendorRequest (i:uint): usb_Request_t = "mac#make_VendorRequest"

fun{a:t@ype} asix_write_cmd {i, n: nat} {l: agz} (
    pf: !(@[a][n]) @ l |
    usbd: !usb_device_vt (i), cmd: uint, wValue: int, wIndex: int, count: int n, data: ptr l
  ): [s: int] status s =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb }
else let
  val (xfer_v | s) =
    urb_begin_control_write (pf | urb, make_RequestType (HostToDevice, Vendor, Device),
                             make_VendorRequest cmd,
                             wValue, wIndex, count, data)
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
end end // [asix_write_cmd]

fun{a:t@ype} asix_read_cmd {i, n: nat} {l: agz} (
    pf: !(@[a][n]) @ l |
    usbd: !usb_device_vt (i), cmd: uint, wValue: int, wIndex: int, count: int n, data: ptr l
  ): [s: int] status s =
let
  var urb: urb0?
  val s = usb_device_alloc_urb (usbd, 0, 0, urb)
in if s != OK then
  s where { prval _ = usb_device_release_null_urb urb }
else let
  val (xfer_v | s) =
    urb_begin_control_write (pf | urb, make_RequestType (DeviceToHost, Vendor, Device),
                             make_VendorRequest cmd,
                             wValue, wIndex, count, data)
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
end end // [asix_read_cmd]

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

// //////////////////////////////////////////////////

fun asix_reset {i: nat} (dev: !usb_device_vt i): [s: int] status s = let
  val s = asix_sw_reset (dev, g1int2uint 0) // blink LEDs
  val s = asix_write_gpio (dev, AX_GPIO_RSE lor AX_GPIO_GPO_2 lor AX_GPIO_GPO2EN, 5)
  val _ = $extfcall (void, "debuglog", 0, 1, "asix_write_gpio returned %d\n", s)
in if s != OK then s else let
  var v = @[uint8][2](byte0)
  val s = asix_read_cmd (view@ v | dev, AX_CMD_READ_PHY_ID, 0, 0, 2, addr@ v)
in if s != OK then s else let
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_CMD_READ_PHY_ID returned %d (v=%d)\n", s, array_get_at (v, 1))
  val embd_phy = 0
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
  val s = asix_sw_reset(dev, if embd_phy = 0 then AX_SWRESET_IPRL else AX_SWRESET_PRTE)
  val _ = $extfcall (void, "debuglog", 0, 1, "AX_SWRESET (IPRL or PRTE) returned %d\n", s)
in if s != OK then s else let
  val _ = msleep(150)

in
  OK
end end end end end end end // flattened lets

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

fun asix_init {i: nat} (usbd: !usb_device_vt i): void = let
  val s = usb_set_configuration (usbd, 1)
  val _ = $extfcall (void, "debuglog", 0, 1, "usb_set_configuration returned %d\n", s)
  var cfgval: uint8
  val s = usb_get_configuration (usbd, cfgval)
  val _ = $extfcall (void, "debuglog", 0, 1, "usb_get_configuration returned %d (cfgval=%d)\n", s, cfgval)

  val _ = asix_write_mac usbd
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

  val _ = asix_reset usbd

  var v = @[uint16][1](word0)
  val s = asix_read_cmd (view@ v | usbd, AX_CMD_READ_RX_CTL, 0, 0, 1, addr@ v)
  val _ = $extfcall (void, "debuglog", 0, 1, "asix_read_cmd returned %d (v=%#x)\n", s, array_get_at (v, 0))
in
  ()
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
    if usb_device_id_product usbd = 0x772a then
      asix_operate usbd
    else () else ();
  usb_release_device usbd;
  busyloop () where { fun busyloop (): void = busyloop () }; 0
end

// //////////////////////////////////////////////////


%{$

int main(void)
{
  svc3("ASIX\n", 9, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);


  //while(*((volatile u32 *) (_ipcmappings[1].address)) == 0)   ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  while(__atomic_load_n((u32 *) _ipcmappings[1].address, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"((_ipcmappings[1].address) ));
  usb_device_t *usbd = (usb_device_t *) _ipcmappings[1].address;
  DLOG(1, "asix ****************** &usbdevices[0]=%#x\n", &usbd[0]);
  //while(*((volatile u32 *) (&usbd[1].flags)) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[2].flags));
  while(__atomic_load_n(&usbd[2].flags, __ATOMIC_SEQ_CST) == 0) ASM("MCR p15, #0, %0, c7, c14, #1"::"r"(&usbd[2].flags));
  DLOG(1, "asix ****************** usbd->qh.capabilities=%#x\n", QH(usbd[2],0).capabilities);
  DLOG(1, "asix ****************** invoking atsmain\n");
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
