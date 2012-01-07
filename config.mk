ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage
QEMU = qemu-system-arm
SFLAGS = -MMD
CFLAGS = -Wall -MMD


