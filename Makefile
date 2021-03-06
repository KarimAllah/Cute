#
# Cute Makefile
# (C) 2009-2012 Ahmed S. Darwish <darwish.07@gmail.com>
#

CC	= gcc
CPP	= cpp
LD	= ld
RM = rm
LN = ln
CD = cd
GZIP = gzip
OBJCOPY = objcopy

EMULATOR = /home/karim/sources/qemu/qemu/build/x86_64-softmmu/qemu-system-x86_64
EMULATOR_OPTIONS = -smp 1 -display sdl -vga std -serial stdio -no-reboot

#
# Machine-dependent C Flags:
#
# Use the AMD64 'kernel' code model for reasons stated in our
# head.S bootstrap code.
#
# Disable SSE floating point ops. They need special CPU state
# setup, or several #UD and #NM exceptions will be triggered.
#
# Do not use the AMD64 ABI 'red zone'. This was a bug that
# costed me an entire week to fix!
#
# Basically the zone is 128-byte area _below_ the stack pointer
# that can be used for temporary local state, especially for
# leaf functions.
#
# Ofcouse this red zone is disastrous to be used in the kernel
# since it's where the CPU pushes %ss, %rsp, %rflags, %cs and
# %rip on the event of invoking an interrupt handler. If used,
# it will simply lead to kernel stack corruption.
#
# Check attached docs for extra info on -mcmodel and the zone.
#
CMACH_FLAGS =				\
  -m64					\
  -mcmodel=kernel			\
  -mno-mmx				\
  -mno-sse				\
  -mno-sse2				\
  -mno-sse3				\
  -mno-3dnow				\
  -mno-red-zone

#
# GCC C dialect flags:
#
# We're a freestanding environment by ISO C99 definition:
# - Code get executed without benefit of an OS (it's a kernel).
# - Program startup and termination is implementation-defined.
# - Any library facilities outside C99 'strict conformance'
#   options are also implementation defined.
#
# Poking GCC with the 'freestanding' flag assures it won't
# presume availability of any 'hosted' facilities like libc.
#
# After using -nostdinc, we add compiler's specific includes
# back (stdarg.h, etc) using the -iwithprefix flag.
#
CDIALECT_FLAGS =			\
  -std=gnu99				\
  -ffreestanding			\
  -fno-builtin				\
  -nostdlib				\
  -nostdinc				\
  -iwithprefix include			\
  -I include/

#
# C Optimization flags:
#
# Use -O3 to catch any weird bugs early on
#
# Note-1! Fallback to -O2 at official releases
# Note-2! Shouldn't we disable strict aliasing?
#
COPT_FLAGS =				\
  -O3					\
  -g					\
  -pipe

#
# Warnings request and dismissal flags:
#
# - We've previously caught 2 bugs causeed by an implicit cast
# to a smaller-width type: carefully inspect warnings reported
# by the '-Wconversion' flag.
#
# - We may like to warn about aggregate returns cause we don't
# want to explode the stack if the structure type returned got
# _innocently_ bigger over time. Check '-Waggregate-return'.
#
# Options are printed in GCC HTML documentation order.
#
CWARN_FLAGS =				\
  -Wall					\
  -Wextra				\
  -Wchar-subscripts			\
  -Wformat=2				\
  -Wmissing-include-dirs		\
  -Wparentheses				\
  -Wtrigraphs				\
  -Wunused				\
  -Wundef				\
  -Wpointer-arith			\
  -Wcast-qual				\
  -Wwrite-strings			\
  -Waddress				\
  -Wlogical-op				\
  -Wstrict-prototypes			\
  -Wmissing-prototypes			\
  -Wmissing-declarations		\
  -Wmissing-noreturn			\
  -Wnormalized=nfc			\
  -Wredundant-decls			\
  -Wvla					\
  -Wdisabled-optimization		\
  -Wno-type-limits			\
  -Wno-missing-field-initializers

ifeq ($(ARCH),ia64)
CARCH_FLAGS	= -D__ARCH_IA64__
endif

