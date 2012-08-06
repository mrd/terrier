/*
 * Begin initialization in C.
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
#include "arm/memory.h"
#include "arm/asm.h"
#include "mem/virtual.h"

#ifdef USE_VMM
#ifdef OMAP4460
static region_t sdrc_region = { 0x4E000000, (void *) 0x4E000000, &l1pt, 1, 20, 0, 0, R_PM }; /* DMM region */
region_t l4wakeup_region = { 0x4A000000, (void *) 0x4A000000, &l1pt, 1, 20, 0, 0, R_PM };
#endif
#ifdef OMAP3530
static region_t sdrc_region = { 0x6D000000, (void *) 0x6D000000, &l1pt, 1, 20, 0, 0, R_PM };
region_t l4wakeup_region = { 0x48300000, (void *) 0x48300000, &l1pt, 1, 20, 0, 0, R_PM };
#endif
#endif

#ifdef OMAP3530
PACKED_STRUCT(control_module_id) {
  PACKED_FIELD(u32, _rsv1);
  PACKED_FIELD(u32, control_idcode); /* 0x4830A204 */
  PACKED_FIELD(u32, _rsv2[2]);
  PACKED_FIELD(u32, control_production_id); /* 0x4830A210 */
  PACKED_FIELD(u32, _rsv3);
  PACKED_FIELD(u32, control_die_id[4]); /* 0x4830A218 */
} PACKED_END;
static volatile struct control_module_id *cm_id = (struct control_module_id *) 0x4830A200;
#endif

#ifdef OMAP4460
#define SYSCTRL_GENERAL_CORE 0x4A002000
PACKED_STRUCT(omap4_control_module) {
  PACKED_FIELD(u32, gen_core_revision);
  PACKED_FIELD(u32, gen_core_hwinfo);
  PACKED_FIELD(u32, _rsv1[2]);
  PACKED_FIELD(u32, gen_core_sysconfig);
  PACKED_FIELD(u32, _rsv2[123]);
  PACKED_FIELD(u32, std_fuse_die_id_0);
  PACKED_FIELD(u32, id_code);
  PACKED_FIELD(u32, std_fuse_die_id_1);
  PACKED_FIELD(u32, std_fuse_die_id_2);
  PACKED_FIELD(u32, std_fuse_die_id_3);
  PACKED_FIELD(u32, std_fuse_prod_id_0);
  PACKED_FIELD(u32, std_fuse_prod_id_1);
} PACKED_END;
#endif

#ifdef OMAP3530
static struct { char *version; u32 idcode; } idcodes[] = {
  {"OMAP35x ES1.0"   , 0x0B6D602F},
  {"OMAP35x ES2.0"   , 0x1B7AE02F},
  {"OMAP35x ES2.1"   , 0x2B7AE02F},
  {"OMAP35x ES3.0"   , 0x3B7AE02F},
  {"OMAP35x ES3.1"   , 0x4B7AE02F},
  {"OMAP35x ES3.1.2" , 0x7B7AE02F},
  {NULL, 0}
};
#endif

