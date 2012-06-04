#!/bin/sh

[ -z "$1" -o -z "$2" -o -z "$3" ] && exit 1

IMGFILE="$1"
OUTPUT="$2"
CORE="$3"

sh util/bb_nandflash.sh u-boot/$CORE/x-load.bin.ift $OUTPUT x-loader
sh util/bb_nandflash.sh u-boot/$CORE/u-boot.bin $OUTPUT u-boot
sh util/bb_nandflash.sh $IMGFILE $OUTPUT kernel

# for linux:
# bb_nandflash.sh uImage $OUTPUT kernel
# bb_nandflash.sh rd-ext2-8M.bin $OUTPUT rootfs

util/bb_nandflash_ecc $OUTPUT 0x0 0xe80000
exit 0
