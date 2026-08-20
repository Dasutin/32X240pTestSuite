ROOTDIR = $(MARSDEV)

LIBPATH = -L$(ROOTDIR)/sh-elf/lib -L$(ROOTDIR)/sh-elf/lib/gcc/sh-elf/4.6.2 -L$(ROOTDIR)/sh-elf/sh-elf/lib
INCPATH = -Isrc -Iinc -Ires -Ires_md -I$(ROOTDIR)/sh-elf/include -I$(ROOTDIR)/sh-elf/sh-elf/include

CCFLAGS = -m2 -mb -Wall -c -fomit-frame-pointer -fno-builtin  -ffunction-sections -fdata-sections
CCFLAGS += -fno-align-loops -fno-align-functions -fno-align-jumps -fno-align-labels -funroll-loops -lto

HWFLAGS := $(CCFLAGS)
HWFLAGS += -O1 -fno-lto

LDFLAGS = -T ./mars.ld -Wl,-Map=output.map -nostdlib -Wl,--print-memory-usage -Wl,--gc-sections --specs=nosys.specs -flto
ASFLAGS = --big

EXTRA = 

PREFIX = $(ROOTDIR)/sh-elf/bin/sh-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJC = $(PREFIX)objcopy

DD = dd
RM = rm -f

TARGET = build/240pMars
LIBS = $(LIBPATH) -lc -lgcc -lgcc-Os-4-200 -lnosys

M68K_BIN = src_md/m68k.bin
M68K_SOURCES = \
	$(wildcard src_md/*.c) \
	$(wildcard src_md/*.s) \
	$(wildcard liblzss/*.c)

OBJS = $(wildcard src/*.c)
SHSS = $(wildcard src/*.s)
SHOBJS = \
	crt0.o \
	sh2_fixed.o
SHOBJS += $(OBJS:.c=.o)
SHOBJS += $(SHSS:.s=.o)

.PHONY: all release debug clean

release: EXTRA = -Os
release: $(M68K_BIN) $(TARGET).32x

debug: EXTRA = -O0 -g -gdwarf-2
debug: $(M68K_BIN) $(TARGET).32x

$(M68K_BIN): $(M68K_SOURCES) src_md/Makefile src_md/mars-md.ld
	$(MAKE) -C src_md release

$(TARGET).32x: $(TARGET).elf
	$(OBJC) -O binary $< temp.bin
	$(DD) if=temp.bin of=$@ bs=128K conv=sync
	$(RM) temp.bin

$(TARGET).elf: $(SHOBJS)
	$(CC) $(LDFLAGS) $(SHOBJS) $(LIBS) -o $(TARGET).elf

crt0.o: $(M68K_BIN)

src/hw_32x.o: src/hw_32x.c
	$(CC) $(HWFLAGS) $(INCPATH) $< -o $@

src/%.o: src/%.c
	$(CC) $(CCFLAGS) $(EXTRA) $(INCPATH) $< -o $@

src/%.o: src/%.s
	$(AS) $(ASFLAGS) $(INCPATH) $< -o $@

clean:
	make clean -C src_md
	$(RM) *.o $(TARGET).32x $(TARGET).elf output.map
	$(RM) src/*.o
