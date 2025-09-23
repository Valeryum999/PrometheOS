#!/bin/sh
set -e
. ./iso.sh

#-s -S
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom prometheos.iso
