#!/bin/sh
set -e
. ./scripts/build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/prometheos.kernel isodir/boot/prometheos.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "prometheos" {
	multiboot /boot/prometheos.kernel
}
EOF
grub-mkrescue -o prometheos.iso isodir
