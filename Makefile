# Nuke built-in rules.
.SUFFIXES:

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := prometheos.bin

# User controllable toolchain and toolchain prefix.
TOOLCHAIN := i686-elf
TOOLCHAIN_PREFIX := 
ifneq ($(TOOLCHAIN),)
    ifeq ($(TOOLCHAIN_PREFIX),)
        TOOLCHAIN_PREFIX := $(TOOLCHAIN)-
    endif
endif

# User controllable C compiler command.
ifneq ($(TOOLCHAIN_PREFIX),)
    CC := $(TOOLCHAIN_PREFIX)gcc
else
    CC := cc
endif

# User controllable linker command.
LD := $(TOOLCHAIN_PREFIX)ld

# Defaults overrides for variables if using "llvm" as toolchain.
ifeq ($(TOOLCHAIN),llvm)
    CC := clang
    LD := ld.lld
endif

# User controllable C flags.
CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra

# User controllable C preprocessor flags. We set none by default.
CPPFLAGS :=

# User controllable linker flags. We set none by default.
LDFLAGS := -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep -q '^Target: '; echo $$?)

# If the C compiler is Clang, set the target as needed.
ifeq ($(CC_IS_CLANG),1)
    override CC += \
        -target x86_64-unknown-none-elf
endif

# Use "find" to glob all *.c, *.s files in the tree and obtain the
# object and header dependency file names.
override SRCFILES := $(shell find -L src -type f 2>/dev/null | LC_ALL=C sort)
override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.s,$(SRCFILES))
override OBJ := $(addprefix obj/,$(CFILES:.c=.o) $(ASFILES:.s=.o))

# Default target. This must come first, before header dependencies.
.PHONY: all
all: bin/$(OUTPUT)

# Link rules for the final executable.
bin/$(OUTPUT): Makefile linker.ld $(OBJ)
	mkdir -p "$(dir $@)"
	$(CC) $(LDFLAGS) $(OBJ) -o $@

# Compilation rules for *.c files.
obj/%.o: %.c Makefile
	mkdir -p "$(dir $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
obj/%.o: %.s Makefile
	mkdir -p "$(dir $@)"
	i686-elf-as $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf bin obj