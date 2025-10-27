#!/bin/sh
set -e
. ./scripts/iso.sh
if [ $1 == "DEBUG" ]; then
    qemu-system-i386 -s -S -cdrom prometheos.iso
else
    qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -cdrom prometheos.iso
fi
#-s -S
