ROOTDIR ?= $(MARSDEV)

.DELETE_ON_ERROR:

LIBPATH = -L$(ROOTDIR)/sh-elf/lib -L$(ROOTDIR)/sh-elf/lib/gcc/sh-elf/4.6.2 -L$(ROOTDIR)/sh-elf/sh-elf/lib
INCPATH = -Isrc -Iinc -Ires -Ires_md -I$(ROOTDIR)/sh-elf/include -I$(ROOTDIR)/sh-elf/sh-elf/include

CCFLAGS = -m2 -mb -Wall -c -fomit-frame-pointer -fno-builtin -ffunction-sections -fdata-sections
CCFLAGS += -fno-align-loops -fno-align-functions -fno-align-jumps -fno-align-labels -funroll-loops -lto

HWFLAGS := $(CCFLAGS)
HWFLAGS += -O1 -fno-lto

LDFLAGS = -T ./mars.ld -nostdlib -Wl,--print-memory-usage -Wl,--gc-sections --specs=nosys.specs -flto
ASFLAGS = --big

EXTRA =

PREFIX = $(ROOTDIR)/sh-elf/bin/sh-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
OBJC = $(PREFIX)objcopy

M68K_PREFIX = $(ROOTDIR)/m68k-elf/bin/m68k-elf-
M68K_AS = $(M68K_PREFIX)as
M68K_OBJC = $(M68K_PREFIX)objcopy

DD = dd
RM = rm -f

GENESIS_ROOT ?= genesis
GENESIS_REPOSITORY = https://github.com/ArtemioUrbina/240pTestSuite.git
GENESIS_BRANCH = master
GENESIS_SOURCE = $(GENESIS_ROOT)/240psuite/Genesis/240p
GENESIS_ADAPTER = $(abspath tools/genesis-build/Makefile)
GENESIS_BIN = $(GENESIS_SOURCE)/out/genesis.bin
GENESIS_VECTORS = $(GENESIS_SOURCE)/out/genesis-vectors.bin

TARGET = build/240pMars
COMBINED_TARGET = build/240pMD32X
LIBS = $(LIBPATH) -lc -lgcc -lgcc-Os-4-200 -lnosys

