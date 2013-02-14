/*
 * Interrupt handling (C portion)
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

#include "types.h"
#include "omap/early_uart3.h"
#include "intr/interrupts.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "arm/smp/per-cpu.h"
#include "mem/virtual.h"
#include "sched/process.h"
#define MODULE "intr"
#include "debug/log.h"
#include "debug/cassert.h"

#ifdef OMAP4460
#define GIC                     /* Generic Interrupt Controller */
#define GICv1                   /* v1 */
#endif

#ifdef OMAP3530
#define MPU_INTC_BASE_PHYS_ADDR 0x48200000
#define MPU_INTC_BASE_ADDR 0x48200000
#ifdef USE_VMM
region_t mpu_intc_region = {
  MPU_INTC_BASE_PHYS_ADDR,
  (void *) MPU_INTC_BASE_ADDR,
  &l1pt, 1, 20, 0, 0, R_PM
};
#endif
#endif
#ifdef OMAP4460
#define MPU_INTC_BASE_PHYS_ADDR 0x48200000
#define MPU_INTC_BASE_ADDR 0x48200000
#ifdef USE_VMM
region_t mpu_intc_region = {
  MPU_INTC_BASE_PHYS_ADDR,
  (void *) MPU_INTC_BASE_ADDR,
  &l1pt, 1, 20, 0, 0, R_PM
};
#endif
#endif

#ifdef OMAP3530
PACKED_STRUCT(intcps) {
  PACKED_FIELD(u32, revision);
  u8 _rsvd1[12];
  PACKED_FIELD(u32, sysconfig);
  PACKED_FIELD(u32, sysstatus);
  u8 _rsvd2[40];
  PACKED_STRUCT(sir_irq) {
    PACKED_FIELD(u32, activeirq:7);
    PACKED_FIELD(u32, spuriousirqflag:25);
  } PACKED_END sir_irq;
  PACKED_STRUCT(sir_fiq) {
    PACKED_FIELD(u32, activefiq:7);
    PACKED_FIELD(u32, spuriousfiqflag:25);
  } PACKED_END sir_fiq;
  PACKED_FIELD(u32, control);
#define INTCPS_CONTROL_NEWIRQAGR BIT(0)
#define INTCPS_CONTROL_NEWFIQAGR BIT(1)
  PACKED_FIELD(u32, protection);
  PACKED_FIELD(u32, idle);
  u8 _rsvd3[12];
  PACKED_FIELD(u32, irq_priority);
  PACKED_FIELD(u32, fiq_priority);
  PACKED_FIELD(u32, threshold);
  u8 _rsvd4[20];
  PACKED_STRUCT(intcps_n) {
    u32 itr;
    u32 mir;
    u32 mir_clear;
    u32 mir_set;
    u32 isr_set;
    u32 isr_clear;
    u32 pending_irq;
    u32 pending_fiq;
  } PACKED_END n[3];
  u8 _rsvd5[32];
  PACKED_STRUCT(ilr) {
    PACKED_FIELD(u32, fiqnirq:1);
    PACKED_FIELD(u32, _rsvd1:1);
    PACKED_FIELD(u32, priority:6);
    PACKED_FIELD(u32, _rsvd2:24);
  } PACKED_END ilr[96];
} PACKED_END;
CASSERT(offsetof(struct intcps, threshold) == 0x68, interrupts);
CASSERT(offsetof(struct intcps, n[2].pending_fiq) == 0xdc, interrupts);
CASSERT(offsetof(struct intcps, ilr) == 0x100, interrupts);

static volatile struct intcps *intc = (struct intcps *) MPU_INTC_BASE_ADDR;
#endif

