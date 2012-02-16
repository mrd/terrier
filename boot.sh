#!/bin/sh

SERIAL=/dev/ttyS0

IMAGE=terrier.uimg
LOADADDR=82000000
GOCMD=bootm
EXPECT="-- HALTED --"

./ucmd    -p $SERIAL -c '' -e '#'
./ucmd    -p $SERIAL -c " loadb $LOADADDR 115200" -e 'bps...'
./ukermit -p $SERIAL -f $IMAGE
./ucmd    -p $SERIAL -c "$GOCMD $LOADADDR" -e "$EXPECT"
