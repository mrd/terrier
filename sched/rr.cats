#include "ats_types.h"
#include "ats_basics.h"
#include "types.h"
#include "mem/virtual.h"
#include "mem/physical.h"
#include "arm/memory.h"
#include "arm/asm.h"
#include "arm/smp/spinlock.h"
#include "arm/smp/per-cpu.h"
#include "omap/timer.h"
#include "sched/process.h"
#include "intr/interrupts.h"
#define MODULE "sched_rr"
#include "debug/log.h"

void *atspre_null_ptr = 0;      //FIXME
ats_bool_type atspre_peq(ats_ptr_type a, ats_ptr_type b)
{
  return a == b;
}

static pid_t runq_head;         /* global runqueue: can only hold cleanly saved processes */
static DEFSPINLOCK(rrlock);

#define get_runq_head() ((void *) &runq_head)
#define rel_runq_head(p)
#define get_cpuq_head(v) ((void *) (&cpu_read(pid_t, cpu_runq_head)))
#define rel_cpuq_head(p)
#define rrlock_lock() spinlock_lock(&rrlock)
#define rrlock_unlock() spinlock_unlock(&rrlock)
static status waitqueue_append(pid_t *q, process_t *p);
#define pqueue_is_empty(q) (*((pid_t *) q) == NOPID)