#ifdef GIC
#define SPURIOUS_ID 1023        /* spurious interrupt ID number */
#define NUM_INTS 1020           /* number of actual interrupt IDs */
volatile u32* PERIPHBASE = NULL;
PACKED_STRUCT(distributor) {
  PACKED_FIELD(u32, DCR);    /* distributor control */
  PACKED_FIELD(u32, ICTR);   /* interrupt controller type */
  PACKED_FIELD(u32, IIDR);   /* implementer identification */
  PACKED_FIELD(u32, _rsvd1[29]);
  PACKED_FIELD(u32, ISR[32]); /* interrupt security */
  PACKED_FIELD(u32, ISER[32]);  /* interrupt set-enable */
  PACKED_FIELD(u32, ICER[32]);  /* interrupt clear-enable */
  PACKED_FIELD(u32, ISPR[32]);  /* interrupt set-pending */
  PACKED_FIELD(u32, ICPR[32]);  /* interrupt clear-pending */
  PACKED_FIELD(u32, ABR[32]);   /* active bit registers */
  PACKED_FIELD(u32, _rsvd2[32]);
  PACKED_FIELD(u32, IPR[256]);  /* interrupt priority registers */
  PACKED_FIELD(u32, IPTR[256]); /* interrupt processor targets registers */
  PACKED_FIELD(u32, ICFR[64]);  /* interrupt configuration registers */
  PACKED_FIELD(u32, impl_defined[64]); /* implementation defined registers */
  PACKED_FIELD(u32, _rsvd3[64]);
  PACKED_FIELD(u32, SGIR);      /* software generated interrupt register */
  PACKED_FIELD(u32, _rsvd4[51]);
  PACKED_FIELD(u32, identification[12]); /* identification registers */
} PACKED_END;
CASSERT(offsetof(struct distributor, ISR) == 0x80, interrupts);
CASSERT(offsetof(struct distributor, ISER) == 0x100, interrupts);
CASSERT(offsetof(struct distributor, ICER) == 0x180, interrupts);
CASSERT(offsetof(struct distributor, ISPR) == 0x200, interrupts);
CASSERT(offsetof(struct distributor, ICPR) == 0x280, interrupts);
CASSERT(offsetof(struct distributor, ABR) == 0x300, interrupts);
CASSERT(offsetof(struct distributor, IPTR) == 0x800, interrupts);
CASSERT(offsetof(struct distributor, SGIR) == 0xf00, interrupts);
CASSERT(sizeof(struct distributor) == 0x1000, interrupts);

PACKED_STRUCT(cpuinterface) {
  PACKED_FIELD(u32, ICR);       /* CPU interface control register */
  PACKED_FIELD(u32, PMR);       /* interrupt priority mask register */
  PACKED_FIELD(u32, BPR);       /* binary point register */
  PACKED_FIELD(u32, IAR);       /* interrupt acknowledge register */
  PACKED_FIELD(u32, EOIR);      /* end of interrupt register */
  PACKED_FIELD(u32, RPR);       /* running priority register */
  PACKED_FIELD(u32, HPIR);      /* highest pending interrupt register */
  PACKED_FIELD(u32, ABPR);      /* aliased binary point register */
  PACKED_FIELD(u32, _rsvd1[8]);
  PACKED_FIELD(u32, impl_defined[36]); /* implementation defined registers */
  PACKED_FIELD(u32, _rsvd2[11]);
  PACKED_FIELD(u32, IIDR);             /* CPU interface identification register */
} PACKED_END;
CASSERT(offsetof(struct cpuinterface, EOIR) == 0x10, interrupts);
CASSERT(offsetof(struct cpuinterface, ABPR) == 0x1c, interrupts);
CASSERT(sizeof(struct cpuinterface) == 0x100, interrupts);

volatile struct distributor *DISTBASE;
volatile struct cpuinterface *CPUBASE;
static u32 GICVERSION, num_cpus, num_ints, num_ext_ints;
static u32 num_supported, num_permanent, num_prio_levels, min_binary_point;
static u32 supported[32], permanent[32];
#endif

#ifdef GIC
static irq_handler_t sgi_table[16];
static irq_handler_t ppi_table[16];
#endif
static irq_handler_t irq_table[96];
status intc_set_int_type_intid(u32 id, u32 is_edge);
status intc_set_targets_intid(u32 id, u32 targets);
u32 intc_get_targets_intid(u32 id);
status intc_set_priority_intid(u32 id, u32 prio);
status intc_unmask_intid(u32 id);
status intc_mask_intid(u32 id);

void intc_secondary_cpu_init(void)
{
  CPUBASE->BPR = min_binary_point;
  CPUBASE->PMR = 0xFF;          /* set mask to lowest priority */
  CPUBASE->ICR |= 1; /* enable interrupt forwarding in CPU interface */
}

