# If any overriding is necessary, you should do `make CC=i386-elf-gcc`, NOT `CC=i386-elf-gcc make`

TARGET_ARCH = i386
DESTDIR = out
CC = clang --target=$(TARGET_ARCH)-unknown-none-elf
LD = ld.lld
AR = llvm-ar
GRUB = grub
export TARGET_ARCH CC LD AR

.PHONY: qemu

qemu: $(DESTDIR)/prometheos.iso
	qemu-system-$(TARGET_ARCH) -cdrom $<

$(DESTDIR)/prometheos.iso: $(DESTDIR)/prometheos.kernel
	mkdir -p $(DESTDIR)/iso/boot/grub
	cp $(DESTDIR)/prometheos.kernel $(DESTDIR)/iso/boot/
	cp ../fat12.img $(DESTDIR)/iso/boot/prometheos.initrd
	cp grub.cfg $(DESTDIR)/iso/boot/grub/grub.cfg

	$(GRUB)-mkrescue -o $(DESTDIR)/prometheos.iso $(DESTDIR)/iso

$(DESTDIR)/prometheos.kernel: $(DESTDIR)/libk.a
	@mkdir -p $(DESTDIR)
	$(MAKE) -C kernel DESTDIR=../$(DESTDIR)
	$(GRUB)-file --is-x86-multiboot $@
$(DESTDIR)/libk.a:
	@mkdir -p $(DESTDIR)
	$(MAKE) -C libc DESTDIR=../$(DESTDIR)

clean:
	rm -rf $(DESTDIR)
