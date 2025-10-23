#!/bin/sh
set -e
if [ $1 == "DEBUG" ]; then
    qemu-system-i386 -s -S -cdrom prometheos.iso
else
    . ./scripts/iso.sh
    qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -cdrom prometheos.iso
fi
#-s -S