/* INTerrupt Controller initialization */
void intc_init(void)
{
  int i;
#ifdef USE_VMM
  if(vmm_map_region(&mpu_intc_region) != OK) {
    early_panic("Unable to map interrupt controller registers.");
    return;
  }
#endif

#ifdef OMAP3530
  DLOG(1, "IP revision=%#x\n", intc->revision & 0xF);
  intc->sysconfig = 0x1;        /* Software reset */
  intc->n[0].mir_set = ~0;      /* Mask all IRQs */
  intc->n[1].mir_set = ~0;
  intc->n[2].mir_set = ~0;
#endif
#ifdef GIC
  DLOG(1, "CFGBASE=%#x\n", arm_config_base_address());
  PERIPHBASE = (u32 *)arm_config_base_address();
  DISTBASE = (void *)(PERIPHBASE + 1024);
  CPUBASE = (void *)(PERIPHBASE + 64);

  DLOG(1, "Snoop Control Unit CFG=%#x\n", PERIPHBASE[1]);
  DLOG(1, "DIST DCR=%#x ICTR=%#x\n", DISTBASE->DCR, DISTBASE->ICTR);
  num_cpus = 1 + GETBITS(DISTBASE->ICTR, 5, 3);
  num_ints = (1 + GETBITS(DISTBASE->ICTR, 0, 5)) << 5;
  num_ext_ints = GETBITS(DISTBASE->ICTR, 0, 5) << 5;
  DLOG(1, "%d processors; %d interrupts; %d external interrupt lines\n",
       num_cpus, num_ints, num_ext_ints);
  DLOG(1, "DIST implementor identification=%#x\n", DISTBASE->IIDR);
  DLOG(1, "CPU identification=%#x\n", CPUBASE->IIDR);
  DLOG(1, "DIST identification=%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
       DISTBASE->identification[0], DISTBASE->identification[1], DISTBASE->identification[2],
       DISTBASE->identification[3], DISTBASE->identification[4], DISTBASE->identification[5],
       DISTBASE->identification[6], DISTBASE->identification[7], DISTBASE->identification[8],
       DISTBASE->identification[9], DISTBASE->identification[10], DISTBASE->identification[11]);
  GICVERSION = GETBITS(DISTBASE->identification[6], 4, 4);
  DLOG(1, "Detected Generic Interrupt Controller Architecture v%d\n",
       GICVERSION);

  /* probe supported and permanently enabled interrupts */
  DISTBASE->DCR = 0;            /* disable distributor */
  u32 j;
  num_supported = num_permanent = 0;
  for(i=0; i<32; i++) {
    DISTBASE->ISER[i] = 0xFFFFFFFF;
    supported[i] = DISTBASE->ISER[i];
    for(j=0; j<32; j++) num_supported += GETBITS(supported[i], j, 1);
  }
  for(i=0; i<32; i++) {
    DISTBASE->ICER[i] = 0xFFFFFFFF;
    permanent[i] = DISTBASE->ICER[i];
    for(j=0; j<32; j++) num_permanent += GETBITS(permanent[i], j, 1);
  }
  DLOG(1, "Discovered %d supported and %d permanently enabled interrupts.\n", num_supported, num_permanent);
  DISTBASE->IPR[8] = 0xFF;
  num_prio_levels = 256 >> (31 - count_leading_zeroes(256 - DISTBASE->IPR[8]));
  DLOG(1, "Discovered %d interrupt priority levels.\n", num_prio_levels);
  DISTBASE->DCR = 1;            /* renable distributor */

  for(i=0; i<NUM_INTS; i++) {
    u32 word = intc_get_targets_intid(i);
    intc_set_targets_intid(i, ~word);
    if(intc_get_targets_intid(i) != word)
      break;                    /* first modifiable index */
  }
  DLOG(1, "Setting target for interrupt IDs >= %d to first CPU only.\n", i);
  for(; i<NUM_INTS; i++) {
    intc_set_targets_intid(i, 0x1);   /* set to single cpu */
    intc_set_int_type_intid(i, 0);    /* set to level mode */
  }

  CPUBASE->BPR = 0;
  min_binary_point = CPUBASE->BPR;
  DLOG(1, "CPU minimum binary point=%d\n", min_binary_point);
  CPUBASE->PMR = 0xFF;          /* set mask to lowest priority */
  DLOG(1, "CPU PMR=%#x RPR=%#x\n", CPUBASE->PMR, CPUBASE->RPR);

  CPUBASE->ICR |= 1; /* enable interrupt forwarding in CPU interface */

  DLOG(1, "Testing SGI\n");
  DISTBASE->SGIR = SETBITS(2, 24, 2) | SETBITS(3, 0, 4); /* send ID3 to self */
#endif

  for(i=0;i<96;i++) irq_table[i] = NULL;
#ifdef GIC
  for(i=0;i<16;i++) ppi_table[i] = NULL;
  for(i=0;i<16;i++) sgi_table[i] = NULL;
#endif
}

