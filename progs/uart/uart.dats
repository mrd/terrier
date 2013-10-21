// uart: serial port module

// (currently testing IPC mechanisms)

#define ATS_DYNLOADFLAG 0

//staload _ = "prelude/DATS/basics.dats"
staload _ = "prelude/DATS/integer.dats"

staload "uart.sats"
staload "ipcmem.sats"
staload "fourslot2w.sats"
staload _ = "fourslot.dats"
staload _ = "fourslot2w.dats"

extern fun atsmain (): int = "ext#atsmain"
extern fun mydelay (): void = "mac#mydelay"

typedef buf_t = $extype "buf_t"
extern fun printbuf (_: buf_t): void = "mac#printbuf"

typedef uart_ipc_t = @(int,buf_t)

implement atsmain () = let
  fun do_nothing (): void = do_nothing ()
  fun loop {l: addr} {n: nat} (
        pf_uart: !fourslot2w_v (l, n, int, uart_ipc_t, B) |
        uart: ptr l,
        counter: int
      ): void = let
    val @(stamp, v) = fourslot2w_readB<int,uart_ipc_t> (pf_uart | uart)
    val counter' = (if stamp = counter
                     then counter + 1 where {
                       val _ = fourslot2w_writeB<int,uart_ipc_t> (pf_uart | uart, counter + 1)
                       val _ = printbuf v
                     }
                     else counter): int
  in
    loop (pf_uart | uart, counter')
  end // end [loop]

  var pages: int?
  val (opt_pf | uart) = ipcmem_get_view (0, pages)
in
  if uart > 0 then
    let
      prval Some_v pf_ipcmem = opt_pf
      val s = fourslot2w_init<int,uart_ipc_t> (pf_ipcmem | uart, pages, B)
    in
      if s = 0 then 0 where {
        prval Right_v pf_uart = pf_ipcmem

        (* ... *)

        val counter0 = 1
        val _ = fourslot2w_writeB<int,uart_ipc_t> (pf_uart | uart, counter0)
        val _ = loop (pf_uart | uart, counter0)

        prval pf_ipcmem = fourslot2w_ipc_free pf_uart
        prval _ = ipcmem_put_view pf_ipcmem


      } else 1 where {
        // failed to initialize
        prval Left_v pf_ipcmem = pf_ipcmem
        prval _ = ipcmem_put_view pf_ipcmem
        val _ = do_nothing ()
      }

    end else 1 where {
    // failed to acquire ipcmem
    prval None_v () = opt_pf
    val _ = do_nothing ()
  }
end // end [atsmain]


%{$

int main(void)
{
  svc3("UART\n", 6, 0);
  if(_ipcmappings[0].address == 0)
    svc3("0NULL\n", 7, 0);

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
