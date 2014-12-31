
typedef status (s:int) = int s  (* helper *)

macdef OK = $extval(status 0, "OK")
macdef EINVALID = $extval(status 1, "EINVALID")
macdef EUNDEFINED = $extval(status 2, "EUNDEFINED")
macdef ENOSPACE = $extval(status 3, "ENOSPACE")
macdef EINCOMPLETE = $extval(status 4, "EINCOMPLETE")
macdef EDATA = $extval(status 5, "EDATA")
macdef ETIMEOUT = $extval(status 6, "ETIMEOUT")