void meminfo(void)
{
#ifdef OMAP3530
  extern void *_physical_start, *_kernel_pages;
  volatile u32 *SDRC_MCFG_0 = (u32 *) 0x6D000080;
  volatile u32 *SDRC_MCFG_1 = (u32 *) 0x6D0000B0;
  volatile u32 *SDRC_CS_CFG = (u32 *) 0x6D000040;
  u32 cs0_ramsize = ((*SDRC_MCFG_0 >> 8) & 0x3FF) << 1;
  u32 cs1_ramsize = ((*SDRC_MCFG_1 >> 8) & 0x3FF) << 1;
  u32 cs1_start   = (((*SDRC_CS_CFG >> 8) & 0x3) | ((*SDRC_CS_CFG & 0xF) << 2)) * 0x2000000 + 0x80000000;
  u32 cs0_start   = 0x80000000, cs0_end = 0x80000000 + (cs0_ramsize << 20) - 1;
  u32 cs1_end     = cs1_start + (cs1_ramsize << 20) - 1;
  u32 kp_start    = (u32) &_physical_start;
  u32 kp_end      = (u32) &_physical_start + (((u32) &_kernel_pages) << 12) - 1;
  printf_uart3("SDRC CS0_size=%d MB CS1_size=%d MB CS1_start=%x\n",
               cs0_ramsize, cs1_ramsize, cs1_start);
  printf_uart3("Physical memory map:\n");
  printf_uart3("  %#x - %#x CS0 dynamic RAM\n", cs0_start, cs0_end);
  printf_uart3("    %#x - %#x kernel physical addresses\n", kp_start, kp_end);
  if (cs1_ramsize)
    printf_uart3("  %#x - %#x CS1 dynamic RAM\n", cs1_start, cs1_end);
#endif
#ifdef OMAP4460
  volatile u32 *DMM = (u32 *) 0x4E000000;
  u32 lisa_cnt = DMM[2] & 0x1F, i;
  printf_uart3("DMM rev=%#x hwinfo=%#x lisainfo=%#x\n", DMM[0], DMM[1], DMM[2]);
  lisa_cnt = lisa_cnt > 4 ? 4 : lisa_cnt; /* only seems to be 4 in the manual */
  for(i=0;i<lisa_cnt;i++) {
    printf_uart3("  LISA_MAP_%d: %#x sysaddr=%#x size=%dM addrspc=%d intlv=%d map=%d sdrcaddr=%#x\n",
                 i, DMM[16+i],
                 GETBITS(DMM[16+i], 24, 8),
                 1 << (GETBITS(DMM[16+i], 20, 3) + 4),
                 GETBITS(DMM[16+i], 18, 2),
                 GETBITS(DMM[16+i], 16, 2),
                 GETBITS(DMM[16+i], 8, 2),
                 GETBITS(DMM[16+i], 0, 8));
  }
#endif
}

static char *cache_type_desc[] = {
  "None", "I-only", "D-only", "Separate I&D", "Unified", "Reserved", "Reserved", "Reserved"
};

