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

##################################################

SCHED=rms_sched

C_FILES = init/init.c init/stub.c init/early_uart3.c init/device_id.c \
	mem/virtual.c intr/interrupts.c omap/timer.c omap/smp.c \
	sched/process.c \
	debug/log.c tests/cswitch.c tests/test_process.c
S_FILES = init/startup.S intr/table.S
DATS_FILES = sched/$(SCHED).dats mem/physical.dats
PROGS = progs/idle progs/uart progs/ehci progs/test progs/test2 progs/smsclan progs/asix

IMGNAME = terrier
ADDRESS = 0x80008000
TESTS =

##################################################

include config.mk

##################################################

CFLAGS += -Iinclude -D$(CORE) -Wno-attributes
SFLAGS += -Iinclude -D$(CORE)

ATSFLAGS = $(KERNEL_ATSFLAGS)
ATSCFLAGS = $(KERNEL_ATSCFLAGS)

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
ATSCFLAGS += -g -O0
else
CFLAGS += -O$(OPT)
ATSCFLAGS += -O$(OPT)
endif
CFLAGS += $(patsubst %,-DTEST_%,$(TESTS))

ifeq ($(USE_VMM),1)
CFLAGS += -DUSE_VMM
SFLAGS += -DUSE_VMM
LDSCRIPT = ldscripts/$(IMGNAME)-vmm.ld
else
LDSCRIPT = ldscripts/$(IMGNAME).ld
endif

NO_SMP=1 # FIXME: until I fix the scheduler
ifeq ($(NO_SMP),1)
CFLAGS += -DNO_SMP
SFLAGS += -DNO_SMP
endif

CFLAGS += -DSCHED=$(SCHED)

LIBGCC = $(shell $(CC) -print-libgcc-file-name)

##################################################

C_OBJS = $(patsubst %.c,%.o,$(C_FILES))
S_OBJS = $(patsubst %.S,%.o,$(S_FILES))
DATS_OBJS = $(patsubst %.dats,%_dats.o,$(DATS_FILES))
ATSCTEMPS = $(patsubst %.dats,%_dats.c,$(DATS_FILES))
OBJS = $(C_OBJS) $(S_OBJS) $(DATS_OBJS)
DFILES = $(patsubst %.c,%.d,$(C_FILES)) $(patsubst %.S,%.d,$(S_FILES)) $(patsubst %.dats,%_dats.d,$(DATS_FILES))
POBJS = $(patsubst %,%/program,$(PROGS))
UBOOT = u-boot/$(CORE)/u-boot.bin

##################################################

.PHONY: all
.SECONDARY:

all: $(IMGNAME.uimg) ucmd ukermit $(IMGNAME)-nand.bin

$(IMGNAME).uimg: $(IMGNAME).bin.gz
	$(MKIMAGE) -A arm -C gzip -O linux -T kernel -d $< -a $(ADDRESS) -e $(ADDRESS) $@

$(IMGNAME)-nand.bin: $(IMGNAME).uimg $(UBOOT) util/bb_nandflash_ecc
	sh util/nand.sh $< $@ $(CORE) 2> /dev/null

$(IMGNAME).bin.gz: $(IMGNAME).bin
	gzip < $< > $@

$(IMGNAME).bin: $(IMGNAME).elf
	$(OBJCOPY) -O binary $< $@

$(IMGNAME).elf: $(OBJS) $(LDSCRIPT) program-map.ld
	$(LD) -M -T $(LDSCRIPT) $(OBJS) $(POBJS) $(LIBGCC) -o $@ > $(IMGNAME).map

program-map.ld: buildprograms
	$(NM) $(POBJS) | egrep "_binary_.*_start" | awk -- '{ printf "LONG(%s);\n", $$3 }' > $@

.PHONY: buildprograms userlib
buildprograms: userlib
	for p in $(PROGS); do make -C $$p USE_VMM=$(USE_VMM); done
userlib:
	make -C userlib USE_VMM=$(USE_VMM)

%.o: %.S
	$(CC) $(SFLAGS) -c $< -o $@

%_dats.o: %.dats
	$(ATSOPT) $(ATSFLAGS) --output $(patsubst %.dats,%_dats.c,$<) --dynamic $<
	$(CC) $(ATSCFLAGS) $(CFLAGS) -o $@ -c $(patsubst %.dats,%_dats.c,$<)

.PHONY: clean test tags cscope

clean:
	rm -f $(IMGNAME).uimg $(IMGNAME).elf $(IMGNAME).bin $(IMGNAME).map $(IMGNAME).bin.gz ucmd ukermit program-map.ld
	rm -f $(OBJS) $(DFILES) $(ATSCTEMPS)
	for p in $(PROGS); do make -C $$p clean; done
	make -C userlib clean

distclean: clean
	rm -f $(IMGNAME)-nand.bin
	rm -f cscope.files cscope.in.out cscope.po.out cscope.out TAGS tags
	(cd util; make clean)
	(cd omap-u-boot-utils; make distclean V=1)
	for p in $(PROGS); do make -C $$p distclean; done
	make -C userlib distclean

test: $(IMGNAME)-nand.bin
	@echo "***** To quit QEMU type: Ctrl-a x"
	$(QEMU) -M beagle -m 128M -nographic -mtdblock $<

tags:
	find . -name '*.[ch]' | xargs etags -o TAGS 
	find . -name '*.[ch]' | xargs ctags -o tags

cscope: cscope.files
	cscope -b -q -k

cscope.files:
	find . -name '*.[ch]' > $@

util/bb_nandflash_ecc: util/bb_nandflash_ecc.c
	(cd util; make bb_nandflash_ecc)

.PHONY: ucmd ukermit
ucmd:
	(cd omap-u-boot-utils; make ucmd V=1)
	cp omap-u-boot-utils/ucmd .

ukermit:
	(cd omap-u-boot-utils; make ukermit V=1)
	cp omap-u-boot-utils/ukermit .

-include $(DFILES)

# vi: set noexpandtab sts=8 sw=8:
