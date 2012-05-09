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
#include "omap3/early_uart3.h"
#include "intr/interrupts.h"
#include "arm/memory.h"
#include "arm/status.h"
#include "arm/asm.h"
#include "mem/virtual.h"
#include "sched/process.h"
#define MODULE "intr"
#include "debug/log.h"
#include "debug/cassert.h"

#define MPU_INTC_BASE_PHYS_ADDR 0x48200000

#define MPU_INTC_BASE_ADDR 0x48200000

#ifdef USE_VMM
region_t mpu_intc_region = {
  MPU_INTC_BASE_PHYS_ADDR,
  (void *) MPU_INTC_BASE_ADDR,
  &l1pt, 1, 20, 0, 0, R_PM
};
#endif

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
/* CASSERT(offsetof(struct intcps, threshold) == 0x68, interrupts); */
/* CASSERT(offsetof(struct intcps, n[2].pending_fiq) == 0xdc, interrupts); */
/* CASSERT(offsetof(struct intcps, ilr) == 0x100, interrupts); */

static volatile struct intcps *intc = (struct intcps *) MPU_INTC_BASE_ADDR;

static irq_handler_t irq_table[96];

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

  DLOG(1, "IP revision=%#x\n", intc->revision & 0xF);
  intc->sysconfig = 0x1;        /* Software reset */
  intc->n[0].mir_set = ~0;      /* Mask all IRQs */
  intc->n[1].mir_set = ~0;
  intc->n[2].mir_set = ~0;
  for(i=0;i<96;i++) irq_table[i] = NULL;
}

void intr_init(void)
{
  extern void *_vector_table;

  /* set up vector table base address */
  arm_set_vector_base_address((u32) &_vector_table);
  intc_init();
}

status intc_set_irq_handler(u32 irq_num, void (*handler)(u32))
{
  irq_table[irq_num] = handler;
  return OK;
}

status intc_mask_irq(u32 irq_num)
{
  if (irq_num < 32)
    intc->n[0].mir_set |= BIT(irq_num - 0);
  else if (irq_num < 64)
    intc->n[1].mir_set |= BIT(irq_num - 32);
  else
    intc->n[2].mir_set |= BIT(irq_num - 64);
  return OK;
}

status intc_unmask_irq(u32 irq_num)
{
  if (irq_num < 32)
    intc->n[0].mir_clear |= BIT(irq_num - 0);
  else if (irq_num < 64)
    intc->n[1].mir_clear |= BIT(irq_num - 32);
  else
    intc->n[2].mir_clear |= BIT(irq_num - 64);
  return OK;
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

extern context_t *_prev_context, *_next_context;
extern process_t *current;

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

void HANDLES() _handle_reset(void)
{
  DLOG(1, "_handle_reset\n");
}

void HANDLES("UNDEF") _handle_undefined_instruction(void)
{
  u32 lr;
  ASM("MOV %0, lr":"=r"(lr));
  DLOG(1, "_handle_undefined_instruction @%#x = %#x\n", lr - 4, *((u32 *)(lr - 4)));
}

void _handle_swi2(u32 lr, u32 *r4_r11)
{
  extern process_t *current;
  extern u32 svc_stack_top;
  u32 *stack = (&svc_stack_top) - 7;
  _prev_context = _next_context = &current->ctxt;
  lr -= 4;                      /* the SWI is the previous instruction */
  DLOG(1, "_handle_swi lr=%#x (%#x), stack=%#x\n",
       lr, *((u32 *) lr), stack);
  DLOG(1, "r0 : %#.08x r1: %#.08x r2 : %#.08x r3 : %#.08x\n", stack[0], stack[1], stack[2], stack[3]);
  DLOG(1, "r4 : %#.08x r5: %#.08x r6 : %#.08x r7 : %#.08x\n", r4_r11[0], r4_r11[1], r4_r11[2], r4_r11[3]);
  DLOG(1, "r8 : %#.08x r9: %#.08x r10: %#.08x r11: %#.08x\n", r4_r11[4], r4_r11[5], r4_r11[6], r4_r11[7]);
  DLOG(1, "r12: %#.08x r14_irq: %#.08x spsr_irq: %#.08x\n", stack[4], stack[5], stack[6]);

  switch(stack[0]) {
  case 0:                       /* set entry */
    current->entry = (void *) stack[1];
    return;
  }
}

void __attribute__((naked)) _handle_swi(u32 lr)
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
  sr = arm_mmu_instr_fault_status();
  DLOG(1, "_handle_prefetch_abort lr=%#x sp=%#x sr=%#x ar=%#x\n", lr - 4, sp, sr,
       arm_mmu_instr_fault_addr());
  DLOG(1, "  source=%s\n", fault_status_register_encodings[find_fault_status(sr)].source);
  early_panic("prefetch abort");
}

void HANDLES("ABORT") _handle_data_abort(void)
{
  u32 *regs;
  ASM("STMFD sp!,{r0-r12}");
  ASM("MOV %0, sp":"=r"(regs));
  ASM("ADD sp, sp, #52");
  u32 lr, sp, sr;
  ASM("MOV %0, lr":"=r"(lr));
  ASM("MOV %0, sp":"=r"(sp));
  DLOG(1, "r0 : %#.08x r1: %#.08x r2 : %#.08x r3 : %#.08x\n", regs[0], regs[1], regs[2], regs[3]);
  DLOG(1, "r4 : %#.08x r5: %#.08x r6 : %#.08x r7 : %#.08x\n", regs[4], regs[5], regs[6], regs[7]);
  DLOG(1, "r8 : %#.08x r9: %#.08x r10: %#.08x r11: %#.08x\n", regs[8], regs[9], regs[10], regs[11]);
  DLOG(1, "r12: %#.08x sp: %#.08x lr : %#.08x\n", regs[12], sp, lr);
  sr = arm_mmu_data_fault_status();
  DLOG(1, "_handle_data_abort lr=%#x sp=%#x sr=%#x ar=%#x\n", lr - 8, sp, sr,
       arm_mmu_data_fault_addr());
  DLOG(1, "  source=%s\n", fault_status_register_encodings[find_fault_status(sr)].source);
  early_panic("data abort");
}

void _handle_irq(void)
{
  u32 activeirq = intc->sir_irq.activeirq;
  _prev_context = _next_context = &current->ctxt;
  intc_mask_irq(activeirq);
  if (irq_table[activeirq])
    irq_table[activeirq](activeirq);
  else
    DLOG(1, "_handle_irq sir_irq=%#x\n", activeirq);
  intc->control = INTCPS_CONTROL_NEWIRQAGR;
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