void intr_init(void)
{
  extern void *_vector_table;

  /* set up vector table base address */
  arm_set_vector_base_address((u32) &_vector_table);
  intc_init();
}

status intc_set_intid_handler(u32 intid, void (*handler)(u32))
{
  if(intid >= 32) irq_table[intid - 32] = handler;
  else if(intid >= 16) ppi_table[intid - 16] = handler;
  else sgi_table[intid] = handler;
  return OK;
}

status intc_set_irq_handler(u32 irq_num, void (*handler)(u32))
{
  irq_table[irq_num] = handler;
  return OK;
}

#ifdef GIC
status intc_mask_intid(u32 id)
{
  BITMAP_SET(DISTBASE->ICER, id);
  return OK;
}
#endif

status intc_mask_irq(u32 irq_num)
{
#ifdef OMAP3530
  if (irq_num < 32)
    intc->n[0].mir_set |= BIT(irq_num - 0);
  else if (irq_num < 64)
    intc->n[1].mir_set |= BIT(irq_num - 32);
  else
    intc->n[2].mir_set |= BIT(irq_num - 64);
  return OK;
#endif
#ifdef GIC
  return intc_mask_intid(irq_num + 32);
#endif
}

#ifdef GIC
status intc_unmask_intid(u32 id)
{
  BITMAP_SET(DISTBASE->ISER, id);
  return OK;
}
#endif

status intc_unmask_irq(u32 irq_num)
{
#ifdef OMAP3530
  if (irq_num < 32)
    intc->n[0].mir_clear |= BIT(irq_num - 0);
  else if (irq_num < 64)
    intc->n[1].mir_clear |= BIT(irq_num - 32);
  else
    intc->n[2].mir_clear |= BIT(irq_num - 64);
  return OK;
#endif
#ifdef GIC
  return intc_unmask_intid(irq_num + 32);
#endif
}

#ifdef GIC
status intc_set_priority_intid(u32 id, u32 prio)
{
  /* each 32-bit word is split into 4 8-bit priority fields */
  /* |........ ........ ........ ........|........ ........ ........ ........| */
  /*    IRQ0      IRQ1    IRQ2     IRQ3     IRQ4     IRQ5     IRQ6     IRQ7    */
  /* up to IRQ1020. Must use 32-bit access. */
  u32 array_i = id >> 2;
  u32 bit_i = (id & 0x3) << 3;
  /* ASSERT(array_i * 32 + bit_i == id * 8); */
  u32 word = DISTBASE->IPR[array_i];
  word &= ~(0xFF << bit_i);
  word |= (prio & 0xFF) << bit_i;
  DISTBASE->IPR[array_i] = word;
  return OK;
}
#endif

status intc_set_priority(u32 irq_num, u32 prio)
{
#ifdef OMAP3530
  return EINVALID;
#endif
#ifdef GIC
  return intc_set_priority_intid(irq_num + 32, prio);
#endif
}

#ifdef GIC
u32 intc_get_targets_intid(u32 id)
{
  /* each 32-bit word is split into 4 8-bit targets fields */
  /* |........ ........ ........ ........|........ ........ ........ ........| */
  /*    IRQ0      IRQ1    IRQ2     IRQ3     IRQ4     IRQ5     IRQ6     IRQ7    */
  /* up to IRQ1020. Must use 32-bit access. */
  u32 array_i = id >> 2;
  u32 bit_i = (id & 0x3) << 3;
  /* ASSERT(array_i * 32 + bit_i == id * 8); */
  u32 word = DISTBASE->IPTR[array_i];
  return (word << bit_i) & 0xFF;
}
#endif

