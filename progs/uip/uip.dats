// uIP process

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
staload "uip.sats"
#include "atspre_staload.hats"


// implication
infix ==>
stadef ==> (p: bool, q: bool) = (~p || q)

// iff
infix <==>
stadef <==> (p: bool, q: bool) = (~p || q) && (p || ~q)

macdef byte0 = $extval(uint8, "0")
macdef word0 = $extval(uint16, "0")

fun{a:vt0p} sizeofGEZ (): [s: nat] int s = $UN.cast{intGte(0)}(g1ofg0 (sizeof<a>))

// should match value in uip-conf.h for BUFSIZE
stadef UIP_BUFSIZE = 1500
macdef UIP_BUFSIZE = $extval(uint UIP_BUFSIZE, "UIP_BUFSIZE")

stadef USBNET_BUFSIZE = 1600
macdef USBNET_BUFSIZE = 1600

stadef USBNET_BUF_NUM_UINTS = 400
macdef USBNET_BUF_NUM_UINTS = 400

typedef buf_t = (@[uint][USBNET_BUF_NUM_UINTS])

// //////////////////////////////////////////////////

extern fun atsmain (): int = "ext#atsmain"
extern fun uip_loop {i: nat} {l:agz} (
    pfmac: !(@[uint8][6]) @ l | usbd: !usb_device_vt i, mac: ptr l
  ): [s: int] status s = "ext#uip_loop"
extern fun copy_into_uip_buf {n: nat | n <= UIP_BUFSIZE} (
    & buf_t, uint n
  ): void = "mac#copy_into_uip_buf"
extern fun copy_from_uip_buf {m: nat} {l: agz} (
    & buf_t
  ): [n: nat | n <= m && n <= UIP_BUFSIZE] uint n = "mac#copy_from_uip_buf"
extern fun do_one_send (
    ! fixedslot >> _
  ): void = "ext#do_one_send"

// //////////////////////////////////////////////////

implement atsmain () = 0

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
extern fun get_uip_len (): int = "mac#get_uip_len"

macdef UIP_ETHTYPE_IP = $extval(uint, "UIP_ETHTYPE_IP")
macdef UIP_ETHTYPE_ARP = $extval(uint, "UIP_ETHTYPE_ARP")

extern fun htons (uint): uint = "mac#htons" // just use uint for simplicity, should be ok

macdef CLOCK_SECOND = $extval(int, "CLOCK_SECOND")
macdef UIP_CONNS = $extval(int, "UIP_CONNS")
extern fun clock_time (): int = "mac#clock_time"

extern fun get_itag (! fixedslot >> _): uint
extern fun set_itag (! fixedslot >> _, uint): void

