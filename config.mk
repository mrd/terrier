ARCH = arm-none-linux-gnueabi
CC = $(ARCH)-gcc
LD = $(ARCH)-ld
AS = $(ARCH)-as
OBJCOPY = $(ARCH)-objcopy
MKIMAGE = mkimage
QEMU = qemu-system-arm
SFLAGS = -MMD -mcpu=cortex-a8
CFLAGS = -Wall -MMD -mcpu=cortex-a8