void cacheinfo(void)
{
  u32 cache_type, tlb_type, cr, clidr, i;
  ASM("MRC p15, #0, %0, c1, c0, #0":"=r"(cr));
  printf_uart3("control_register=%#x\n", cr);
  ASM("MRC p15, #0, %0, c0, c0, #1":"=r"(cache_type));
  printf_uart3("cache_type=%#x writeback_granule=%d dminline=%d iminline=%d l1policy=%s\n",
               cache_type,
               1 << GETBITS(cache_type, 24, 4),
               GETBITS(cache_type, 16, 4) << 2,
               GETBITS(cache_type, 0, 4) << 2,
               GETBITS(cache_type, 14, 2) == 2 ? "(virtual index, physical tag)" : "(unknown)");
  ASM("MRC p15, #0, %0, c0, c0, #3":"=r"(tlb_type));
  printf_uart3("tlb_type=%#x ILsize=%d DLsize=%d separate_I_D=%d\n",
               tlb_type, GETBITS(tlb_type, 16, 8), GETBITS(tlb_type, 8, 8), GETBITS(tlb_type, 0, 1));
  ASM("MRC p15, #1, %0, c0, c0, #1":"=r"(clidr));
  printf_uart3("cache level ID reg=%#x LoUU=%d LoC=%d LoUIS=%d\n",
               clidr,
               GETBITS(clidr, 27, 3),
               GETBITS(clidr, 24, 3),
               GETBITS(clidr, 21, 3));
  for(i=0;i<7;i++) {
    u32 ctype = GETBITS(clidr,i*3,3), ccsidr, numsets, assocs;
    if(ctype) {
      printf_uart3("  L%d: %s\n", i+1, cache_type_desc[ctype]);
      if(ctype != 1) {
        /* CSSELR: Select current CCSIDR by Level and Type=0 (Data or Unified) */
        ASM("MCR p15, #2, %0, c0, c0, #0"::"r"((i<<1)|0));
        /* CCSIDR: Cache Size ID registers */
        ASM("MRC p15, #1, %0, c0, c0, #0":"=r"(ccsidr));
        assocs = GETBITS(ccsidr,3,10)+1;
        numsets = GETBITS(ccsidr,13,15)+1;
        printf_uart3("  L%d D: CCSIDR=%#x linesize=%d assoc=%d numsets=%d %s%s%s%s\n",
                     i+1, ccsidr,
                     1<<(GETBITS(ccsidr, 0, 3)+4), /* words -> bytes */
                     assocs,
                     numsets,
                     GETBITS(ccsidr,28,1)? "WT " : "",
                     GETBITS(ccsidr,29,1)? "WB " : "",
                     GETBITS(ccsidr,30,1)? "RA " : "",
                     GETBITS(ccsidr,31,1)? "WA " : "");
        u32 A = ceiling_log2(assocs);
        u32 S = ceiling_log2(numsets);
        u32 L = GETBITS(ccsidr, 0, 3)+4;
        u32 s, w;
        for(w=0;w<assocs;w++) {
          for(s=0;s<numsets;s++) {
            u32 Rt = SETBITS(i,1,3) | SETBITS(s,L,S) | SETBITS(w,32-A,A);
            ASM("MCR p15, #0, %0, c7, c14, #2"::"r"(Rt));
          }
        }
      }
      if(ctype == 1 || ctype == 3) {
        /* CSSELR: Select current CCSIDR by Level and Type=1 (Instruction) */
        ASM("MCR p15, #2, %0, c0, c0, #0"::"r"((i<<1)|1));
        /* CCSIDR: Cache Size ID registers */
        ASM("MRC p15, #1, %0, c0, c0, #0":"=r"(ccsidr));
        printf_uart3("  L%d I: CCSIDR=%#x linesize=%d assoc=%d numsets=%d %s%s%s%s\n",
                     i+1, ccsidr,
                     1<<(GETBITS(ccsidr, 0, 3)+4), /* words -> bytes */
                     GETBITS(ccsidr,3,10)+1,
                     GETBITS(ccsidr,13,15)+1,
                     GETBITS(ccsidr,28,1)? "WT " : "",
                     GETBITS(ccsidr,29,1)? "WB " : "",
                     GETBITS(ccsidr,30,1)? "RA " : "",
                     GETBITS(ccsidr,31,1)? "WA " : "");
      }
    }
  }
}