CFLAGS =				\
  $(CMACH_FLAGS)			\
  $(CDIALECT_FLAGS)			\
  $(COPT_FLAGS)				\
  $(CARCH_FLAGS)			\
  $(CWARN_FLAGS)

# Share headers between assembly, C, and LD files
CPPFLAGS = -D__KERNEL__
AFLAGS = -D__ASSEMBLY__

# Warn about the sloppy UNIX linkers practice of
# merging global common variables
LDFLAGS = --warn-common

# Our global kernel linker script, after being
# 'cpp' pre-processed from the *.ld source
PROCESSED_LD_SCRIPT = arch/$(ARCH)/kernel.ldp

# GCC-generated C code header files dependencies
# Check '-MM' and '-MT' at gcc(1)
DEPS_ROOT_DIR = .deps
DEPS_DIRS    += $(DEPS_ROOT_DIR)

# 'Sparse' compiler wrapper
CGCC	= cgcc

#
# Object files listings
#

# Core and Secondary CPUs bootstrap
ifeq ($(ARCH),ia64)
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/arch/ia64
ARCH_OBJS =		\
  arch/ia64/head.o		\
  arch/ia64/trampoline.o	\
  arch/ia64/rmcall.o		\
  arch/ia64/e820.o		\
  arch/ia64/_e820.o		\
  arch/ia64/mptables.o	\
  arch/ia64/sched.o		\
  arch/ia64/load_ramdisk.o \
  arch/ia64/init.o \
  arch/ia64/memory.o \
  arch/ia64/smpboot.o \
  arch/ia64/idt.o \
  arch/ia64/_idt.o \
  arch/ia64/percpu.o \
  arch/ia64/syscall.o \
  arch/ia64/thread.o
endif

# Memory management
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/mm
MM_OBJS =		\
  mm/memory.o	\
  mm/page_alloc.o	\
  mm/vm_map.o		\
  mm/kmalloc.o
  

# Devices
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/dev
ifeq ($(ARCH),ia64)
DEV_OBJS =		\
  dev/serial.o		\
  dev/pic.o		\
  dev/apic.o		\
  dev/ioapic.o		\
  dev/pit.o		\
  dev/keyboard.o \
  dev/vga.o
endif

# Ext2 file system
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/ext2
EXT2_OBJS =		\
  ext2/ext2.o		\
  ext2/ext2_debug.o	\
  ext2/file.o		\
  ext2/file_tests.o	\
  ext2/files_list.o

# Isolated library code
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/lib
LIB_OBJS =		\
  lib/list.o		\
  lib/unrolled_list.o	\
  lib/hash.o		\
  lib/bitmap.o		\
  lib/string.o		\
  lib/printf.o		\
  lib/atomic.o		\
  lib/spinlock.o

# All other kernel objects
DEPS_DIRS		+= $(DEPS_ROOT_DIR)/kern
KERN_OBJS =		\
  $(ARCH_OBJS)		\
  $(MM_OBJS)		\
  $(DEV_OBJS)		\
  $(EXT2_OBJS)		\
  $(LIB_OBJS)		\
  kern/sched.o		\
  kern/syscall.o	\
  kern/kthread.o	\
  kern/panic.o		\
  kern/percpu.o		\
  kern/ramdisk.o	\
  kern/binder.o		\
  kern/paging.o		\
  kern/wait.o		\
  kern/proc.o		\
  kern/init_array.o	\
  kern/main.o

BOOTSECT_OBJS =		\
  arch/ia64/bootsect.o

BUILD_OBJS    =		$(BOOTSECT_OBJS) $(KERN_OBJS)
BUILD_DIRS    =		$(DEPS_DIRS)
	
# Control output verbosity
# `@': suppresses echoing of subsequent command
VERBOSE=0
ifeq ($(VERBOSE), 0)
	E = @echo
	Q = @
else
	E = @\#
	Q =
endif

all: final

