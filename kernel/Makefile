DEFAULT_HOST!=../scripts/default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../scripts/target-triplet-to-arch.sh $(HOST)

CFLAGS?=-Og -g
CPPFLAGS?=
LDFLAGS?=
LIBS?=

OBJDIR?=../out
DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(HOSTARCH)
OBJDIR=../obj

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
kernel/kernel.o \
kernel/pager.o \
kernel/gdt.o \
kernel/isrs_gen.o \
kernel/idt.o \
kernel/isr.o \
kernel/page_frame_allocator.o \
kernel/fs/disk.o \
kernel/fs/fat.o \

TEMPOBJS = \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o
OBJS=$(patsubst %.o,$(OBJDIR)/%.o,$(TEMPOBJS))

TEMPLINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/crtn.o
LINK_LIST=$(patsubst %.o,$(OBJDIR)/%.o,$(TEMPLINK_LIST))

.PHONY: all clean install install-headers install-kernel
.SUFFIXES:

all: prometheos.kernel

prometheos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot $@

$(OBJDIR)/%.o: %.S
	echo $@ "SESSO!"

$(OBJDIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

$(OBJDIR)/%.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

#$(OBJDIR)/%.o: arch/%.c
#	@mkdir -p $(shell dirname $@)
#	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)
#
#$(OBJDIR)/%.o: arch/%.S
#	@mkdir -p $(shell dirname $@)
#	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)


$(OBJDIR)/$(ARCHDIR)/crtbegin.o $(OBJDIR)/$(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

kernel/isrs_gen.c arch/i386/isrs_gen.inc &:
	python ../scripts/generate_isrs.py kernel/isrs_gen.c arch/i386/isrs_gen.S

clean:
	rm -f prometheos.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: prometheos.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp prometheos.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