M68K_BIN = src_md/m68k.bin
M68K_COMBINED_BIN = src_md/m68k-combined.bin
M68K_SOURCES = \
	$(wildcard src_md/*.c) \
	$(wildcard src_md/*.s) \
	$(wildcard liblzss/*.c)

OBJS = $(wildcard src/*.c)
SHSS = $(wildcard src/*.s)
SHCOMMONOBJS = sh2_fixed.o
SHCOMMONOBJS += $(OBJS:.c=.o)
SHCOMMONOBJS += $(SHSS:.s=.o)

.PHONY: all release debug combined genesis-source genesis-build clean

all: release

release: EXTRA = -Os
release: $(TARGET).32x

debug: EXTRA = -O0 -g -gdwarf-2
debug: $(TARGET).32x

combined: EXTRA = -Os
combined: genesis-build $(M68K_COMBINED_BIN)
	$(MAKE) EXTRA=-Os $(COMBINED_TARGET).bin

genesis-source:
	@if [ -e "$(GENESIS_ROOT)" ] && [ ! -d "$(GENESIS_ROOT)/.git" ]; then \
		echo "$(GENESIS_ROOT) exists but is not the managed upstream checkout"; \
		echo "Move or remove it, then run make combined again"; \
		exit 1; \
	fi
	@if [ ! -d "$(GENESIS_ROOT)/.git" ]; then \
		git clone --filter=blob:none --no-checkout --single-branch \
			--branch "$(GENESIS_BRANCH)" "$(GENESIS_REPOSITORY)" "$(GENESIS_ROOT)"; \
		git -C "$(GENESIS_ROOT)" sparse-checkout init --cone; \
		git -C "$(GENESIS_ROOT)" sparse-checkout set \
			240psuite/Genesis/240p/res \
			240psuite/Genesis/240p/src/boot; \
		git -C "$(GENESIS_ROOT)" checkout "$(GENESIS_BRANCH)"; \
	else \
		if [ -n "$$(git -C "$(GENESIS_ROOT)" status --porcelain --untracked-files=no)" ]; then \
			echo "The managed Genesis checkout has local tracked changes; refusing to overwrite them"; \
			exit 1; \
		fi; \
		git -C "$(GENESIS_ROOT)" sparse-checkout set \
			240psuite/Genesis/240p/res \
			240psuite/Genesis/240p/src/boot; \
		git -C "$(GENESIS_ROOT)" pull --ff-only origin "$(GENESIS_BRANCH)"; \
	fi

genesis-build: genesis-source
	$(MAKE) -C "$(GENESIS_SOURCE)" -f "$(GENESIS_ADAPTER)" release
	$(DD) if="$(GENESIS_BIN)" of="$(GENESIS_VECTORS)" bs=1 skip=8 count=248

$(M68K_BIN): $(M68K_SOURCES) src_md/Makefile src_md/mars-md.ld
	$(MAKE) -C src_md release

$(M68K_COMBINED_BIN): $(M68K_SOURCES) src_md/Makefile src_md/mars-md.ld
	$(MAKE) -C src_md combined

dual_boot.bin: dual_boot.s
	$(M68K_AS) $< -o dual_boot.o
	$(M68K_OBJC) -O binary dual_boot.o $@
	$(RM) dual_boot.o

$(TARGET).32x: $(TARGET).elf
	$(OBJC) -O binary $< $(TARGET).tmp
	$(DD) if=$(TARGET).tmp of=$@ bs=128K conv=sync
	$(RM) $(TARGET).tmp

$(COMBINED_TARGET).bin: $(COMBINED_TARGET).elf $(GENESIS_BIN) tools/fix_checksum.sh
	$(OBJC) -O binary $< $(COMBINED_TARGET).tmp
	@test "$$(stat -c %s $(COMBINED_TARGET).tmp)" -le 655360 || \
		{ echo "32X payload exceeds its 640 KiB ROM region"; exit 1; }
	$(DD) if=$(COMBINED_TARGET).tmp of=$@ bs=128K conv=sync
	truncate --size=655360 $@
	$(DD) if=$(GENESIS_BIN) of=$@ bs=128K seek=5 conv=notrunc
	sh tools/fix_checksum.sh $@
	$(RM) $(COMBINED_TARGET).tmp

$(TARGET).elf: crt0.o $(SHCOMMONOBJS)
	$(CC) $(LDFLAGS) -Wl,-Map=$(TARGET).map crt0.o $(SHCOMMONOBJS) $(LIBS) -o $@

$(COMBINED_TARGET).elf: crt0_combined.o $(SHCOMMONOBJS)
	$(CC) $(LDFLAGS) -Wl,-Map=$(COMBINED_TARGET).map crt0_combined.o $(SHCOMMONOBJS) $(LIBS) -o $@

crt0.o: crt0.s $(M68K_BIN)
	$(AS) $(ASFLAGS) $(INCPATH) $< -o $@

crt0_combined.o: crt0.s $(M68K_COMBINED_BIN) dual_boot.bin $(GENESIS_VECTORS)
	$(AS) $(ASFLAGS) --defsym COMBINED_ROM=1 $(INCPATH) $< -o $@

src/hw_32x.o: src/hw_32x.c
	$(CC) $(HWFLAGS) $(INCPATH) $< -o $@

src/%.o: src/%.c
	$(CC) $(CCFLAGS) $(EXTRA) $(INCPATH) $< -o $@

src/%.o: src/%.s
	$(AS) $(ASFLAGS) $(INCPATH) $< -o $@

clean:
	$(MAKE) clean -C src_md
	@if [ -d "$(GENESIS_SOURCE)" ]; then \
		$(MAKE) -C "$(GENESIS_SOURCE)" -f "$(GENESIS_ADAPTER)" clean; \
	fi
	$(RM) *.o dual_boot.bin output.map
	$(RM) $(TARGET).32x $(TARGET).elf $(TARGET).map $(TARGET).tmp
	$(RM) $(COMBINED_TARGET).bin $(COMBINED_TARGET).elf $(COMBINED_TARGET).map $(COMBINED_TARGET).tmp
	$(RM) src/*.o
