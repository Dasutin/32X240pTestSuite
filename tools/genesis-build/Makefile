ROOTDIR ?= $(MARSDEV)
SGDK ?= /opt/sgdk

ADAPTER_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
LINKER_SCRIPT := $(ADAPTER_DIR)/genesis.ld
RESOURCE_ADAPTER := $(ADAPTER_DIR)/resources.s

PREFIX = $(ROOTDIR)/m68k-elf/bin/m68k-elf-
CC = $(PREFIX)gcc
LD = $(PREFIX)ld
NM = $(PREFIX)nm
OBJC = $(PREFIX)objcopy

BUILD = out
TARGET = $(BUILD)/genesis
ROM_SIZE = 262144

INCPATH = -I. -Ires -I$(SGDK)/inc -I$(SGDK)/res
CFLAGS = -m68000 -Wall -fno-builtin -O1 -fomit-frame-pointer $(INCPATH)
LDFLAGS = -B$(ROOTDIR)/m68k-elf/bin -n -T $(LINKER_SCRIPT) -nostdlib \
	-Wl,-Map=$(TARGET).map
LIBS = $(SGDK)/lib/libmd.a $(SGDK)/lib/libgcc.a

ROOT_C = $(wildcard *.c)
ROOT_S = $(wildcard *.s)
RES_S = $(wildcard res/*.s)
OBJS = \
	$(patsubst %.c,$(BUILD)/%.o,$(ROOT_C)) \
	$(patsubst %.s,$(BUILD)/%.o,$(ROOT_S)) \
	$(patsubst res/%.s,$(BUILD)/res/%.o,$(RES_S)) \
	$(BUILD)/res/resources.o
BOOT_OBJ = $(BUILD)/src/boot/sega.o

.DELETE_ON_ERROR:
.PHONY: all release clean

all: release

release: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJC) -O binary $< $(TARGET).tmp
	@test "$$(stat -c %s $(TARGET).tmp)" -le $(ROM_SIZE) || \
		{ echo "Genesis payload exceeds its 256 KiB ROM region"; exit 1; }
	dd if=$(TARGET).tmp of=$@ bs=256K conv=sync
	rm -f $(TARGET).tmp

$(TARGET).elf: $(BOOT_OBJ) $(OBJS) $(LINKER_SCRIPT)
	$(CC) $(LDFLAGS) $(BOOT_OBJ) $(OBJS) $(LIBS) -o $@
	$(NM) -n -S $@ > $(TARGET).symbols

$(BOOT_OBJ): src/boot/sega.s $(BUILD)/rom_head.bin
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/rom_head.bin: $(BUILD)/rom_head.o $(LINKER_SCRIPT)
	$(LD) -T $(LINKER_SCRIPT) -nostdlib --oformat binary -o $@ $<

$(BUILD)/rom_head.o: src/boot/rom_head.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/res/resources.o: $(RESOURCE_ADAPTER)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
