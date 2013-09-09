// ehci: host-controller module

// (currently testing IPC as well)

#define ATS_DYNLOADFLAG 0
staload "ehci.sats"
staload "ipcmem.sats"
staload "fourslot2w.sats"
staload _ = "fourslot2w.dats"

extern fun hsusbhc_init(): void = "mac#hsusbhc_init"

extern fun atsmain (): int = "ext#atsmain"

typedef buf_t = $extype "buf_t"
extern fun buf_init {n: nat | n < 123} (b: &buf_t? >> buf_t, x: string n): void = "mac#buf_init"

typedef uart_ipc_t = @(int,buf_t)

absviewt@ype uart_token_vt = int
extern fun get_uart_token_vt (): uart_token_vt = "mac#get_uart_token_vt"
extern fun put_uart_token_vt (_: uart_token_vt): void = "mac#put_uart_token_vt"
extern castfn int_of_uart_token_vt (_: uart_token_vt): int
extern castfn uart_token_vt_of_int (_: int): uart_token_vt

// write a number to the UART process via IPC
fun write_string_uart {l: addr} {len, n: nat | len < 123} (
        pf_uart: !fourslot2w_v (l, n, int, uart_ipc_t, A) |
        utoken: &uart_token_vt,
        uart: ptr l,
        s: string len
      ): void = let

  var ctrbuf : @(int, buf_t)
  val _ = buf_init (ctrbuf.1, s)

  fun loop {l:addr} {n:nat} (
        pf_uart: !fourslot2w_v (l, n, int, uart_ipc_t, A) |
        uart: ptr l,
        oldcounter: int
      ): int = let

    val counter = fourslot2w_readA (pf_uart | uart)
  in
    if counter = oldcounter then
      loop (pf_uart | uart, oldcounter)
    else counter
  end // end [loop]

  val counter = int_of_uart_token_vt utoken

  // loop until counter increments up from old value
  val counter' = loop (pf_uart | uart, counter)
in
  ctrbuf.0 := counter';
  // write data to UART
  fourslot2w_writeA (pf_uart | uart, ctrbuf);
  // save new token
  utoken := uart_token_vt_of_int counter'
end // end [write_num_uart]

implement atsmain () = let
  fun do_nothing ():void = do_nothing ()

  var pages: int?
  val (opt_pf | uart) = ipcmem_get_view (0, pages)
in
  if uart > null then
    let
      prval Some_v pf_ipcmem = opt_pf
      val s = fourslot2w_init<int,uart_ipc_t> (pf_ipcmem | uart, pages, A)
    in
      if s = 0 then 0 where {
        prval Right_v pf_uart = pf_ipcmem

        (* ... *)
        var utoken = get_uart_token_vt ()
        val _ = write_string_uart (pf_uart | utoken, uart, "a")
        val _ = write_string_uart (pf_uart | utoken, uart, "b")


        val _ = hsusbhc_init ()
        val _ = write_string_uart (pf_uart | utoken, uart, "c")

        val _ = put_uart_token_vt utoken
        val _ = do_nothing ()


        prval pf_ipcmem = fourslot2w_ipc_free pf_uart
        prval _         = ipcmem_put_view pf_ipcmem

      } else 1 where {
        // failed to initialize
        prval Left_v pf_ipcmem = pf_ipcmem
        prval _                = ipcmem_put_view pf_ipcmem
        val _                  = do_nothing ()
      }

    end else 1 where {
    // failed to acquire ipcmem
    prval None_v () = opt_pf
    val _           = do_nothing ()
  }
end // [atsmain]


%{

int main(void)
{
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