# Check kernel code against common semantic C errors
# using the 'sparse' semantic parser
#
# REAL_CC: do not depend on the system-wide CC envi-
# ronment variable, use our chosen compiler instead.
.PHONY: check
check: clean
	$(E) "  SPARSE build"
	$(Q) $(MAKE) REAL_CC=$(CC) CC=$(CGCC) all

#
# Build final, self-contained, bootable disk image
#

BOOT_BIN       = kern/image
RAMDISK_BIN    = build/ramdisk
FINAL_HD_IMAGE = build/hd-image
BUILD_SCRIPT   = tools/build-hdimage.py

.PHONY: user

CLIB_INCLUDE = -Iuser/libc/include -Iinclude/uapi
CLIB_OBJECTS = user/libc/exit.o user/libc/syscall.o user/libc/string.o user/libc/printf.o user/libc/binder.o user/libc/malloc.o user/libc/ds/trees.o user/libc/ds/lists.o

USER_FLAGS = -O0 -nostdinc -nostdlib -iwithprefix include -std=gnu99 -fno-builtin

clib:
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/exit.c -o user/libc/exit.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/syscall.c -o user/libc/syscall.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/printf.c -o user/libc/printf.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/string.c -o user/libc/string.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/binder.c -o user/libc/binder.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/malloc.c -o user/libc/malloc.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/ds/trees.c -o user/libc/ds/trees.o
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/libc/ds/lists.c -o user/libc/ds/lists.o

user: clib
	# dummy_proc
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/dummy_proc.c -o user/tmp.o
	$(Q) $(LD) -T user/user.ld user/tmp.o $(CLIB_OBJECTS) -o user/dummy_proc.o
	$(Q) $(OBJCOPY) -O binary user/dummy_proc.o user/dummy_proc
	# looper
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/looper.c -o user/tmp.o
	$(Q) $(LD) -T user/user.ld user/tmp.o $(CLIB_OBJECTS) -o user/looper.o
	$(Q) $(OBJCOPY) -O binary user/looper.o user/looper
	# vga_worker
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/vga_worker.c -o user/tmp.o
	$(Q) $(LD) -T user/user.ld user/tmp.o $(CLIB_OBJECTS) -o user/vga_worker.o
	$(Q) $(OBJCOPY) -O binary user/vga_worker.o user/vga_worker
	# algorithms
	$(Q) $(CC) $(CLIB_INCLUDE) $(USER_FLAGS) -c user/algorithms.c -o user/tmp.o
	$(Q) $(LD) -T user/user.ld user/tmp.o $(CLIB_OBJECTS) -o user/algorithms.o
	$(Q) $(OBJCOPY) -O binary user/algorithms.o user/algorithms


final: $(BUILD_DIRS) $(FINAL_HD_IMAGE)
	$(E) "Disk image ready:" $(FINAL_HD_IMAGE)

$(FINAL_HD_IMAGE): $(BOOT_BIN) $(RAMDISK_BIN) $(BUILD_SCRIPT)
	$(E) "  PYTHON3  " $@
	$(Q) python $(BUILD_SCRIPT)

# If no ramdisk image exist, create an empty placeholder.
# Also create the 'build/' directory if it doesn't exist.
$(RAMDISK_BIN): user
	$(Q) test -d 'build' || (echo "  MKDIR     build")
	$(Q) mkdir -p build
	$(Q) dd if=/dev/zero of=$(RAMDISK_BIN) count=1 bs=100K
	$(Q) mkfs.ext2 -F build/ramdisk
	$(Q) sudo mount -o loop $(RAMDISK_BIN) /mnt
	$(Q) cp user/dummy_proc /mnt/init
	$(Q) cp user/looper /mnt/looper
	$(Q) cp user/vga_worker /mnt/vga_worker
	$(Q) cp user/algorithms /mnt/algorithms
	$(Q) sudo umount /mnt


#
# Build kernel + bootsector ELF and binaries
#