u32 intc_get_targets(u32 irq_num)
{
#ifdef OMAP3530
  return 1;                     /* must be single-cpu anyhow */
#endif
#ifdef GIC
  return intc_get_targets_intid(irq_num + 32);
#endif
}

#ifdef GIC
status intc_set_targets_intid(u32 id, u32 targets)
{
  /* each 32-bit word is split into 4 8-bit targets fields */
  /* |........ ........ ........ ........|........ ........ ........ ........| */
  /*    IRQ0      IRQ1    IRQ2     IRQ3     IRQ4     IRQ5     IRQ6     IRQ7    */
  /* up to IRQ1020. Must use 32-bit access. */
  u32 array_i = id >> 2;
  u32 bit_i = (id & 0x3) << 3;
  /* ASSERT(array_i * 32 + bit_i == id * 8); */
  u32 word = DISTBASE->IPTR[array_i];
  word &= ~(0xFF << bit_i);
  word |= (targets & 0xFF) << bit_i;
  DISTBASE->IPTR[array_i] = word;
  return OK;
}
#endif

status intc_set_targets(u32 irq_num, u32 targets)
{
#ifdef OMAP3530
  return EINVALID;
#endif
#ifdef GIC
  return intc_set_targets_intid(irq_num + 32, targets);
#endif
}

#ifdef GIC
status intc_set_int_type_intid(u32 id, u32 is_edge)
{
  /* each 32-bit word is split into 16 2-bit config fields */
  /* the most significant config bit is for edge/level type */
  u32 array_i = id >> 4;
  u32 bit_i = ((id & 0xF) << 1) + 1;
  /* ASSERT(array_i * 32 + bit_i == id * 2 + 1); */
  u32 word = DISTBASE->ICFR[array_i];
  word &= ~(1 << bit_i);
  word |= (is_edge & 0x1) << bit_i;
  DISTBASE->ICFR[array_i] = word;
  return OK;
}
#endif

status intc_set_int_type(u32 irq_num, u32 is_edge)
{
#ifdef OMAP3530
  return EINVALID;
#endif
#ifdef GIC
  return intc_set_int_type_intid(irq_num + 32, is_edge);
#endif
}

u32 intc_get_running_priority(void)
{
#ifdef OMAP3530
  return 0xff;
#endif
#ifdef GIC
  return CPUBASE->RPR;
#endif
}

u32 intc_get_priority_mask(void)
{
#ifdef OMAP3530
  return 0xff;
#endif
#ifdef GIC
  return CPUBASE->PMR;
#endif
}

status intc_set_priority_mask(u32 prio)
{
#ifdef OMAP3530
  return EINVALID;
#endif
#ifdef GIC
  CPUBASE->PMR = prio & 0xff;
  return OK;
#endif
}

#ifdef __GNUC__
#define HANDLES(t) __attribute__ ((interrupt (t)))
#else
#error "HANDLES unsupported"
#endif

struct {
  char *source; u32 status; u32 dom_valid; u32 far_valid;
} fault_status_register_encodings[] = {
  { "Alignment", 0b00001, 0, 1 },
  { "PMSA - TLB miss (MPU)", 0b00000, 0, 1 },
  { "Instruction Cache Maintenance Operation Fault", 0b00100, 0, 1 },
  { "External Abort on Translation 1st level", 0b01100, 0, 1 },
  { "External Abort on Translation 2nd level", 0b01110, 1, 1 },
  { "Translation Section", 0b00101, 0, 1 },
  { "Translation Page", 0b00111, 1, 1 },
  { "Domain Section", 0b01001, 1, 1 },
  { "Domain Page", 0b01011, 1, 1 },
  { "Permission Section", 0b01101, 1, 1 },
  { "Permission Page", 0b01111, 1, 1 },
  { "Precise External Abort", 0b01000, 0, 1 },
  { "TLB Lock", 0b10100, 0, 0 },
  { "Coprocessor Data Abort", 0b11010, 0, 0 },
  { "Imprecise External Abort", 0b10110, 0, 0 },
  { "Parity Error Exception", 0b11000, 0, 0 },
  { "Debug event", 0b00010, 1, 0 },
  { "", 0, 0, 0 }
};