void identify_device(void)
{
  u32 mainid;
  u32 feat;
  char *core;

  ASM("MRC p15, #0, %0, c0, c0, #0":"=r"(mainid));
  switch(GETBITS(mainid, 4, 12)) {
  case 0xc08:
    core = "Cortex-A8";
    break;
  case 0xc09:
    core = "Cortex-A9";
    break;
  default:
    core = "n/a";
    break;
  }
  printf_uart3("MainID=%#x %s variant=%d arch=%#x %s (%#x) rev=%d\n",
               mainid,
               GETBITS(mainid, 24, 8) == 0x41 ? "ARM" : "n/a",
               GETBITS(mainid, 20, 4),
               GETBITS(mainid, 16, 4),
               core,
               GETBITS(mainid, 4, 12),
               GETBITS(mainid, 0, 4));
  ASM("MRC p15, #0, %0, c0, c1, #0":"=r"(feat));
  printf_uart3("Proc_Feat0=%#x%s%s%s%s\n",
               feat,
               GETBITS(feat, 12, 4) == 1 ? " (ThumbEE)" : "",
               GETBITS(feat, 8, 4) == 1 ? " (Jazelle)" : "",
               GETBITS(feat, 4, 4) == 3 ? " (Thumb-2)" : "",
               GETBITS(feat, 0, 4) == 1 ? " (ARM)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #1":"=r"(feat));
  printf_uart3("Proc_Feat1=%#x%s%s%s\n",
               feat,
               GETBITS(feat, 8, 4) ? " (uController)" : "",
               GETBITS(feat, 4, 4) == 1 ? " (Sec. Ext. v1)" : "",
               GETBITS(feat, 0, 4) == 1 ? " (ARMv4 model)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #2":"=r"(feat));
  printf_uart3("Debug_Feat=%#x%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 20, 4) ? " (uController debug - mmap)" : "",
               GETBITS(feat, 16, 4) ? " (trace debug - mmap)" : "",
               GETBITS(feat, 12, 4) ? " (trace debug - co-p)" : "",
               GETBITS(feat, 8, 4) == 4 ? " (core debug - mmap)" : "",
               GETBITS(feat, 4, 4) ? " (secure debug - co-p)" : "",
               GETBITS(feat, 0, 4) ? " (core debug - co-p)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #4":"=r"(feat));
  printf_uart3("MemModel_Feat0=%#x%s%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 24, 4) ? " (FCSE)" : "",
               GETBITS(feat, 20, 4) ? " (aux control)" : "",
               GETBITS(feat, 16, 4) ? " (TCM)" : "",
               GETBITS(feat, 12, 4) ? " (outer shareable)" : "",
               GETBITS(feat, 8, 4)  ? " (cache coherence)" : "",
               GETBITS(feat, 4, 4) ? " (PMSA)" : "",
               GETBITS(feat, 0, 4) == 3 ? " (VMSAv7/ARMv6-ext)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #5":"=r"(feat));
  printf_uart3("MemModel_Feat1=%#x%s%s%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 28, 4) == 2 ? " (BTB)" : "",
               GETBITS(feat, 24, 4) ? " (L1 test clean)" : "",
               GETBITS(feat, 20, 4) ? " (L1 unified maint.)" : "",
               GETBITS(feat, 16, 4) ? " (L1 Harvard maint.)" : "",
               GETBITS(feat, 12, 4)  ? " (L1 unified maint. SaW)" : "",
               GETBITS(feat, 8, 4) ? " (L1 Harvard maint. SaW)" : "",
               GETBITS(feat, 4, 4) ? " (L1 unified maint. MVA)" : "",
               GETBITS(feat, 0, 4) ? " (L1 Harvard maint. MVA)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #6":"=r"(feat));
  printf_uart3("MemModel_Feat2=%#x%s%s%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 28, 4) ? " (HW access flag)" : "",
               GETBITS(feat, 24, 4) ? " (WFI)" : "",
               GETBITS(feat, 20, 4) == 2 ? " (mem barriers)" : "",
               GETBITS(feat, 16, 4) ? " (unified TLB)" : "",
               GETBITS(feat, 12, 4) == 2 ? " (Harvard TLB)" : "",
               GETBITS(feat, 8, 4) ? " (L1 Harvard maint. range)" : "",
               GETBITS(feat, 4, 4) ? " (L1 Harvard bg prefetch)" : "",
               GETBITS(feat, 0, 4) ? " (L1 Harvard fg prefetch)" : "");
  ASM("MRC p15, #0, %0, c0, c1, #7":"=r"(feat));
  printf_uart3("MemModel_Feat3=%#x%s%s%s%s\n",
               feat,
               GETBITS(feat, 28, 4) ? " (supersections)" : "",
               GETBITS(feat, 8, 4) == 2 ? " (branch pred. maint.)" : "",
               GETBITS(feat, 4, 4) ? " (cache maint. SaW)" : "",
               GETBITS(feat, 0, 4) ? " (cache maint. MVA)" : "");
  ASM("MRC p15, #0, %0, c0, c2, #0":"=r"(feat));
  printf_uart3("ISA0=%#x%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 24, 4) ? " (DIV)" : "",
               GETBITS(feat, 20, 4) ? " (BKPT)" : "",
               GETBITS(feat, 12, 4) ? " (Compare&Branch)" : "",
               GETBITS(feat, 8, 4) ? " (Bitfield)" : "",
               GETBITS(feat, 4, 4) ? " (CLZ)" : "",
               GETBITS(feat, 0, 4) ? " (SWP/SWPB)" : "");
  ASM("MRC p15, #0, %0, c0, c2, #1":"=r"(feat));
  printf_uart3("ISA1=%#x%s%s%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 28, 4) ? " (Jazelle)" : "",
               GETBITS(feat, 24, 4) == 3 ? " (ARM/Thumb branches)" : "",
               GETBITS(feat, 20, 4) ? " (immediate instr)" : "",
               GETBITS(feat, 16, 4) ? " (IfThen)" : "",
               GETBITS(feat, 12, 4) == 2 ? " (Sign/Zero-extend)" : "",
               GETBITS(feat, 8, 4) ? " (Exception2 instrs)" : "",
               GETBITS(feat, 4, 4) ? " (Exception1 instrs)" : "",
               GETBITS(feat, 0, 4) ? " (Endian control)" : "");
  ASM("MRC p15, #0, %0, c0, c2, #2":"=r"(feat));
  printf_uart3("ISA2=%#x%s%s%s%s%s%s%s%s\n",
               feat,
               GETBITS(feat, 28, 4) == 2 ? " (Reversal)" : "",
               GETBITS(feat, 24, 4) ? " (PSR instrs)" : "",
               GETBITS(feat, 20, 4) == 2 ? " (unsigned multiply)" : "",
               GETBITS(feat, 16, 4) == 3 ? " (signed multiply)" : "",
               GETBITS(feat, 12, 4) == 2 ? " (multiply)" : "",
               GETBITS(feat, 8, 4) ? " (interruptible instrs)" : "",
               GETBITS(feat, 4, 4) == 3 ? " (memory hint instrs)" : "",
               GETBITS(feat, 0, 4) ? " (LDRD/STRD)" : "");


#ifdef USE_VMM
  if(vmm_map_region(&sdrc_region) != OK) {
    printf_uart3("Unable to map sdrc_region\n");
    return;
  }
  if(vmm_map_region(&l4wakeup_region) != OK) {
    printf_uart3("Unable to map l4wakeup_region\n");
    return;
  }
#endif

#ifdef OMAP4460
  struct omap4_control_module *o4cm = (struct omap4_control_module *) SYSCTRL_GENERAL_CORE;
  printf_uart3("rev=%#x hwinfo=%#x syscnf=%#x idcode=%#x dieID=%#x%x%x%x prodID=%#x%x\n",
               o4cm->gen_core_revision, o4cm->gen_core_hwinfo, o4cm->gen_core_sysconfig,
               o4cm->id_code,
               o4cm->std_fuse_die_id_0, o4cm->std_fuse_die_id_1, o4cm->std_fuse_die_id_2, o4cm->std_fuse_die_id_3,
               o4cm->std_fuse_prod_id_0, o4cm->std_fuse_prod_id_1);
#endif

#ifdef OMAP3530
  int i;
  u32 idcode;
  u32 prodcode;

  idcode = cm_id->control_idcode;
  prodcode = cm_id->control_production_id;
  printf_uart3("IDCODE=%x PRODUCTION_ID=%x DIE_ID=%x%x%x%x\n",
               idcode, prodcode,
               cm_id->control_die_id[3],
               cm_id->control_die_id[2],
               cm_id->control_die_id[1],
               cm_id->control_die_id[0]);
  for(i=0; idcodes[i].idcode != 0; i++)
    if(idcodes[i].idcode == idcode) {
      printf_uart3("%s %s\n", idcodes[i].version, prodcode == 0xf0 ? "(GP device)" : "");
      break;
    }
#endif

  meminfo();
  cacheinfo();
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
