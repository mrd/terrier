(*
** This is mostly for staloading
** template code in ATSLIB/prelude
*)

(* ****** ****** *)
//
// Author: Hongwei Xi
// AuthorEmail: gmhwxiATgmailCOM
//
(* ****** ****** *)

#ifndef SHARE_ATSPRE_STALOAD
#define SHARE_ATSPRE_STALOAD 1

(* ****** ****** *)
//
#define
PATSPRE_targetloc "$PATSHOME/prelude"
//
(* ****** ****** *)
//
staload _ = "{$PATSPRE}/DATS/basics.dats"
//
(* ****** ****** *)
//
staload _ = "{$PATSPRE}/DATS/pointer.dats"
//
(* ****** ****** *)
//
staload _ = "{$PATSPRE}/DATS/integer.dats"
staload _ = "{$PATSPRE}/DATS/integer_fixed.dats"
//

#endif // end of [#ifndef SHARE_ATSPRE_STALOAD]

(* ****** ****** *)

(* end of [atspre_staload.hats] *)