DEF_PER_CPU_EXTERN(process_t *, current);
DEF_PER_CPU_EXTERN(context_t *, _next_context);
DEF_PER_CPU_EXTERN(context_t *, _prev_context);

static int find_fault_status(u32 sr)
{
  u32 status = GETBITS(sr,0,4) | (GETBITS(sr,10,1) << 4);
  int i;
  for(i=0;fault_status_register_encodings[i].source[0] != 0;i++) {
    if(fault_status_register_encodings[i].status == status)
      return i;
  }
  return i;
}

#define DUMP_REGS do {                                                  \
  u32 regs[16];                                                         \
  clrex();                                                              \
  ASM("STMIA %0,{r0-r15}^"::"r"(regs));                                 \
  DLOG(1, "r0 : %#.08x r1: %#.08x r2 : %#.08x r3 : %#.08x\n", regs[0], regs[1], regs[2], regs[3]); \
  DLOG(1, "r4 : %#.08x r5: %#.08x r6 : %#.08x r7 : %#.08x\n", regs[4], regs[5], regs[6], regs[7]); \
  DLOG(1, "r8 : %#.08x r9: %#.08x r10: %#.08x r11: %#.08x\n", regs[8], regs[9], regs[10], regs[11]); \
  DLOG(1, "r12: %#.08x sp: %#.08x lr : %#.08x pc : %#.08x\n", regs[12], regs[13], regs[14], regs[15]); \
  } while(0)

void HANDLES() _handle_reset(void)
{
  DLOG(1, "_handle_reset\n");
}

void HANDLES("UNDEF") _handle_undefined_instruction(void)
{
  u32 lr;
  ASM("MOV %0, lr":"=r"(lr));
  DUMP_REGS;
  DLOG(1, "_handle_undefined_instruction @%#x = %#x\n", lr - 4, *((u32 *)(lr - 4)));
}

void _handle_swi2(u32 lr, u32 *r4_r11)
{
  process_t *cur = cpu_read(process_t *, current);
  extern u32 svc_stack_top;
  u32 *stack = (&svc_stack_top) - 7;
  cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
  cpu_write(context_t *, _next_context, &cpu_read(process_t *, current)->ctxt);
  lr -= 4;                      /* the SWI is the previous instruction */
#if 0
  DLOG(1, "_handle_swi lr=%#x (%#x), stack=%#x\n",
       lr, *((u32 *) lr), stack);
  DLOG(1, "r0 : %#.08x r1: %#.08x r2 : %#.08x r3 : %#.08x\n", stack[0], stack[1], stack[2], stack[3]);
  DLOG(1, "r4 : %#.08x r5: %#.08x r6 : %#.08x r7 : %#.08x\n", r4_r11[0], r4_r11[1], r4_r11[2], r4_r11[3]);
  DLOG(1, "r8 : %#.08x r9: %#.08x r10: %#.08x r11: %#.08x\n", r4_r11[4], r4_r11[5], r4_r11[6], r4_r11[7]);
  DLOG(1, "r12: %#.08x r14_irq: %#.08x spsr_irq: %#.08x\n", stack[4], stack[5], stack[6]);
#endif
  u32 instruction = *((u32 *) lr);
  /* SWI number is encoded as first 24 bits of SWI instruction */
  switch(instruction & 0xffffff) {
  case 0:                       /* set entry */
    cpu_read(process_t *, current)->entry = (void *) stack[1];
    return;
  case 3: {                        /* DLOG */
    /* r0: string pointer */
    /* r2: string length */
    /* r2: no prefix? */
    void *fmt = (void *) stack[0]; /* r0 */
    u32 fmtlen = stack[1];         /* r1 */
    if(process_is_valid_pointer(cur, fmt, fmtlen) == OK && ((char *)fmt)[fmtlen] == 0) {
      if(stack[2])
        DLOG_NO_PREFIX(1, "%s", (char *) fmt);
      else
        DLOG(1, "%s", (char *) fmt);
    } else
      DLOG(1, "invalid pointer given to syscall3 (ptr=%#x len=%d)\n", fmt, fmtlen);

    return;
  }
  }
}

void NAKED _handle_swi(u32 lr)
{
  ASM("STMFD sp!, {r4-r11,lr}\n\t"
      "MOV r1, sp\n\t"
      "BL _handle_swi2\n\t"
      "LDMFD sp!, {r4-r11,lr}\n\t"
      "BX lr");
}

