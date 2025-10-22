ARCHDIR = arch/$(TARGET_ARCH)
DESTDIR = out

include $(ARCHDIR)/arch.mk

override LDFLAGS += -T $(ARCHDIR)/linker.ld
override CFLAGS += -Iinclude -std=gnu11 -I../libc/include -ffreestanding

OBJS += \
	$(DESTDIR)/kernel/kernel.o \
	$(DESTDIR)/kernel/pager.o \
	$(DESTDIR)/kernel/gdt.o \
	$(DESTDIR)/kernel/isrs_gen.o \
	$(DESTDIR)/kernel/idt.o \
	$(DESTDIR)/kernel/isr.o \
	$(DESTDIR)/kernel/io.o \
	$(DESTDIR)/kernel/irq.o \
	$(DESTDIR)/kernel/keyboard.o \
	$(DESTDIR)/kernel/cursor.o \
	$(DESTDIR)/kernel/page_frame_allocator.o \
	$(DESTDIR)/kernel/pic.o \
	$(DESTDIR)/kernel/fs/disk.o \
	$(DESTDIR)/kernel/fs/fat.o \
	$(DESTDIR)/kernel/elf32.o \
	$(DESTDIR)/kernel/process.o \
	$(DESTDIR)/kernel/scheduler.o \
	$(ARCH_OBJS) \

.PHONY: clean

$(DESTDIR)/prometheos.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(DESTDIR)/libk.a

$(DESTDIR)/%.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(DESTDIR)/%.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(DESTDIR)
