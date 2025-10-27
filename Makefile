# If any overriding is necessary, you should do `make CC=i386-elf-gcc`, NOT `CC=i386-elf-gcc make`

.PHONY: qemu, clean

TARGET_ARCH = i386
DESTDIR = obj
#CC = clang --target=$(TARGET_ARCH)-unknown-none-elf
CC = i686-elf-gcc
LD = $(CC) 
AR = i686-elf-ar 
GRUB = grub

override LDFLAGS += -T $(ARCHDIR)/linker.ld -g -Og -ffreestanding --sysroot=/home/valeryum/Desktop/kernel/sysroot -isystem=/usr/include -Wall -Wextra -nostdlib -lk -lgcc
override CFLAGS += -g -Og -std=gnu11 -ffreestanding --sysroot=/home/valeryum/Desktop/kernel/sysroot -isystem=/usr/include -Wall -Wextra -D__is_kernel -Iinclude
LIBK_FLAGS = -D__is_libc -Iinclude -D__is_libk 
ARCHDIR = arch/$(TARGET_ARCH)
include $(ARCHDIR)/arch.mk

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
	qemu-system-$(TARGET_ARCH) -cdrom $<

clean:
	rm -rf $(DESTDIR)

$(DESTDIR)/prometheos.iso: $(DESTDIR)/prometheos.kernel 
	mkdir -p $(DESTDIR)/iso/boot/grub
	cp $(DESTDIR)/prometheos.kernel $(DESTDIR)/iso/boot/
	cp ../fat12.img $(DESTDIR)/iso/boot/prometheos.initrd
	cp grub.cfg $(DESTDIR)/iso/boot/grub/grub.cfg

	$(GRUB)-mkrescue -o $(DESTDIR)/prometheos.iso $(DESTDIR)/iso

$(DESTDIR)/prometheos.kernel: $(OBJS) $(ARCHDIR)/linker.ld libk.a
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	$(GRUB)-file --is-x86-multiboot $@

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)
	
$(DESTDIR)/lib/%.libk.o: lib/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $(LIBK_FLAGS) $< -o $@

$(DESTDIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(DESTDIR)/%.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@
