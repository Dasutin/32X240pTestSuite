ROOTDIR = $(MARSDEV)

LIBPATH = -L$(ROOTDIR)/m68k-elf/lib -L$(ROOTDIR)/m68k-elf/lib/gcc/m68k-elf/4.6.2 -L$(ROOTDIR)/m68k-elf/m68k-elf/lib
INCPATH = -I. -I$(ROOTDIR)/m68k-elf/include -I$(ROOTDIR)/m68k-elf/m68k-elf/include -I../liblzss

CCFLAGS = -m68000 -Wall -c -fomit-frame-pointer
ASFLAGS = -m68000 --register-prefix-optional
LINKFLAGS = -T ./mars-md.ld -Wl,-Map=output.map -nostdlib -Wl,--print-memory-usage

EXTRA = 

PREFIX = $(ROOTDIR)/m68k-elf/bin/m68k-elf-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJC = $(PREFIX)objcopy

DD = dd
RM = rm -f

TARGET = m68k
COMBINED_TARGET = m68k-combined
LIBS = $(LIBPATH) -lc -lgcc -lnosys
COMMON_OBJS = font.o pal.o psg.o main.o segacd.o controllers.o kos.o cd.o lzss.o vgm.o vgm_player.o xvprintf.o planeb.o planeb_asm.o diagnostics.o diagnostic_68k.o
OBJS = crt0.o $(COMMON_OBJS)
COMBINED_OBJS = crt0_combined.o $(COMMON_OBJS)

.PHONY: all release combined debug clean

release: EXTRA = -Os
release: $(TARGET).bin

combined: EXTRA = -Os
combined: $(COMBINED_TARGET).bin

debug: EXTRA = -O0 -g -gdwarf-2
debug: $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	$(OBJC) -O binary --gap-fill 0 --pad-to 0x8000 $< $(TARGET).bin
	@test "$$(stat -c %s $(TARGET).bin)" -eq 32768

$(TARGET).elf: $(OBJS) Makefile mars-md.ld
	$(CC) $(LINKFLAGS) $(OBJS) $(LIBS) -o $(TARGET).elf

$(COMBINED_TARGET).bin: $(COMBINED_TARGET).elf
	$(OBJC) -O binary --gap-fill 0 --pad-to 0x8000 $< $@
	@test "$$(stat -c %s $@)" -eq 32768

$(COMBINED_TARGET).elf: $(COMBINED_OBJS) Makefile mars-md.ld
	$(CC) -T ./mars-md.ld -Wl,-Map=$(COMBINED_TARGET).map -nostdlib -Wl,--print-memory-usage \
		$(COMBINED_OBJS) $(LIBS) -o $@

crt0_combined.o: crt0.s
	$(AS) $(ASFLAGS) --defsym COMBINED_ROM=1 $(INCPATH) $< -o $@

%.o: %.c
	$(CC) $(CCFLAGS) $(EXTRA) $(INCPATH) $< -o $@

controllers.o: controllers.c
	$(CC) $(CCFLAGS) $(EXTRA) -fno-builtin -fno-tree-loop-distribute-patterns $(INCPATH) $< -o $@

diagnostics.o: diagnostics.c
	$(CC) $(CCFLAGS) $(EXTRA) -fno-builtin -fno-tree-loop-distribute-patterns -fno-caller-saves $(INCPATH) $< -o $@

%.o: ../liblzss/%.c
	$(CC) $(CCFLAGS) $(EXTRA) $(INCPATH) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $(INCPATH) $< -o $@

clean:
	$(RM) *.o *.bin *.elf *.map output.map
