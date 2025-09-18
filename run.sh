if grub-file --is-x86-multiboot bin/prometheos.bin; then
  echo multiboot confirmed
else
  echo the file is not multiboot
fi

mkdir -p isodir/boot/grub
cp bin/prometheos.bin isodir/boot/prometheos.bin
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o prometheos.iso isodir

qemu-system-i386 -cdrom prometheos.iso