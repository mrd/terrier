CORE = OMAP4460
OPT = 2
DEBUG = 0

ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
NM = $(ARCH)-nm
ATSCC = atscc
ATSOPT = atsopt
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage
QEMU = qemu-system-arm
SFLAGS = -MMD -march=armv7-a
CFLAGS = -Wall -MMD -march=armv7-a -mno-unaligned-access
ATSFLAGS =
USER_CFLAGS = -I../../userlib/include -mno-unaligned-access
USER_LDFLAGS = -L../../userlib
