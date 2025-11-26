# If any overriding is necessary, you should do `make CC=i386-elf-gcc`, NOT `CC=i386-elf-gcc make`

.PHONY: qemu, kvm, debug, clean

TARGET_ARCH = i386
DESTDIR = obj
#CC = clang --target=$(TARGET_ARCH)-unknown-none-elf
CC = i686-prometheos-gcc
LD = $(CC) 
AR = i686-prometheos-ar
GRUB = grub

override LDFLAGS += -T $(ARCHDIR)/linker.ld -g -Og --sysroot=/home/valeryum/PrometheOS/sysroot -ffreestanding -nostdlib
override CFLAGS += -g -Og -std=gnu11 --sysroot=/home/valeryum/PrometheOS/sysroot -ffreestanding -Wall -Wextra -D__is_kernel -nostdlib -Iinclude
LIBK_FLAGS = -g -Og -std=gnu11 --sysroot=/home/valeryum/PrometheOS/sysroot -ffreestanding -Wall -Wextra -D__is_libk -isystem=/usr/include/libk -nostdlib -Iinclude
ARCHDIR = arch/$(TARGET_ARCH)
include $(ARCHDIR)/arch.mk

QEMU_FLAGS = -monitor stdio -chardev file,id=dbglog,path=debug.log -device isa-debugcon,iobase=0xE9,chardev=dbglog -cdrom

OBJS = \
	$(ARCH_OBJS) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard kernel/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.o,$(wildcard kernel/fs/*.c)) 
LIBK_OBJS = \
	$(patsubst %.c,$(DESTDIR)/%.libk.o,$(wildcard lib/ctype/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.libk.o,$(wildcard lib/stdio/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.libk.o,$(wildcard lib/stdlib/*.c)) \
	$(patsubst %.c,$(DESTDIR)/%.libk.o,$(wildcard lib/string/*.c)) \

qemu: $(DESTDIR)/prometheos.iso
	qemu-system-$(TARGET_ARCH) $(QEMU_FLAGS) $<

kvm: $(DESTDIR)/prometheos.iso
	qemu-system-$(TARGET_ARCH) --enable-kvm $(QEMU_FLAGS) $<

debug: $(DESTDIR)/prometheos.iso
	qemu-system-$(TARGET_ARCH) -s -S $(QEMU_FLAGS) $<

clean:
	rm -rf $(DESTDIR)

$(DESTDIR)/prometheos.iso: $(DESTDIR)/prometheos.kernel 
	mkdir -p $(DESTDIR)/iso/boot/grub
	cp $(DESTDIR)/prometheos.kernel $(DESTDIR)/iso/boot/
	cp ../cut_fat12.img $(DESTDIR)/iso/boot/prometheos.initrd
	cp grub.cfg $(DESTDIR)/iso/boot/grub/grub.cfg

	$(GRUB)-mkrescue -o $(DESTDIR)/prometheos.iso $(DESTDIR)/iso

$(DESTDIR)/prometheos.kernel: $(OBJS) $(ARCHDIR)/linker.ld sysroot/usr/lib/libk.a
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lk
	$(GRUB)-file --is-x86-multiboot $@

sysroot/usr/lib/libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)
	
$(DESTDIR)/lib/%.libk.o: lib/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(LIBK_FLAGS) -c $< -o $@

$(DESTDIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(DESTDIR)/%.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@
