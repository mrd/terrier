ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage

ADDRESS = 0x80008000

IMGNAME = puppy

SFLAGS =
CFLAGS =

$(IMGNAME).uimg: $(IMGNAME).bin
	$(MKIMAGE) -A arm -C none -O linux -T kernel -d $< -a $(ADDRESS) -e $(ADDRESS) $@

$(IMGNAME)-nand.bin: $(IMGNAME).uimg u-boot/u-boot.bin util/bb_nandflash_ecc
	sh util/nand.sh $< $@ 2> /dev/null

$(IMGNAME).bin: $(IMGNAME).elf
	$(OBJCOPY) -O binary $< $@

$(IMGNAME).elf: init/init.o init/startup.o ldscripts/$(IMGNAME).ld
	$(LD) -T ldscripts/$(IMGNAME).ld init/init.o init/startup.o -o $@

%.o: %.S
	$(AS) $(SFLAGS) $< -o $@

.PHONY: clean test

clean:
	rm -f $(IMGNAME).uimg $(IMGNAME).elf $(IMGNAME).bin init/startup.o init/init.o
	(cd util; make clean)

distclean: clean
	rm -f $(IMGNAME)-nand.bin

test: $(IMGNAME)-nand.bin
	@echo "***** To quit QEMU type: Ctrl-a x"
	qemu-system-arm -M beagle -m 128M -nographic -mtdblock $<

util/bb_nandflash_ecc: util/bb_nandflash_ecc.c
	(cd util; make bb_nandflash_ecc)

init/startup.o: init/startup.S
init/init.o: init/init.c