void HANDLES("ABORT") _handle_prefetch_abort(void)
{
  u32 lr; u32 sp, sr;
  ASM("MOV %0, lr":"=r"(lr));
  ASM("MOV %0, sp":"=r"(sp));
  DUMP_REGS;
  sr = arm_mmu_instr_fault_status();
  DLOG(1, "_handle_prefetch_abort lr=%#x sp=%#x sr=%#x ar=%#x\n", lr - 4, sp, sr,
       arm_mmu_instr_fault_addr());
  DLOG(1, "  source=%s\n", fault_status_register_encodings[find_fault_status(sr)].source);
  early_panic("prefetch abort");
}

void HANDLES("ABORT") _handle_data_abort(void)
{
  u32 lr; u32 sp, sr;
  ASM("MOV %0, lr":"=r"(lr));
  ASM("MOV %0, sp":"=r"(sp));
  DUMP_REGS;
  sr = arm_mmu_data_fault_status();
  DLOG(1, "_handle_data_abort lr=%#x sp=%#x sr=%#x ar=%#x\n", lr - 8, sp, sr,
       arm_mmu_data_fault_addr());
  DLOG(1, "  source=%s (%s; SD=%d)\n",
       fault_status_register_encodings[find_fault_status(sr)].source,
       GETBITS(sr, 11, 1) ? "W" : "R",
       GETBITS(sr, 12, 1));
  early_panic("data abort");
}

void _handle_irq(void)
{
  cpu_write(context_t *, _prev_context, &cpu_read(process_t *, current)->ctxt);
  cpu_write(context_t *, _next_context, &cpu_read(process_t *, current)->ctxt);
#ifdef OMAP3530
  u32 activeirq = intc->sir_irq.activeirq;
#endif
#ifdef GIC
  /* read IAR to acknowledge interrupt */
  u32 IAR = CPUBASE->IAR;
  /* IAR = ID number of highest priority pending interrupt or number
   * that indicates spurious interrupt. */
  if(IAR == SPURIOUS_ID) return; /* spurious interrupt */
  u32 activeirq = IAR - 32;      /* shared peripheral IRQs start at 32 */
  if(IAR >= 32) {
#endif
    //DLOG(1, "_handle_irq activeirq=%#x\n", activeirq);
    intc_mask_irq(activeirq);
    if(irq_table[activeirq])
      irq_table[activeirq](activeirq);
#ifdef OMAP3530
    intc->control = INTCPS_CONTROL_NEWIRQAGR;
#endif
#ifdef GIC
  } else if(IAR >= 16) {
    //DLOG(1, "_handle_irq: PRIVATE PERIPHERAL INTERRUPT IAR=%#x\n", IAR);
    intc_mask_intid(IAR);
    if(ppi_table[IAR - 16])
      ppi_table[IAR - 16](IAR);
  } else {
    //DLOG(1, "_handle_irq: SOFTWARE GENERATED INTERRUPT IAR=%#x\n", IAR);
    intc_mask_intid(IAR);
    if(sgi_table[IAR])
      sgi_table[IAR](IAR);
  }
  CPUBASE->EOIR = IAR;
  /* EOIR write causes priority drop and interrupt deactivation */
  /* EOIR write must be from most recently acknowledged interrupt */
#if 0
  DLOG(1, "ABR[%#x]=%d\n", IAR, !!BITMAP_TST(DISTBASE->ABR, IAR));
  DLOG(1, "ISPR[%#x]=%d\n", IAR, !!BITMAP_TST(DISTBASE->ISPR, IAR));
  DLOG(1, "ISER[%#x]=%d\n", IAR, !!BITMAP_TST(DISTBASE->ISER, IAR));
  DLOG(1, "CPU PMR=%#x RPR=%#x\n", CPUBASE->PMR, CPUBASE->RPR);
#endif

#endif
  data_sync_barrier();
}

void HANDLES("FIQ") _handle_fiq(void)
{
  DLOG(1, "_handle_fiq\n");
}


/*
 * Local Variables:
 * indent-tabs-mode: nil
 * mode: C
 * c-file-style: "gnu"
 * c-basic-offset: 2
 * End:
 */

/* vi: set et sw=2 sts=2: */
