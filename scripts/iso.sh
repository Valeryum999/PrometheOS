#!/bin/sh
set -e
. ./scripts/build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/prometheos.kernel isodir/boot/prometheos.kernel
cp ../fat12.img sysroot/boot/prometheos.initrd
cp sysroot/boot/prometheos.initrd isodir/boot/prometheos.initrd
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "prometheos" {
	multiboot /boot/prometheos.kernel
	module /boot/prometheos.initrd
}
EOF
grub-mkrescue -o prometheos.iso isodir
