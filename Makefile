# Top-level Makefile
#
# -------------------------------------------------------------------
#
# Copyright (C) 2011 Matthew Danish.
#
# All rights reserved. Please see enclosed LICENSE file for details.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# - Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# - Neither the name of the author nor the names of its contributors
#   may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.

ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage

ADDRESS = 0x80008000

IMGNAME = puppy

OPT = 2

SFLAGS = -Iinclude
CFLAGS = -Iinclude -O$(OPT)

LIBGCC = $(shell $(CC) -print-libgcc-file-name)

.PHONY: all
all: $(IMGNAME.uimg) ucmd ukermit $(IMGNAME)-nand.bin

$(IMGNAME).uimg: $(IMGNAME).bin
	$(MKIMAGE) -A arm -C none -O linux -T kernel -d $< -a $(ADDRESS) -e $(ADDRESS) $@

$(IMGNAME)-nand.bin: $(IMGNAME).uimg u-boot/u-boot.bin util/bb_nandflash_ecc
	sh util/nand.sh $< $@ 2> /dev/null

$(IMGNAME).bin: $(IMGNAME).elf
	$(OBJCOPY) -O binary $< $@

$(IMGNAME).elf: init/init.o init/startup.o init/early_uart3.o init/device_id.o ldscripts/$(IMGNAME).ld
	$(LD) -T ldscripts/$(IMGNAME).ld init/init.o init/startup.o init/early_uart3.o init/device_id.o $(LIBGCC) -o $@

%.o: %.S
	$(CC) $(SFLAGS) $(SFLAGS) -c $< -o $@

.PHONY: clean test

clean:
	rm -f $(IMGNAME).uimg $(IMGNAME).elf $(IMGNAME).bin init/startup.o init/init.o ucmd ukermit
	(cd util; make clean)
	(cd omap-u-boot-utils; make clean V=1)

distclean: clean
	rm -f $(IMGNAME)-nand.bin
	(cd omap-u-boot-utils; make distclean V=1)

test: $(IMGNAME)-nand.bin
	@echo "***** To quit QEMU type: Ctrl-a x"
	qemu-system-arm -M beagle -m 128M -nographic -mtdblock $<

util/bb_nandflash_ecc: util/bb_nandflash_ecc.c
	(cd util; make bb_nandflash_ecc)

.PHONY: ucmd ukermit
ucmd:
	(cd omap-u-boot-utils; make ucmd V=1)
	cp omap-u-boot-utils/ucmd .

ukermit:
	(cd omap-u-boot-utils; make ukermit V=1)
	cp omap-u-boot-utils/ukermit .

init/startup.o: init/startup.S
init/init.o: init/init.c
init/early_uart3.o: init/early_uart3.c
init/device_id.o: init/device_id.c

# vi: set noexpandtab sts=8 sw=8:
