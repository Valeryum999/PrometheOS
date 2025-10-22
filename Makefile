# If any overriding is necessary, you should do `make CC=i386-elf-gcc`, NOT `CC=i386-elf-gcc make`

.PHONY: qemu, clean

TARGET_ARCH = i386
DESTDIR = out
CC = clang --target=$(TARGET_ARCH)-unknown-none-elf
LD = ld.lld
AR = llvm-ar
GRUB = grub

override LDFLAGS += -T $(ARCHDIR)/linker.ld
override CFLAGS += -Iinclude -Ilib/include -std=gnu11 -ffreestanding

ARCHDIR = arch/$(TARGET_ARCH)
include $(ARCHDIR)/arch.mk

OBJS = \
	$(ARCH_OBJS) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard kernel/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard kernel/fs/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard lib/ctype/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard lib/stdio/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard lib/stdlib/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard lib/string/*.c)) \

qemu: $(DESTDIR)/prometheos.iso
	qemu-system-$(TARGET_ARCH) -cdrom $<

clean:
	rm -rf $(DESTDIR)

$(DESTDIR)/prometheos.iso: $(DESTDIR)/prometheos.kernel
	mkdir -p $(DESTDIR)/iso/boot/grub
	cp $(DESTDIR)/prometheos.kernel $(DESTDIR)/iso/boot/
	cp ../fat12.img $(DESTDIR)/iso/boot/prometheos.initrd
	cp grub.cfg $(DESTDIR)/iso/boot/grub/grub.cfg

	$(GRUB)-mkrescue -o $(DESTDIR)/prometheos.iso $(DESTDIR)/iso

$(DESTDIR)/prometheos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(GRUB)-file --is-x86-multiboot $@

$(DESTDIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(DESTDIR)/%.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@
