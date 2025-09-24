#!/bin/sh
set -e
. ./scripts/iso.sh

#-s -S
qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -cdrom prometheos.iso
