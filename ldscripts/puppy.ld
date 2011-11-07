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

ENTRY(_reset)
SECTIONS
{
 . = 0x80008000;
 _kernel_start = .;

 .startup . : { init/startup.o(.text) }
 _physical_start = LOADADDR(.startup);
 .text : { *(.text) }
 . = ALIGN(0x1000);

 _kernel_readonly_end = .;

 .data : { *(.data) }
 .bss : { *(.bss) }

 . = ALIGN(0x1000);

 . = . + 0x1000; /* 4kB of stack memory */
 svc_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 fiq_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 irq_stack_top = .;

 . = . + 0x1000; /* 4kB of stack memory */
 sys_stack_top = .;

 _kernel_end = ALIGN(0x1000);
 _kernel_pages = (_kernel_end - _kernel_start) / 0x1000;
 _kernel_readonly_pages = (_kernel_readonly_end - _kernel_start) / 0x1000;
 _kernel_readwrite_pages = (_kernel_end - _kernel_readonly_end) / 0x1000;

 /* To make arm-none-linux-gnueabi-gcc shut up: */
 __aeabi_unwind_cpp_pr0 = .;
 __aeabi_unwind_cpp_pr1 = .;
}
