CORE = OMAP4460
OPT = 2
DEBUG = 0

ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
NM = $(ARCH)-nm
ATSCC = patscc
ATSOPT = patsopt
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage
QEMU = qemu-system-arm
SFLAGS = -MMD -march=armv7-a -ffreestanding
CFLAGS = -Wall -MMD -march=armv7-a -mno-unaligned-access -ffreestanding

ifndef ATSHOME
ifdef PATSHOME
ATSHOME = $(PATSHOME)
else
ATSHOME := $(shell ls -1td /usr/lib/ats2-postiats-*/|head -1)
endif
endif

KERNEL_ATSFLAGS += -IATS include
KERNEL_ATSCFLAGS += -Iinclude -D_ATS_EXCEPTION_NONE -D_ATS_CCOMP_PRELUDE_NONE -D_ATS_CCOMP_HEADER_NONE -I$(ATSHOME) -Wno-unused-function -Wno-unused-label -Wno-unused-but-set-variable

USER_CFLAGS = -I../../userlib/include -mno-unaligned-access -ffreestanding
USER_ATSFLAGS = -IATS ../../userlib/include
USER_ATSCFLAGS += -D_ATS_EXCEPTION_NONE -D_ATS_CCOMP_PRELUDE_NONE -D_ATS_CCOMP_HEADER_NONE -I$(ATSHOME) -Wno-unused-function -Wno-unused-label -Wno-unused-but-set-variable
USER_LDFLAGS = -L../../userlib
USER_LIBS = `$(CC) -print-libgcc-file-name`
ifeq ($(USE_VMM),1)
USER_CFLAGS += -DUSE_VMM
USER_SFLAGS += -DUSE_VMM
endif
