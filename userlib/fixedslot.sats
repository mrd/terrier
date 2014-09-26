#define ATS_STALOADFLAG 0 // no run-time staloading

%{#

#include "fixedslot.cats"

%}

staload "ipcmem.sats"

absvtype fixedslot_vt
vtypedef fixedslot = fixedslot_vt

fun{a:t@ype} fixedslot_read (!fixedslot): a
fun{a:t@ype} fixedslot_write (!fixedslot, a): void
fun fixedslot_initialize_reader (!fixedslot): void
fun fixedslot_initialize_writer (!fixedslot): void
