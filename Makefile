ROOTDIR = $(MARSDEV)

LDSCRIPTSDIR = $(ROOTDIR)/ldscripts

LIBPATH = -L$(ROOTDIR)/sh-elf/lib -L$(ROOTDIR)/sh-elf/lib/gcc/sh-elf/4.6.2 -L$(ROOTDIR)/sh-elf/sh-elf/lib
INCPATH = -Isrc -Iinc -I$(ROOTDIR)/sh-elf/include -I$(ROOTDIR)/sh-elf/sh-elf/include

CCFLAGS = -m2 -mb -Wall -c -fomit-frame-pointer -fno-builtin  -ffunction-sections -fdata-sections
CCFLAGS += -fno-align-loops -fno-align-functions -fno-align-jumps -fno-align-labels
CCFLAGS += -D__32X__ -DMARS

HWFLAGS := $(CCFLAGS)
HWFLAGS += -Os -fno-lto
#HWFLAGS += -fno-lto

CCFLAGS += -O2 -funroll-loops -fno-align-loops -fno-align-functions -fno-align-jumps -fno-align-labels -lto

#LDFLAGS = -T $(LDSCRIPTSDIR)/mars.ld -Wl,-Map=output.map -nostdlib -Wl,--gc-sections --specs=nosys.specs -flto
LDFLAGS = -T $(LDSCRIPTSDIR)/mars_chill.ld -Wl,-Map=output.map -nostdlib -Wl,--gc-sections -flto
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

OBJS = $(wildcard src/*.c)
SHSS = $(wildcard src/*.s)
SHOBJS = \
	crt0.o \
	sh2_fixed.o
SHOBJS += $(OBJS:.c=.o)
SHOBJS += $(SHSS:.s=.o)

.PHONY: all release debug

release: EXTRA = -Os
release: m68k.bin $(TARGET).32x

debug: EXTRA = -O0 -g -gdwarf-2
debug: m68k.bin $(TARGET).32x

m68k.bin:
	make -C src_md

$(TARGET).32x: $(TARGET).elf
	$(OBJC) -O binary $< temp.bin
	$(DD) if=temp.bin of=$@ bs=128K conv=sync
	$(RM) temp.bin

$(TARGET).elf: $(SHOBJS)
	$(CC) $(LDFLAGS) $(SHOBJS) $(LIBS) -o $(TARGET).elf

crt0.o: | m68k.bin

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
