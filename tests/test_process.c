/*
 * Testing Processes
 *
 * -------------------------------------------------------------------
 *
 * Copyright (C) 2011 Matthew Danish.
 *
 * All rights reserved. Please see enclosed LICENSE file for details.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * - Neither the name of the author nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef TEST_PROCESS

#include "types.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "sched/process.h"
#include "sched/sched.h"
#include "sched/elf.h"
#define MODULE "test_process"
#include "debug/log.h"

extern u32 _program_map_start, _program_map_count;

status test_process(void)
{
  extern process_t *current;
  void *entry;
  process_t *p, *q;
  u32 *progs = (u32 *) &_program_map_start, cnt = (u32) &_program_map_count;

  if(cnt < 2) {
    DLOG(1, "test_process: requires > 2 programs\n");
    return EINVALID;
  }

  DLOG(1, "test_process: loading program found at %#x\n", progs[0]);
  if(program_load((void *) progs[0], &p) != OK) {
    DLOG(1, "program_load failed\n");
    return EINVALID;
  }

  DLOG(1, "test_process: loading program found at %#x\n", progs[1]);
  if(program_load((void *) progs[1], &q) != OK) {
    DLOG(1, "program_load failed\n");
    return EINVALID;
  }

  sched_wakeup(q);
  process_switch_to(p);

  current = p;
  entry = p->entry;
  DLOG(1, "%#x: %#x %#x %#x %#x\n", entry,
       ((u32 *) entry)[0], ((u32 *) entry)[1], ((u32 *) entry)[2], ((u32 *) entry)[3]);

  DLOG(1, "Switching to usermode.\n");

  sched_launch_first_process(p);

  return OK;
}

#endif

/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
