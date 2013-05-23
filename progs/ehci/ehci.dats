#define ATS_DYNLOADFLAG 0
staload "ehci.sats"

extern fun hsusbhc_init(): void = "mac#hsusbhc_init"

extern fun atsmain (): int = "ext#atsmain"

implement atsmain () = let fun loop () = loop () in hsusbhc_init (); loop (); 0 end

%{

int main(void)
{
  return atsmain();
}

%}