implement uip_loop {i} (pfmac | usbd, mac) = let
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


  fun uip_periodic_check (
      ofs: ! fixedslot >> _,
      periodic_timer: &uip_timer_t,
      arp_timer: &uip_timer_t
    ): void =
    if timer_expired periodic_timer then
      let
        fun loop1 (
            ofs: ! fixedslot >> _, j: int
          ): void =
          if j >= UIP_CONNS then () else begin
            uip_periodic j;
            if get_uip_len () > 0 then begin
              uip_arp_out ();
              do_one_send (ofs)
            end else ();
            loop1 (ofs, j + 1)
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
        loop1 (ofs, 0);
        //loop2 (usbd, 0);
        if timer_expired arp_timer then begin
          timer_reset arp_timer;
          uip_arp_timer ();
        end else ()
      end
    else ()

  fun loop_body (
        ofs: ! fixedslot >> _,
        rbuf: & buf_t
      ): void =
    let
      val size = g1ofg0 (array_get_at (rbuf, 2))
    in
      if size > UIP_BUFSIZE
      then ()
      else let
        val _ = copy_into_uip_buf (rbuf, size);
      in
        // process current read
        if get_uip_buftype () = htons (UIP_ETHTYPE_IP) then begin
          uip_arp_ipin ();
          uip_input ();
          if get_uip_len () > 0 then begin
            uip_arp_out ();
            do_one_send (ofs)
          end else ()
        end else if get_uip_buftype () = htons (UIP_ETHTYPE_ARP) then begin
          uip_arp_arpin ();
          if get_uip_len () > 0 then
            do_one_send (ofs)
          else ()
        end;
        ()
      end
    end

  // ATS helped sort out the resource alloc & clean-up for these functions
  fun loop (
        ifs: ! fixedslot >> _,
        ofs: ! fixedslot >> _,
        itag: uint,
        otag: uint,
        periodic_timer: & uip_timer_t,
        arp_timer: & uip_timer_t,
        cyc_prev: int
      ): #[s: int | s != 0] status s =
  let
    var rbuf = @[uint][USBNET_BUF_NUM_UINTS](g0int2uint 0)
    val itag' = get_itag ifs
  in
    if itag = itag' then begin // No Rx
      uip_periodic_check (ofs, periodic_timer, arp_timer);
      loop (ifs, ofs, itag, otag, periodic_timer, arp_timer, cyc_prev)
    end else // Rx one packet
      let
        val lbody_start = clock_time ()
        val cyc_start = $extfcall (int, "arm_read_cycle_counter", ())
        val _ = fixedslot_readptr (view@ rbuf | ifs, addr@ rbuf, sizeof<buf_t>)
        val _ = set_itag (ofs, itag')
        val _ = loop_body (ofs, rbuf)
        val lbody_end = clock_time ()
        val cyc_end = $extfcall (int, "arm_read_cycle_counter", ())
        val lbody_dur = lbody_end - lbody_start
        val cyc_dur = cyc_end - cyc_start
        //val _ = $extfcall (void, "debuglog", 0, 1, "lbody_dur=%d cyc_dur=%d cyc_loop=%d\n", lbody_dur, cyc_dur, cyc_end - cyc_prev)
        val cyc_next = $extfcall (int, "arm_read_cycle_counter", ())
      in
        loop (ifs, ofs, itag', otag, periodic_timer, arp_timer, cyc_prev)
      end
  end

  fun start_loop (): void = let
    var ipages: int?
    val (opt_pf | iipc) = ipcmem_get_view (0, ipages)
  in if iipc = 0 then () where { prval None_v () = opt_pf } else let
    prval Some_v pf_ipcmem = opt_pf
    val ifs = fixedslot_initialize_reader (pf_ipcmem | iipc, ipages)
    var opages: int?
    val (opt_pf | oipc) = ipcmem_get_view (1, opages)
  in if oipc = 0 then () where { prval None_v () = opt_pf
                                 val (pf | _) = fixedslot_free ifs
                                 prval _ = ipcmem_put_view pf } else let
    prval Some_v pf_ipcmem = opt_pf
    val ofs = fixedslot_initialize_writer (pf_ipcmem | oipc, opages)

    var periodic_timer: uip_timer_t
    var arp_timer: uip_timer_t
    val _ = uip_preamble (periodic_timer, arp_timer)

    val _ = loop (ifs, ofs, g0int2uint 0, g0int2uint 0, periodic_timer, arp_timer, $extfcall (int, "arm_read_cycle_counter", ()))

    val (pf | _) = fixedslot_free ifs
    prval _ = ipcmem_put_view pf

    val (pf | _) = fixedslot_free ofs
    prval _ = ipcmem_put_view pf

  in
    ()
  end end end

(*    
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
*)
  var ethaddr: uip_eth_addr_t
in
  uip_eth_addr_array (pfmac | ethaddr, mac);
  uip_setethaddr ethaddr;
  OK
end


%{$

void appcall(void)
{
}

void uip_log(char *m)
{
  DLOG(1, "uIP log message: %s\n", m);
}

int main(void)
{
  svc3("UIP\n", 6, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);

  DLOG(1, "uip ****************** invoking atsmain\n");
  return atsmain();
}

%}
