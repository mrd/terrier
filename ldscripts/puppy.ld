/*
 * LD Script for kernel
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

/* The physical addresses of the kernel begin at 0x80000000. The
 * virtual addresses begin at 0xC0000000. For some reason, the load
 * address must be strictly greater than 0x80000000, so I have chosen the
 * same as Linux, at 0x80008000. */

/* The stub is responsible for setting up virtual memory to match this file. */

ENTRY(_reset)
SECTIONS
{
 . = 0x80008000;
 _physical_start = .;
 _stub_start = .;

 /* The stub runs in identity-mapped space */

 .startup . : { init/startup.o (.text) }
 .stubtext : { init/stub.o (.text) }
 .stubARMtab : { init/stub.o (.ARM.extab) }
 .stubARMidx : { init/stub.o (.ARM.exidx) }

 _stub_end = ALIGN(0x1000);
 _stub_len = _stub_end - _stub_start;

 . = 0xC0008000;
 /* From now on, virtual and physical addresses are separate. */
 . += _stub_len;                /* skip stub code */

 _kernel_start = .;

 /* The virtual address is taken from the counter, but the physical
  * address must be specified with AT. */

 .text : AT ( _stub_end ) { *(.text) }
 .rodata : AT ( LOADADDR (.text) + SIZEOF (.text) ) { *(.rodata) }

 . = ALIGN(0x1000);

 _kernel_readonly_end = .;

 .data : AT ( LOADADDR (.text) + _kernel_readonly_end - _kernel_start ) { *(.data) }
 .bss : AT ( LOADADDR (.data) + SIZEOF (.data) ) { *(.bss) }

 . = ALIGN(0x1000);

 . = . + 0x1000; /* 4kB of stack memory */
 svc_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 fiq_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 irq_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 sys_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 abt_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 und_stack_top = .;

 _kernel_end = ALIGN(0x1000);

 _kernel_pages = (_kernel_end - _kernel_start) / 0x1000;
 _kernel_readonly_pages = (_kernel_readonly_end - _kernel_start) / 0x1000;
 _kernel_readwrite_pages = (_kernel_end - _kernel_readonly_end) / 0x1000;

 /* compute the physical address of the l1table */
 _l1table_phys = l1table - ((_kernel_start - _stub_len) - _stub_start);

 /* To make arm-none-linux-gnueabi-gcc shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
