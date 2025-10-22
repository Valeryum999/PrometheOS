# The only input should be $TARGET_ARCH and $DESTDIR

.SUFFIXES:

CFLAGS += -ffreestanding -Wall -Wextra -Iinclude
LIBK_CFLAGS := $(CFLAGS)

ARCHDIR=arch/$(TARGET_ARCH)

include $(ARCHDIR)/make.config

CFLAGS += $(ARCH_CFLAGS)
LIBK_CFLAGS += $(KERNEL_ARCH_CFLAGS)

FREEOBJS = \
	$(ARCH_FREEOBJS) \
	$(DESTDIR)/ctype/islower.o \
	$(DESTDIR)/ctype/toupper.o \
	$(DESTDIR)/stdio/printf.o \
	$(DESTDIR)/stdio/putchar.o \
	$(DESTDIR)/stdio/puts.o \
	$(DESTDIR)/stdlib/abort.o \
	$(DESTDIR)/string/itoa.o \
	$(DESTDIR)/string/memcmp.o \
	$(DESTDIR)/string/memcpy.o \
	$(DESTDIR)/string/memmove.o \
	$(DESTDIR)/string/memset.o \
	$(DESTDIR)/string/strchr.o \
	$(DESTDIR)/string/strcmp.o \
	$(DESTDIR)/string/strcpy.o \
	$(DESTDIR)/string/strlen.o \
	$(DESTDIR)/string/strrev.o \

HOSTEDOBJS= \
	$(ARCH_HOSTEDOBJS) \

OBJS= \
	$(FREEOBJS) \
	$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=$(DESTDIR)/libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S

all: $(BINARIES)

$(DESTDIR)/libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

$(DESTDIR)/libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

$(DESTDIR)/%.c.o:
	$(CC) -c $< -o $@ -std=gnu11 $(CFLAGS)

$(DESTDIR)/%.c.S:
	$(CC) -c $< -o $@ $(CFLAGS)

$(DESTDIR)/%.libk.o: %.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(LIBK_CFLAGS) -c $< -o $@ -std=gnu11

$(DESTDIR)/%.libk.o: %.S
	@mkdir -p $(shell dirname $@)
	$(CC) $(LIBK_CFLAGS) -c $< -o $@

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d