BOOTSECT_ELF   = arch/ia64/bootsect.elf
BOOTSECT_BIN   = arch/ia64/bootsect.bin
KERNEL_ELF     = kern/kernel.elf
KERNEL_BIN     = kern/kernel.bin

$(BOOT_BIN): $(BOOTSECT_ELF) $(KERNEL_ELF)
	$(E) "  OBJCOPY  " $@
	$(Q) objcopy -O binary $(BOOTSECT_ELF) $(BOOTSECT_BIN)
	$(Q) objcopy -O binary $(KERNEL_ELF) $(KERNEL_BIN)
	$(Q) cat $(BOOTSECT_BIN) $(KERNEL_BIN) > $@

$(BOOTSECT_ELF): $(BOOTSECT_OBJS)
	$(E) "  LD       " $@
	$(Q) $(LD) $(LDFLAGS) -Ttext 0x0  $< -o $@

include_dir:
	$(Q) $(RM) -rf include/arch
	$(Q) $(CD) include;$(LN) -s ../arch/$(ARCH)/include arch

$(KERNEL_ELF): include_dir $(BUILD_DIRS) $(KERN_OBJS) $(PROCESSED_LD_SCRIPT)
	$(E) "  LD       " $@
	$(Q) $(LD) $(LDFLAGS) -T $(PROCESSED_LD_SCRIPT) $(KERN_OBJS) -o $@

# Patterns for custom implicit rules
%.o: %.S
	$(E) "  AS       " $@
	$(Q) $(CC) -c $(AFLAGS) $(CFLAGS) $< -o $@
	$(Q) $(CC) -MM $(AFLAGS) $(CFLAGS) $< -o $(DEPS_ROOT_DIR)/$*.d -MT $@
%.o: %.c
	$(E) "  CC       " $@
	$(Q) $(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	$(Q) $(CC) -MM $(CPPFLAGS) $(CFLAGS) $< -o $(DEPS_ROOT_DIR)/$*.d -MT $@
%.ldp: %.ld
	$(E) "  CPP      " $@
	$(Q) $(CPP) $(AFLAGS) $(CFLAGS) -P $< -O $@
	$(Q) $(CPP) -MM $(AFLAGS) $(CFLAGS) $< -o $(DEPS_ROOT_DIR)/$*.d -MT $@

# Needed build directories
$(BUILD_DIRS):
	$(E) "  MKDIR    " $@
	$(Q) mkdir -p $@

.PHONY: clean
clean:
	$(E) "  CLEAN"
	$(Q) rm -fr $(BUILD_DIRS)
	$(Q) rm -f  $(BUILD_OBJS)
	$(Q) rm -f  $(ARCH_OBJS)
	$(Q) rm -f  $(PROCESSED_LD_SCRIPT)
	$(Q) rm -f  $(BOOTSECT_ELF) $(BOOTSECT_BIN)
	$(Q) rm -f  $(KERNEL_ELF) $(KERNEL_BIN)
	$(Q) rm -f  $(BOOT_BIN) $(FINAL_HD_IMAGE)
	$(Q) $(RM) -fr include/arch
	$(Q) rm -f	user/dummy_proc.o user/dummy_proc user/looper.o user/looper user/vga_worker.o user/vga_worker user/algorithms.o user/algorithms
	$(Q) rm -f	user/libc/libc.o user/libc/binder.o user/libc/malloc.o user/libc/exit.o user/libc/ds/trees.o user/libc/ds/lists.o
	$(Q) rm -f	build/ramdisk
	$(Q) rm -fr build
	$(Q) rm -f	cute.gz cute

run: final user $(RAMDISK_BIN)
	$(Q) $(EMULATOR) $(EMULATOR_OPTIONS) $(FINAL_HD_IMAGE)

debug: final user $(RAMDISK_BIN)
	gdb $(EMULATOR)

# Include generated dependency files
# `-': no error, not even a warning, if any of the given
# filenames do not exist
-include $(DEPS_ROOT_DIR)/*.d
-include $(DEPS_ROOT_DIR)/*/*.d
