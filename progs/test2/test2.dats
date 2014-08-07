// testing module

// (currently testing IPC mechanisms)

#define ATS_DYNLOADFLAG 0

//staload _ = "prelude/DATS/basics.dats"
staload _ = "prelude/DATS/integer.dats"

staload "test2.sats"
staload "ipcmem.sats"
staload "multislot.sats"
staload _ = "multislot.dats"

extern fun atsmain (): int = "ext#atsmain"
extern fun mydelay (): void = "mac#mydelay"

extern fun printnum (_: int): void = "mac#printnum"

typedef ehci_info_t = int

implement atsmain () = let
  fun do_nothing (): void = do_nothing ()
  fun loop {l: addr} {n, i: nat} (
        pf_ms: !mslot_v (l, n, ehci_info_t, false, i) |
        ms: ptr l, i: int i
      ): void = let
    val x = multislot_read (pf_ms | ms, i)
  in
    if x > 40 then printnum x else loop (pf_ms | ms, i)
  end // end [loop]

  var pages: int?
  val (opt_pf | ms) = ipcmem_get_view (0, pages)
in
  if ms > 0 then
    let
      prval Some_v pf_ipcmem = opt_pf
      var i: int?
      val (e_pf | s) = multislot_initialize_reader (pf_ipcmem | ms, pages, i)
    in
      if s = 0 then 0 where {
        prval Right_v pf_ms = e_pf

        (* ... *)

        val _ = loop (pf_ms | ms, i)
        val _ = do_nothing ()

        prval pf_ipcmem = multislot_release pf_ms
        prval _ = ipcmem_put_view pf_ipcmem

      } else 1 where {
        // failed to initialize
        prval Left_v pf_ipcmem = e_pf
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
  svc3("TEST2\n", 6, 0);
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
