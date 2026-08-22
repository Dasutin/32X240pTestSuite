/*
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2026 Artemio Urbina
 *
 * This file is part of the 240p Test Suite
 *
 * The 240p Test Suite is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The 240p Test Suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 240p Test Suite; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "32x.h"
#include "hw_32x.h"
#include "types.h"
#include "string.h"
#include "32x_images.h"
#include "sound.h"
#include "draw.h"
#include "tests.h"
#include "shared_objects.h"
#include "help.h"
#include "segacd.h"

extern const u8 mdtest_Palette[];
extern u8 *mdtest_Reslist[];
extern const dtilemap_t mdtest_map_Map;

#define SCD_DEFAULT_DATA_LBA 16
#define SCD_INTEGRITY_WORDS  1024
#define SCD_STREAM_SECTORS   64

extern int Mars_GetFRTCounter(void);

static u16 scd_pad(void)
{
	u16 button = MARS_SYS_COMM8;
	if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		button = MARS_SYS_COMM10;
	return button;
}

static int scd_help_pressed(u16 button, u16 pressed)
{
	return (((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) &&
		(pressed & SEGA_CTRL_C)) || (pressed & SEGA_CTRL_Z);
}

u16 HwMdSegaCDCommand(u16 operation, u16 argument)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM2 = argument;
	MARS_SYS_COMM0 = 0x1600 | (operation & 0x00FF);
	while (MARS_SYS_COMM0);
	return MARS_SYS_COMM2;
}

u32 HwMdSegaCDResult0(void)
{
	u32 value = HwMdSegaCDCommand(SCD_OP_RESULT0_HI, 0);
	return (value << 16) | HwMdSegaCDCommand(SCD_OP_RESULT0_LO, 0);
}

u32 HwMdSegaCDResult1(void)
{
	u32 value = HwMdSegaCDCommand(SCD_OP_RESULT1_HI, 0);
	return (value << 16) | HwMdSegaCDCommand(SCD_OP_RESULT1_LO, 0);
}

u16 HwMdSegaCDResult2(void)
{
	return HwMdSegaCDCommand(SCD_OP_RESULT2, 0);
}

static void scd_screen_init(void)
{
	marsVDP256Start();
	initMainBG();
	Hw32xScreenFlip(0);
}

static void scd_draw_title(const char *title)
{
	drawMainBG();
	loadTextPalette();
	drawTextwHighlight((char *)title, 32, 32,
		fontColorGreen, fontColorGreenHighlight);
}

static void scd_draw_hex(const char *label, u32 value, int digits, int x, int y,
	int color)
{
	char buffer[48];
	char *out = buffer;
	strcpy(out, label);
	out += strlen(label);
	*out++ = ' ';
	*out++ = '0';
	*out++ = 'x';
	intToHex(value, out, digits);
	drawTextwHighlight(buffer, x, y, color,
		color == fontColorGreen ? fontColorGreenHighlight : fontColorWhiteHighlight);
}
static void scd_draw_genesis_hex(const char *label, u32 value, int digits,
	int x, int y, int color)
{
	char buffer[16];
	int highlight = fontColorWhiteHighlight;

	if (color == fontColorRed)
		highlight = fontColorRedHighlight;
	else if (color == fontColorGreen)
		highlight = fontColorGreenHighlight;

	drawTextwHighlight((char *)label, x, y, color, highlight);
	strcpy(buffer, " 0x");
	intToHex(value, buffer + 3, digits);
	drawTextwHighlight(buffer, x + strlen(label) * 8, y,
		fontColorWhite, fontColorWhiteHighlight);
}

static void scd_draw_number(const char *label, u32 value, int x, int y, int color)
{
	char buffer[48];
	char *out = buffer;
	strcpy(out, label);
	out += strlen(label);
	*out++ = ' ';
	uintToStr(value, out, 1);
	drawTextwHighlight(buffer, x, y, color,
		color == fontColorGreen ? fontColorGreenHighlight : fontColorWhiteHighlight);
}

static void scd_draw_prompt(const char *message)
{
	drawTextwHighlight((char *)(message ? message :
		"     PRESS ANY BUTTON     "), 64, 176,
		fontColorGreen, fontColorGreenHighlight);
}

static int scd_any_button(u16 pressed)
{
	return pressed & (SEGA_CTRL_A | SEGA_CTRL_B | SEGA_CTRL_C |
		SEGA_CTRL_START);
}

static int scd_pattern_index(u16 expected, const u16 *patterns, int count)
{
	int i;
	for (i = 0; i < count; i++)
		if (patterns[i] == expected)
			return i;
	return 0;
}

static void scd_hint_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	u16 passed;
	u32 hint, shadow;
	int done = 0;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_HINT_TEST, 0);
	hint = HwMdSegaCDResult0() & 0xFFFF;
	shadow = HwMdSegaCDResult1() & 0xFFFF;
	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex(hint == 0xFD0C ? "HINT set OK" : "HINT set FAILED",
			0xA12006, 8, hint == 0xFD0C ? 72 : 56, 96,
			hint == 0xFD0C ? fontColorGreen : fontColorRed);
		scd_draw_genesis_hex(shadow == 0xFD0C ? "HINT Shadow OK" :
			"HINT Shadow FAILED", 0x72, 8,
			shadow == 0xFD0C ? 48 : 32, 104,
			shadow == 0xFD0C ? fontColorGreen : fontColorRed);
		(void)passed;
		scd_draw_prompt(NULL);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			done = 1;
	}
	screenFadeOut(1);
}

static void scd_flag_test(void)
{
	static const u16 patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
	u16 button, pressed, old = 0xFFFF;
	u16 passed, expected, read;
	u32 detail;
	int done = 0, failed_at = 4, i;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_FLAG_TEST, 0);
	detail = HwMdSegaCDResult1();
	expected = detail >> 16;
	read = detail & 0xFF;
	if (!passed)
		failed_at = scd_pattern_index(expected, patterns, 4);

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex("Flag register", 0xA1200E, 8, 56, 64,
			fontColorGreen);
		for (i = 0; i < 4; i++)
		{
			int y = 80 + i * 16;
			scd_draw_genesis_hex("Setting to", patterns[i], 2, 88, y,
				fontColorGreen);
			if (i < failed_at || passed)
				drawTextwHighlight("W/R OK", 128, y + 8,
					fontColorWhite, fontColorWhiteHighlight);
			else if (i == failed_at)
				scd_draw_genesis_hex("FAILED", read, 2, 112, y + 8,
					fontColorRed);
		}
		scd_draw_prompt(NULL);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			done = 1;
	}
	screenFadeOut(1);
}

static void scd_comm_test(void)
{
	static const u16 patterns[] = { 0x5555, 0xAAAA, 0xFFFF, 0x0000 };
	u16 button, pressed, old = 0xFFFF;
	u16 passed, expected, read;
	u32 detail0, detail1;
	int page = 0, failed_page = 8, failed_at = 4, done = 0, i;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_COMM_TEST, 0);
	detail0 = HwMdSegaCDResult0();
	detail1 = HwMdSegaCDResult1();
	expected = detail1 >> 16;
	read = detail1 & 0xFFFF;
	if (!passed)
	{
		if (detail0 >= 0xA12010 && detail0 <= 0xA1201E)
			failed_page = (detail0 - 0xA12010) >> 1;
		else
			failed_page = 0;
		failed_at = scd_pattern_index(expected, patterns, 4);
	}

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex("Command Register", 0xA12010 + page * 2, 8,
			56, 64, fontColorGreen);
		for (i = 0; i < 4; i++)
		{
			int y = 80 + i * 16;
			scd_draw_genesis_hex("Setting to", patterns[i], 4, 88, y,
				fontColorGreen);
			if (page < failed_page || passed ||
				(page == failed_page && i < failed_at))
				drawTextwHighlight("W/R OK", 128, y + 8,
					fontColorWhite, fontColorWhiteHighlight);
			else if (page == failed_page && i == failed_at)
				scd_draw_genesis_hex("FAILED", read, 4, 104, y + 8,
					fontColorRed);
		}
		scd_draw_prompt(NULL);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
		{
			if (++page >= 8 || (!passed && page > failed_page))
				done = 1;
			old = scd_pad();
		}
	}
	screenFadeOut(1);
}

static void scd_draw_ram_patterns(int passed, int failed_at, u32 detail0,
	u16 read)
{
	static const u16 patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
	int i;

	for (i = 0; i < 4; i++)
	{
		int y = 64 + i * 24;
		scd_draw_genesis_hex("Setting to", patterns[i], 4, 96, y,
			fontColorGreen);
		if (passed || i < failed_at)
			drawTextwHighlight("W/R OK", 128, y + 8,
				fontColorWhite, fontColorWhiteHighlight);
		else if (i == failed_at)
		{
			scd_draw_genesis_hex("Failed @", detail0, 7, 80, y + 8,
				fontColorRed);
			scd_draw_genesis_hex("GOT", read, 4, 112, y + 16,
				fontColorRed);
		}
	}
}

static void scd_word_ram_test(void)
{
	static const u16 patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
	u16 button, pressed, old = 0xFFFF;
	u16 passed, expected, read;
	u32 detail0, detail1;
	int done = 0, failed_at = 4;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_WORD_RAM_TEST, 0);
	detail0 = HwMdSegaCDResult0();
	detail1 = HwMdSegaCDResult1();
	expected = detail1 >> 16;
	read = detail1 & 0xFFFF;
	if (!passed)
		failed_at = scd_pattern_index(expected, patterns, 4);

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex("WORD RAM", 0x600000, 7, 88, 32,
			fontColorGreen);
		scd_draw_ram_patterns(passed, failed_at, detail0, read);
		scd_draw_prompt(NULL);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			done = 1;
	}
	screenFadeOut(1);
}

static void scd_program_ram_test(void)
{
	static const u16 patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
	u16 button, pressed, old = 0xFFFF;
	u16 passed, expected, read;
	u32 detail0, detail1;
	int page = -1, failed_bank = 4, failed_at = 4, done = 0;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_PROGRAM_RAM_TEST, 0);
	detail0 = HwMdSegaCDResult0();
	detail1 = HwMdSegaCDResult1();
	expected = detail1 >> 16;
	read = detail1 & 0xFFFF;
	if (!passed && detail0 != 0xA12002)
	{
		if (detail0 >= 0x420000 && detail0 < 0x4A0000)
			failed_bank = (detail0 - 0x420000) / 0x20000;
		else
			failed_bank = 0;
		failed_at = scd_pattern_index(expected, patterns, 4);
	}

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex("Program RAM", 0x420000, 6, 80, 32,
			fontColorGreen);
		if (page < 0)
		{
			if (!passed && detail0 == 0xA12002)
			{
				scd_draw_genesis_hex("Bank Switch FAIL", 0xA12002, 8, 40, 64,
					fontColorRed);
				drawTextwHighlight("Only one bank will be checked", 40, 88,
					fontColorWhite, fontColorWhiteHighlight);
			}
			else
				scd_draw_genesis_hex("Bank Register OK", 0xA12002, 7, 56, 112,
					fontColorWhite);
		}
		else
		{
			scd_draw_genesis_hex("Bank Test", page, 2, 104, 48,
				fontColorGreen);
			scd_draw_ram_patterns(passed || page < failed_bank,
				page == failed_bank ? failed_at : 4, detail0, read);
		}
		scd_draw_prompt(NULL);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
		{
			if ((!passed && detail0 == 0xA12002) ||
				++page >= 4 || (!passed && page > failed_bank))
				done = 1;
			old = scd_pad();
		}
	}
	screenFadeOut(1);
}

typedef struct scd_bios_id_t {
	u32 crc;
	const char *name;
} scd_bios_id_t;

static const scd_bios_id_t scd_bios_ids[] = {
	{ 0xC6D10268, "M1 US 1.10" }, { 0x529AC15A, "M1 EU 1.00" },
	{ 0x79F85384, "M1 JP 1.0S" }, { 0x9BCE40B2, "M1 JP 1.0G" },
	{ 0xF18DDE5B, "M1 JP 1.0L" }, { 0x2EA250C0, "M1 JP 1.0O" },
	{ 0x9D2DA8F2, "M1 JP 1.01" }, { 0x4BE18FF6, "M1 JP 1.11" },
	{ 0x550F30BB, "M1 AS 1.0" }, { 0x2E49D72C, "M2 US 2.11X" },
	{ 0x8AF65F58, "M2 US 2.00" }, { 0x9F6F6276, "M2 US 2.00W" },
	{ 0x4D5CB8DA, "M2 EU 2.00W" }, { 0x0507B590, "M2 EU 2.00" },
	{ 0xC1AA217F, "M2 EU 2.11X" }, { 0xDD6CC972, "M2 JP 2.00C" },
	{ 0x8052C7A0, "AIWA JP 2.11" }, { 0x50CD3D23, "LA 1.04" },
	{ 0x3B10CF41, "LA 1.02" }, { 0x474AAA44, "LA JP 1.05A" },
	{ 0x1493522C, "LA JP 1.05B" }, { 0x00EEDB3A, "LA JP 1.02" },
	{ 0x290F8E33, "X'EYE US 2.00" }, { 0xD21FE71D, "WM JP 1.00" },
	{ 0x2B19972F, "WM 2.00" }, { 0xD48C44B5, "CDX US 2.21X" },
	{ 0xAACB851E, "MMEGA 2.21" }, { 0, NULL }
};

static const char *scd_bios_name(u32 crc)
{
	int i = 0;
	while (scd_bios_ids[i].crc)
	{
		if (scd_bios_ids[i].crc == crc)
			return scd_bios_ids[i].name;
		i++;
	}
	return NULL;
}

static u8 scd_bios_byte(u16 offset)
{
	u16 word = HwMdSegaCDCommand(SCD_OP_BIOS_WORD, offset & 0xFFFE);
	return (offset & 1) ? (word & 0xFF) : (word >> 8);
}

static void scd_bios_info(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, calculated = 0, present;
	u32 crc = 0;
	static char fields[9][49];
	static const int layout[] = {
		16, 16, 17, 15, 16, 48, 14, -2, 16, -4, -4, -4, -4,
		-2, -2, -4, -4, -12, -40, 16, 0
	};
	int i, j = 0, offset = 0x047E;

	present = HwMdSegaCDCommand(SCD_OP_STATUS, 0) & SCD_STATUS_PRESENT;
	for (i = 0; i < 9; i++)
		fields[i][0] = 0;
	if (present)
	{
		for (i = 0; layout[i]; i++)
		{
			int length = layout[i] > 0 ? layout[i] : -layout[i];
			if (layout[i] > 0)
			{
				int k;
				for (k = 0; k < length; k++)
				{
					u8 value = scd_bios_byte(offset + k);
					fields[j][k] = (value >= 0x20 && value <= 0x7E) ?
						(char)value : ' ';
				}
				fields[j][length] = 0;
				j++;
			}
			offset += length;
		}
	}

	scd_screen_init();
	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_genesis_hex("Sega CD BIOS Data at", 0x400000, 8, 48, 32,
			fontColorGreen);
		if (present)
		{
			const char *name;
			for (i = 0; i < 9; i++)
				drawTextwHighlight(fields[i], 96, 64 + i * 8,
					fontColorWhite, fontColorWhiteHighlight);
			if (calculated)
			{
				scd_draw_genesis_hex("CD BIOS CRC32:", crc, 8, 48, 152,
					fontColorGreen);
				name = scd_bios_name(crc);
				if (name)
					drawTextwHighlight((char *)name, 112, 160,
						fontColorWhite, fontColorWhiteHighlight);
				else
					drawTextwHighlight("Unknown BIOS, please report CRC", 32, 160,
						fontColorGreen, fontColorGreenHighlight);
				scd_draw_prompt(NULL);
			}
			else
				scd_draw_prompt("'A' for CRC 'B' to exit");
		}
		else
		{
			drawTextwHighlight("No BIOS found", 96, 160,
				fontColorGreen, fontColorGreenHighlight);
			scd_draw_prompt(NULL);
		}
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (present && (pressed & SEGA_CTRL_A) && !calculated)
		{
			if (HwMdSegaCDCommand(SCD_OP_BIOS_WORD, 0xFFFF))
			{
				crc = HwMdSegaCDResult0();
				calculated = 1;
				old = scd_pad();
			}
		}
		else if (calculated && scd_any_button(pressed))
			done = 1;
		else if (!present && scd_any_button(pressed))
			done = 1;
		else if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))
			done = 1;
	}
	screenFadeOut(1);
}

static void scd_memory_viewer(void)
{
	static const u32 locations[] = {
		0, 0x020000, 0x200000, 0x400000,
		0x420000, 0x600000, 0xA00000, 0xFF0000
	};
	static u8 bytes[16 * 28];
	u16 button, pressed, old = 0xFFFF;
	u32 address = 0x400000, crc = 0;
	int location = 3, bank = 0, ascii = 0, docrc = 0;
	int done = 0, reload = 1;
	int i, row, column;

	for (i = 0; i < 255; i++)
		setColor(i, 0, 0, 0);
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES |
		MARS_VDP_MODE_256;
	HwMdClearScreen();
	Hw32xScreenFlip(0);
	HwMdSegaCDCommand(SCD_OP_MEM_BANK, 0);

	while (!done)
	{
		if (reload)
		{
			char buffer[10];
			HwMdSegaCDCommand(SCD_OP_MEM_ADDR_HI, address >> 16);
			HwMdSegaCDCommand(SCD_OP_MEM_ADDR_LO, address);
			for (i = 0; i < 16 * 28; i += 2)
			{
				u16 word = HwMdSegaCDCommand(SCD_OP_MEM_READ_WORD, 0);
				bytes[i] = word >> 8;
				bytes[i + 1] = word & 0xFF;
			}

			HwMdClearScreen();
			intToHex(address, buffer, 8);
			HwMdPuts(buffer, 0x4000, 32, 0);
			intToHex(address + 448, buffer, 8);
			HwMdPuts(buffer, 0x4000, 32, 27);

			if (docrc)
			{
				CRC32_reset();
				for (i = 0; i < 16 * 28; i++)
					CRC32_update(bytes[i]);
				crc = CRC32_finalize();
				intToHex(crc, buffer, 8);
				HwMdPuts(buffer, 0x0000, 32, 14);
			}

			if (address >= 0x420000 && address < 0x440000)
			{
				HwMdPuts("PB", 0x4000, 33, 3);
				intToHex(bank, buffer, 2);
				HwMdPuts(" 0x", 0x0000, 35, 3);
				HwMdPuts(buffer, 0x0000, 38, 3);
				HwMdPuts("C->bank", 0x4000, 33, 4);
			}

			for (row = 0; row < 28; row++)
			{
				for (column = 0; column < 16; column++)
				{
					u8 value = bytes[row * 16 + column];
					if (ascii)
					{
						buffer[0] = (value >= 32 && value <= 126) ?
							(char)value : ' ';
						buffer[1] = 0;
					}
					else
						intToHex(value, buffer, 2);
					HwMdPuts(buffer, 0x0000, column * 2, row);
				}
			}
			reload = 0;
		}

		Hw32xFlipWait();

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_A)
		{
			docrc ^= 1;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_B)
		{
			if (++location == 8)
				location = 0;
			address = locations[location];
			reload = 1;
		}
		if (pressed & SEGA_CTRL_C)
		{
			if (address >= 0x420000 && address < 0x440000)
			{
				if (++bank > 3)
					bank = 0;
				HwMdSegaCDCommand(SCD_OP_MEM_BANK, bank);
			}
			else
				ascii ^= 1;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_LEFT)
		{
			address = address > 448 ? address - 448 : 0;
			if (address >= 0x800000 && address < 0xA00000)
				address = 0xA0FFFF;
			if (address >= 0xA0FFFF && address < 0xFF0000)
				address = 0x7F0000;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_RIGHT)
		{
			address += 448;
			if (address >= 0x800000 && address < 0xA00000)
				address = 0xA00000;
			if (address >= 0xA0FFFF && address < 0xFF0000)
				address = 0xFF0000;
			if (address >= 0xFFFFFF)
				address = 0xFFFE3F;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_UP)
		{
			address = address >= 0x10000 ? address - 0x10000 : 0;
			if (address >= 0x800000 && address < 0xA00000)
				address = 0xA0FFFF;
			if (address >= 0xA0FFFF && address < 0xFF0000)
				address = 0x7F0000;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_DOWN)
		{
			address += 0x10000;
			if (address >= 0x800000 && address < 0xA00000)
				address = 0xA00000;
			if (address >= 0xA0FFFF && address < 0xFF0000)
				address = 0xFF0000;
			if (address >= 0xFFFFFF)
				address = 0xFFFE3F;
			reload = 1;
		}
		if (pressed & SEGA_CTRL_START)
			done = 1;
	}
	HwMdClearScreen();
	screenFadeOut(1);
}

typedef struct scd_pcm_result_t {
	int status;
	u16 bank;
	u16 offset;
	u16 read;
} scd_pcm_result_t;

static void scd_draw_float_box(void)
{
	static u8 pixels[128 * 80] ATTR_CACHE_ALIGNED;
	static int initialized = 0;
	int row, column;

	if (!initialized)
	{
		for (row = 0; row < 80; row++)
			for (column = 0; column < 128; column++)
				pixels[row * 128 + column] =
					(row == 0 || row == 79 || column == 0 || column == 127) ?
					fontColorBlack : fontBackgroundColorBlack;
		initialized = 1;
	}
	draw_sprite(96, 72, 128, 80, pixels,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
}

static int scd_pcm_ram_menu(int selection)
{
	static const char *items[] = {
		"PCM RAM Check", "Full Test", "Full w/1s wait",
		"Per Bank", "Bank w/1s wait", "return"
	};
	static const int item_x[] = { 104, 120, 104, 128, 104, 136 };
	u16 button, pressed, old = 0xFFFF;
	int i, done = 0;

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		scd_draw_float_box();
		drawTextwHighlight((char *)items[0], item_x[0], 72,
			fontColorGreen, fontColorGreenHighlight);
		for (i = 1; i < 6; i++)
			drawTextwHighlight((char *)items[i], item_x[i], 88 + i * 8,
				selection == i ? fontColorRed : fontColorWhite,
				selection == i ? fontColorRedHighlight :
					fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_UP)
			selection = selection > 1 ? selection - 1 : 5;
		if (pressed & SEGA_CTRL_DOWN)
			selection = selection < 5 ? selection + 1 : 1;
		if (pressed & SEGA_CTRL_A)
			done = 1;
		if (pressed & SEGA_CTRL_START)
		{
			selection = 5;
			done = 1;
		}
	}
	return selection;
}

static void scd_pcm_run_patterns(int target, int wait,
	scd_pcm_result_t *results)
{
	static const u16 patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
	int i;

	for (i = 0; i < 4; i++)
	{
		u16 argument = ((target & 0xFF) << 8) | patterns[i];
		results[i].status = -1;
		results[i].bank = target == 0xFF ? 0 : target;
		results[i].offset = 0;
		results[i].read = 0;
		if (HwMdSegaCDCommand(SCD_OP_PCM_RAM_SET, argument))
		{
			u32 detail;
			if (wait)
				Hw32xDelay(Hw32xDetectPAL() ? 50 : 60);
			results[i].status = HwMdSegaCDCommand(
				SCD_OP_PCM_RAM_COMPARE, argument) ? 1 : 0;
			if (!results[i].status)
			{
				detail = HwMdSegaCDResult0();
				results[i].bank = detail >> 16;
				results[i].offset = detail & 0xFFFF;
				results[i].read = HwMdSegaCDResult2();
			}
		}
	}
}

static void scd_draw_pcm_results(const scd_pcm_result_t *results)
{
	static const u16 patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
	int i;

	for (i = 0; i < 4; i++)
	{
		int y = 48 + i * 32;
		if (results[i].status > 0)
			scd_draw_genesis_hex("Memory OK w/value:", patterns[i], 2,
				64, y, fontColorGreen);
		else if (!results[i].status)
		{
			scd_draw_genesis_hex("Memory Failed w/value:", patterns[i], 2,
				64, y, fontColorRed);
			scd_draw_genesis_hex("bank:", results[i].bank, 2,
				96, y + 8, fontColorGreen);
			scd_draw_genesis_hex("offset (8bit):", results[i].offset & 0x0FFF, 4,
				96, y + 16, fontColorGreen);
			scd_draw_genesis_hex("read value:", results[i].read, 2,
				96, y + 24, fontColorGreen);
		}
		else
			scd_draw_genesis_hex("Memory write failed w/:", patterns[i], 2,
				64, y, fontColorRed);
	}
}

static void scd_pcm_ram_test(void)
{
	u16 button, pressed, old;
	int type = 1, exit_menu = 0;
	scd_pcm_result_t results[4];

	if (!HwMdSegaCDCommand(SCD_OP_INIT, 0))
		return;

	scd_screen_init();
	while (!exit_menu)
	{
		type = scd_pcm_ram_menu(type);
		if (type == 5)
			break;

		if (type == 1 || type == 2)
		{
			int exit_test = 0;
			while (!exit_test)
			{
				int done = 0;
				scd_pcm_run_patterns(0xFF, type == 2, results);
				old = 0xFFFF;
				while (!done)
				{
					Hw32xFlipWait();
					drawMainBG();
					loadTextPalette();
					scd_draw_genesis_hex("PCM RAM Test (Sega CD)", 0x2000, 4,
						40, 32, fontColorGreen);
					scd_draw_pcm_results(results);
					scd_draw_prompt("'A' to test 'B' to exit");
					Hw32xScreenFlip(0);
					button = scd_pad();
					pressed = button & ~old;
					old = button;
					if (pressed & SEGA_CTRL_A)
						done = 1;
					if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))
					{
						done = 1;
						exit_test = 1;
					}
				}
			}
		}
		else
		{
			int bank = 0, exit_test = 0;
			while (!exit_test && bank < 16)
			{
				int done = 0;
				scd_pcm_run_patterns(bank, type == 4, results);
				old = 0xFFFF;
				while (!done)
				{
					Hw32xFlipWait();
					drawMainBG();
					loadTextPalette();
					scd_draw_genesis_hex("PCM RAM Test (Sega CD) Bank", bank, 2,
						32, 32, fontColorGreen);
					scd_draw_pcm_results(results);
					scd_draw_prompt("'A' for next 'B' to exit");
					Hw32xScreenFlip(0);
					button = scd_pad();
					pressed = button & ~old;
					old = button;
					if (pressed & SEGA_CTRL_A)
					{
						bank++;
						done = 1;
					}
					if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))
					{
						done = 1;
						exit_test = 1;
					}
				}
			}
		}
	}
	screenFadeOut(1);
}

static void scd_sound_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0, option = 1, track = 0, tracks = 0;
	int track_type = 0xFF, enable_cd = 0, pcm_timer = 0;
	int warning = 0, model, status;
	u16 cd_status = 0;
	const char *warning_text = NULL;

	status = HwMdSegaCDCommand(SCD_OP_INIT, 0);

	if (!status)
		return;

	scd_screen_init();
	model = scd_bios_byte(0x18A) == '1' ? 1 : 2;
	while (!done)
	{
		char track_text[8] = "  NONE ";
		int max_option = selection == 0 ? 2 :
			(selection == 1 ? 3 : 0);

		if (tracks && track)
		{
			strcpy(track_text, track_type == 0 ? "CDDA   " : "DATA   ");
			uintToStr(track, track_text + 5, 2);
		}
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		drawTextwHighlight("Sega CD Sound Test", 88, 32,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Ricoh RF5C164 (315-5476A)", 64, 72,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Left", 88, 88,
			selection == 0 && option == 0 ? fontColorRed : fontColorWhite,
			selection == 0 && option == 0 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Center", 128, 88,
			selection == 0 && option == 1 ? fontColorRed : fontColorWhite,
			selection == 0 && option == 1 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Right", 184, 88,
			selection == 0 && option == 2 ? fontColorRed : fontColorWhite,
			selection == 0 && option == 2 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("CD Drive", 128, 112,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Play", 80, 120,
			selection == 1 && option == 0 ? fontColorRed : fontColorWhite,
			selection == 1 && option == 0 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Stop", 120, 120,
			selection == 1 && option == 1 ? fontColorRed : fontColorWhite,
			selection == 1 && option == 1 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Open", 160, 120,
			selection == 1 && option == 2 ? fontColorRed : fontColorWhite,
			selection == 1 && option == 2 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Close", 200, 120,
			selection == 1 && option == 3 ? fontColorRed : fontColorWhite,
			selection == 1 && option == 3 ? fontColorRedHighlight :
				fontColorWhiteHighlight);
		drawTextwHighlight("Track", 136, 128,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight(track_text, 128, 136,
			selection == 2 ? fontColorRed : fontColorWhite,
			selection == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		if (!enable_cd)
			drawTextwHighlight("Press C to enable CD", 80, 160,
				fontColorWhite, fontColorWhiteHighlight);
		else if (!tracks)
		{
			if (cd_status == 0x0B)
				drawTextwHighlight("CD tray is open", 96, 160,
					fontColorGreen, fontColorGreenHighlight);
			else if (cd_status == 0x0C)
				drawTextwHighlight("No CD in tray", 112, 160,
					fontColorGreen, fontColorGreenHighlight);
			else if (cd_status == 0x0A)
				drawTextwHighlight("TOC reported 0 tracks", 80, 160,
					fontColorGreen, fontColorGreenHighlight);
		}
		if (warning && warning_text)
			drawTextwHighlight((char *)warning_text, 72, 152,
				fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		if (warning)
			warning--;
		if (pcm_timer && !--pcm_timer)
			HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_UP)
			selection = selection ? selection - 1 : 2;
		if (pressed & SEGA_CTRL_DOWN)
			selection = selection < 2 ? selection + 1 : 0;
		if (selection < 2)
		{
			if (pressed & SEGA_CTRL_LEFT)
				option = option ? option - 1 : max_option;
			if (pressed & SEGA_CTRL_RIGHT)
				option = option < max_option ? option + 1 : 0;
		}
		else if (tracks)
		{
			if (pressed & SEGA_CTRL_LEFT)
				track = track > 1 ? track - 1 : tracks;
			if (pressed & SEGA_CTRL_RIGHT)
				track = track < tracks ? track + 1 : 1;
			if (pressed & (SEGA_CTRL_LEFT | SEGA_CTRL_RIGHT))
			{
				track_type = HwMdSegaCDCommand(SCD_OP_TRACK_INFO, track) ?
					(HwMdSegaCDResult2() & 0xFF) : 0xFF;
			}
		}
		if ((pressed & SEGA_CTRL_A) && status)
		{
			if (selection == 0)
			{
				if (pcm_timer)
					HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
				HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, option + 1);
				pcm_timer = 110;
			}
			else if (selection == 1 && enable_cd)
			{
				if (option == 0)
				{
					if (track && track_type == 0)
						HwMdSegaCDCommand(SCD_OP_CDDA_PLAY, track);
					else if (track)
					{
						warning_text = "Cannot play DATA Track";
						warning = 120;
					}
				}
				else if (option == 1)
					HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
				else if (option == 2)
				{
					if (model == 2)
					{
						warning_text = "Model 2 open manually";
						warning = 120;
					}
					else
						HwMdSegaCDCommand(SCD_OP_TRAY_OPEN, 0);
				}
				else if (model == 2)
				{
					warning_text = "Model 2 close manually";
					warning = 120;
				}
				else
					HwMdSegaCDCommand(SCD_OP_TRAY_CLOSE, 0);
			}
		}
		if (pressed & SEGA_CTRL_C)
		{
			enable_cd ^= 1;
			if (enable_cd)
			{
				u32 disc;
				if (HwMdSegaCDCommand(SCD_OP_DISC_INFO, 0))
				{
					disc = HwMdSegaCDResult0();
					cd_status = disc >> 16;
					tracks = disc & 0xFF;
					track = tracks ? 1 : 0;
					while (track && track <= tracks)
					{
						track_type = HwMdSegaCDCommand(SCD_OP_TRACK_INFO,
							track) ? (HwMdSegaCDResult2() & 0xFF) : 0xFF;
						if (track_type == 0 || track == tracks)
							break;
						track++;
					}
				}
			}
		}
		if (pressed & SEGA_CTRL_START)
			done = 1;
	}
	if (status)
	{
		HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
		HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	}
	screenFadeOut(1);
}

static u32 scd_crc_word(u32 crc, u16 word)
{
	int byte, bit;
	for (byte = 0; byte < 2; byte++)
	{
		u8 value = byte ? (word & 0xFF) : (word >> 8);
		crc ^= value;
		for (bit = 0; bit < 8; bit++)
			crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320UL : 0);
	}
	return crc;
}

static void scd_integrity_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, passed = 0;
	u16 poll = 0;
	u32 sub_crc = 0, main_crc = 0, sh2_crc = 0;
	u32 lba = SCD_DEFAULT_DATA_LBA;

	scd_screen_init();
	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_title("CD-to-32X Streaming Integrity");
		scd_draw_number("Logical sector:", lba, 32, 56, fontColorGreen);
		if (running)
			drawTextwHighlight("Reading and verifying sector...", 32, 88,
				fontColorWhite, fontColorWhiteHighlight);
		else if (!complete)
			drawTextwHighlight("Press A to read one Mode 1 sector.", 32, 88,
				fontColorWhite, fontColorWhiteHighlight);
		else
		{
			drawTextwHighlight(passed ? "All three CRCs match" : "CRC mismatch",
				32, 80, passed ? fontColorGreen : fontColorRed,
				passed ? fontColorGreenHighlight : fontColorRedHighlight);
			scd_draw_hex("Sega CD:", sub_crc, 8, 32, 104, fontColorWhite);
			scd_draw_hex("68000:", main_crc, 8, 32, 120, fontColorWhite);
			scd_draw_hex("SH2:", sh2_crc, 8, 32, 136, fontColorWhite);
		}
		drawTextwHighlight("Left/Right:LBA  A:run  B:exit", 24, 176,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		if (running)
		{
			poll = HwMdSegaCDCommand(SCD_OP_DATA_POLL, 0);
			if (poll)
			{
				int i;
				running = 0;
				complete = 1;
				if (poll == 1)
				{
					sub_crc = HwMdSegaCDResult0();
					main_crc = HwMdSegaCDResult1();
					sh2_crc = 0xFFFFFFFFUL;
					for (i = 0; i < SCD_INTEGRITY_WORDS; i++)
						sh2_crc = scd_crc_word(sh2_crc,
							HwMdSegaCDCommand(SCD_OP_WORD_RAM_READ, i));
					sh2_crc ^= 0xFFFFFFFFUL;
					passed = sub_crc == main_crc && main_crc == sh2_crc;
				}
				else
					passed = 0;
				old = scd_pad();
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_LEFT) && lba) { lba--; complete = 0; }
		if (!running && (pressed & SEGA_CTRL_RIGHT)) { lba++; complete = 0; }
		if (!running && (pressed & SEGA_CTRL_A))
		{
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, lba);
				running = HwMdSegaCDCommand(SCD_OP_DATA_START, 1);
				complete = 0;
			}
		}
		if (!running && (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stream_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, failed = 0;
	u32 last_tick = 0, frames = 0, missed = 0;
	u32 rate = 0;
	u16 poll = 0;

	scd_screen_init();
	while (!done)
	{
		u32 tick = Hw32xGetTicks();
		Hw32xFlipWait();
		scd_draw_title("CD Streaming Bandwidth");
		drawTextwHighlight("Reads 64 sectors while the SH2", 32, 56,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("continues rendering every frame.", 32, 68,
			fontColorWhite, fontColorWhiteHighlight);
		if (running)
			drawTextwHighlight("Streaming...", 32, 96,
				fontColorGreen, fontColorGreenHighlight);
		else if (!complete)
			drawTextwHighlight("Press A to begin.", 32, 96,
				fontColorWhite, fontColorWhiteHighlight);
		else
		{
			if (failed)
				drawTextwHighlight("Disc read failed.", 32, 96,
					fontColorRed, fontColorRedHighlight);
			else
			{
				scd_draw_number("Frames:", frames, 32, 96, fontColorWhite);
				scd_draw_number("Missed frames:", missed, 32, 112,
					missed ? fontColorRed : fontColorGreen);
				scd_draw_number("Approx sectors/sec:", rate, 32, 128, fontColorGreen);
			}
		}
		drawTextwHighlight("A:run  B:exit", 32, 176,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		if (running)
		{
			if (last_tick && tick > last_tick + 1)
				missed += tick - last_tick - 1;
			last_tick = tick;
			frames++;
			poll = HwMdSegaCDCommand(SCD_OP_DATA_POLL, 0);
			if (poll)
			{
				running = 0;
				complete = 1;
				failed = poll != 1;
				if (!failed && frames)
					rate = (SCD_STREAM_SECTORS *
						(Hw32xDetectPAL() ? 50 : 60)) / frames;
				old = scd_pad();
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_A))
		{
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, SCD_DEFAULT_DATA_LBA);
				running = HwMdSegaCDCommand(SCD_OP_DATA_START, SCD_STREAM_SECTORS);
				last_tick = Hw32xGetTicks();
				frames = missed = 0;
				complete = 0;
				failed = 0;
			}
		}
		if (!running && (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stop_audio_source(int source, sound_t *pwm)
{
	if (source == 0) HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
	if (source == 1) HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	if (source == 2) sound_stopSound(pwm);
	if (source == 3) HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
	if (source == 4) HwMdSegaCDCommand(SCD_OP_YM_CONTROL, 0);
}

static void scd_combined_audio(void)
{
	static const char *sources[] = {
		"Sega CD PCM", "Sega CD CD-DA", "32X PWM",
		"Genesis PSG", "Genesis YM2612"
	};
	static const char *pans[] = { "Left", "Center", "Right" };
	u16 button, pressed, old = 0xFFFF;
	int done = 0, source = 0, pan = 1, playing = 0, track = 2;
	sound_t pwm;

	if (!sound_isInitialized())
		Mars_InitSoundDMA();
	sound_load(&pwm, "jump");
	MDPSG_init();
	scd_screen_init();

	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_title("Combined Audio Mixer");
		drawTextwHighlight("Source:", 32, 64,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight((char *)sources[source], 104, 64,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("Output:", 32, 88,
			fontColorGreen, fontColorGreenHighlight);
		if (source == 1 || source == 3)
			drawTextwHighlight("Fixed by hardware/source", 104, 88,
				fontColorWhite, fontColorWhiteHighlight);
		else
			drawTextwHighlight((char *)pans[pan], 104, 88,
				fontColorWhite, fontColorWhiteHighlight);
		if (source == 1)
			scd_draw_number("CD track:", track, 32, 112, fontColorWhite);
		drawTextwHighlight(playing ? "Playing" : "Stopped", 32, 136,
			playing ? fontColorGreen : fontColorWhite,
			playing ? fontColorGreenHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Up/Down:source  Left/Right:output", 16, 168,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("A:play/stop  B:exit", 16, 184,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_UP)
		{
			if (playing) scd_stop_audio_source(source, &pwm);
			playing = 0;
			source = source ? source - 1 : 4;
		}
		if (pressed & SEGA_CTRL_DOWN)
		{
			if (playing) scd_stop_audio_source(source, &pwm);
			playing = 0;
			source = source == 4 ? 0 : source + 1;
		}
		if (source == 1)
		{
			if ((pressed & SEGA_CTRL_LEFT) && track > 1) track--;
			if ((pressed & SEGA_CTRL_RIGHT) && track < 99) track++;
		}
		else if (source != 3)
		{
			if (pressed & SEGA_CTRL_LEFT) pan = pan ? pan - 1 : 2;
			if (pressed & SEGA_CTRL_RIGHT) pan = pan == 2 ? 0 : pan + 1;
		}
		if (pressed & SEGA_CTRL_A)
		{
			if (playing)
			{
				scd_stop_audio_source(source, &pwm);
				playing = 0;
			}
			else
			{
				if (source == 0 && HwMdSegaCDCommand(SCD_OP_INIT, 0))
					playing = HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, pan + 1);
				if (source == 1 && HwMdSegaCDCommand(SCD_OP_INIT, 0))
					playing = HwMdSegaCDCommand(SCD_OP_CDDA_PLAY, track);
				if (source == 2)
					playing = sound_play(&pwm, -1, pan == 0 ? 1 : (pan == 2 ? 2 : 3)) >= 0;
				if (source == 3)
				{
					HwMdPSGSetChandVol(0, 0);
					HwMdPSGSetFrequency(0, 1000);
					playing = 1;
				}
				if (source == 4)
					playing = HwMdSegaCDCommand(SCD_OP_YM_CONTROL,
						1 | (pan << 8));
			}
		}
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
	}
	if (playing) scd_stop_audio_source(source, &pwm);
	MDPSG_stop();
	sound_free(&pwm);
	screenFadeOut(1);
}

static void scd_av_sync(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, frame = 0, offset = 0, track = 3;
	int audio_started = 0, initialized = 0;

	scd_screen_init();
	while (!done)
	{
		int flash = running && ((offset >= 0 && frame == offset) ||
			(offset < 0 && frame == 0));
		Hw32xFlipWait();
		if (flash)
		{
			volatile u8 *fb = (volatile u8 *)((u16 *)&MARS_FRAMEBUFFER + 0x100);
			int i;
			for (i = 0; i < 320 * 224; i++) fb[i] = fontColorWhite;
			loadTextPalette();
		}
		else
		{
			scd_draw_title("CD-DA / 32X Video Sync");
			scd_draw_number("Track:", track, 32, 64, fontColorWhite);
			scd_draw_number("Video offset (frames):", offset, 32, 80, fontColorWhite);
			if (running) scd_draw_number("Frame:", frame, 32, 112, fontColorGreen);
			else drawTextwHighlight("Press A to start playback and flash.", 32, 112,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("Left/Right:offset  Up/Down:track", 16, 168,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("A:start  B:exit", 16, 184,
				fontColorWhite, fontColorWhiteHighlight);
		}
		Hw32xScreenFlip(0);

		if (running)
		{
			if (!audio_started && ((offset >= 0 && frame == 0) ||
				(offset < 0 && frame == -offset)))
			{
				HwMdSegaCDCommand(SCD_OP_CDDA_PLAY, track);
				audio_started = 1;
			}
			frame++;
			if (frame > 300)
			{
				running = 0;
				HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
				audio_started = 0;
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_LEFT) && offset > -30) offset--;
		if (!running && (pressed & SEGA_CTRL_RIGHT) && offset < 30) offset++;
		if (!running && (pressed & SEGA_CTRL_UP) && track < 99) track++;
		if (!running && (pressed & SEGA_CTRL_DOWN) && track > 1) track--;
		if (!running && (pressed & SEGA_CTRL_A) &&
			HwMdSegaCDCommand(SCD_OP_INIT, 0))
		{
			initialized = 1;
			frame = 0;
			audio_started = 0;
			running = 1;
		}
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
	}
	if (initialized)
		HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	screenFadeOut(1);
}

static void scd_wordram_stress(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, passed = 0;
	u16 pass = 0;

	scd_screen_init();
	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_title("Word RAM Ownership Stress");
		drawTextwHighlight("Sub-CPU fills, transfers ownership,", 32, 56,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("and the 68000 verifies each pass.", 32, 68,
			fontColorWhite, fontColorWhiteHighlight);
		scd_draw_number("Completed passes:", pass, 32, 96, fontColorGreen);
		if (complete)
			drawTextwHighlight(passed ? "PASS" : "FAIL", 32, 120,
				passed ? fontColorGreen : fontColorRed,
				passed ? fontColorGreenHighlight : fontColorRedHighlight);
		else if (!running)
			drawTextwHighlight("Press A to run 120 handoffs.", 32, 120,
				fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("B or Start exits.", 32, 176,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		if (running)
		{
			passed = HwMdSegaCDCommand(SCD_OP_WORD_RAM_STRESS,
				(u16)(0x5A00 ^ pass));
			if (!passed || ++pass == 120)
			{
				running = 0;
				complete = 1;
				old = scd_pad();
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_A) &&
			HwMdSegaCDCommand(SCD_OP_INIT, 0))
		{
			pass = 0;
			complete = 0;
			passed = 1;
			running = 1;
		}
		if (!running && (pressed & (SEGA_CTRL_B | SEGA_CTRL_START))) done = 1;
	}
	screenFadeOut(1);
}

static void scd_latency_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, complete = 0;
	u32 minimum = 0, maximum = 0, average = 0;

	scd_screen_init();
	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_title("Multi-CPU Communication Latency");
		drawTextwHighlight("Measures SH2 -> 68000 -> Sub-CPU", 32, 56,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("round trips with 64 commands.", 32, 68,
			fontColorWhite, fontColorWhiteHighlight);
		if (!complete)
			drawTextwHighlight("Press A to measure.", 32, 96,
				fontColorWhite, fontColorWhiteHighlight);
		else
		{
			scd_draw_number("Minimum ticks:", minimum, 32, 96, fontColorWhite);
			scd_draw_number("Maximum ticks:", maximum, 32, 112, fontColorWhite);
			scd_draw_number("Average ticks:", average, 32, 128, fontColorGreen);
		}
		drawTextwHighlight("B or Start exits.", 32, 176,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_A)
		{
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				u32 total = 0;
				int i;
				minimum = 0xFFFFFFFFUL;
				maximum = 0;
				for (i = 0; i < 64; i++)
				{
					u32 start = Mars_GetFRTCounter();
					u32 elapsed;
					HwMdSegaCDCommand(SCD_OP_SUBCPU_PING, i);
					elapsed = (u32)Mars_GetFRTCounter() - start;
					if (elapsed < minimum) minimum = elapsed;
					if (elapsed > maximum) maximum = elapsed;
					total += elapsed;
				}
				average = total / 64;
				complete = 1;
				old = scd_pad();
			}
		}
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stream_overlay(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, failed = 0;
	int camera = 0, direction = 1, priority = 0;
	u16 poll = 0;

	canvas_pitch = 320;
	canvas_yaw = 224;
	HwMdClearPlanes();
	HwMdSetPlaneBitmap(0, mdtest_map_Map.mdPlaneA.bitmap);
	HwMdSetPlaneBitmap(1, mdtest_map_Map.mdPlaneB.bitmap);
	Hw32xSetPalette(mdtest_Palette);
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_68K | MARS_224_LINES | MARS_VDP_MODE_256;
	Hw32xScreenFlip(0);
	init_tilemap(&tm, &mdtest_map_Map, (u8 **)mdtest_Reslist);
	Hw32xSetBGOverlayPriorityBit(0);
	Hw32xSetFGOverlayPriorityBit(1);
	Hw32xSetPalettePriorityAliases(17, 1, 10, 0);

	while (!done)
	{
		Hw32xFlipWait();
		camera += direction;
		if (camera <= 0 || camera >= 128) direction = -direction;
		HwMdHScrollPlane(0, camera);
		HwMdVScrollPlane(1, -camera / 2);
		canvas_rebuild_id++;
		draw_tilemap(&tm, camera << 16, 0, 0, NULL, NULL);
		loadTextPalette();
		drawTextwHighlight("CD Streaming + 32X Overlay", 40, 36,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight(running ? "STREAMING" :
			(complete ? (failed ? "READ ERROR" : "COMPLETE") : "PRESS A"),
			112, 48, running ? fontColorRed : fontColorWhite,
			running ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("A:stream  Start:exit", 80, 176,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight(priority ? "B:32X priority" : "B:MD priority", 88, 188,
			fontColorWhite, fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		if (running)
		{
			poll = HwMdSegaCDCommand(SCD_OP_DATA_POLL, 0);
			if (poll)
			{
				running = 0;
				complete = 1;
				failed = poll != 1;
				old = scd_pad();
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_A) &&
			HwMdSegaCDCommand(SCD_OP_INIT, 0))
		{
			HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, SCD_DEFAULT_DATA_LBA);
			running = HwMdSegaCDCommand(SCD_OP_DATA_START, SCD_STREAM_SECTORS);
			complete = 0;
			failed = 0;
		}
		if (pressed & SEGA_CTRL_B)
		{
			priority ^= 1;
			MARS_VDP_DISPMODE = (priority ? MARS_VDP_PRIO_32X : MARS_VDP_PRIO_68K) |
				MARS_224_LINES | MARS_VDP_MODE_256;
		}
		if (!running && (pressed & SEGA_CTRL_START)) done = 1;
	}
	HwMdClearPlanes();
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
	screenFadeOut(1);
}

static void __attribute__((unused)) menu_segacd32x(void)
{
	static const char *items[] = {
		"CD-to-32X Integrity", "Streaming Bandwidth", "Combined Audio Mixer",
		"CDDA/32X A/V Sync", "Word RAM Handoff Stress",
		"Multi-CPU Latency", "CD Streaming Overlay", "Help", "Back"
	};
	static const int item_y[] = {56, 64, 80, 88, 104, 112, 120, 144, 152};
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0;

	initMainBG();
	Hw32xScreenFlip(0);
	while (!done)
	{
		int i;
		u16 status;
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();
		drawTextwHighlight("Sega CD 32X Tests", 56, 36,
			fontColorGreen, fontColorGreenHighlight);
		status = HwMdSegaCDCommand(SCD_OP_STATUS, 0);
		drawTextwHighlight((status & SCD_STATUS_PRESENT) ? "Detected" : "Not detected",
			216, 36, (status & SCD_STATUS_PRESENT) ? fontColorGreen : fontColorRed,
			(status & SCD_STATUS_PRESENT) ? fontColorGreenHighlight : fontColorRedHighlight);
		for (i = 0; i < 9; i++)
			drawTextwHighlight((char *)items[i], 40, item_y[i],
				selection == i ? fontColorRed : fontColorWhite,
				selection == i ? fontColorRedHighlight : fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_DOWN) selection = selection == 8 ? 0 : selection + 1;
		if (pressed & SEGA_CTRL_UP) selection = selection ? selection - 1 : 8;
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
		if (scd_help_pressed(button, pressed))
		{
			DrawHelp(HELP_SEGACD32X);
			initMainBG();
			Hw32xScreenFlip(0);
			old = scd_pad();
		}
		if (pressed & SEGA_CTRL_A)
		{
			screenFadeOut(1);
			switch (selection)
			{
				case 0: scd_integrity_test(); break;
				case 1: scd_stream_test(); break;
				case 2: scd_combined_audio(); break;
				case 3: scd_av_sync(); break;
				case 4: scd_wordram_stress(); break;
				case 5: scd_latency_test(); break;
				case 6: scd_stream_overlay(); break;
				case 7: DrawHelp(HELP_SEGACD32X); break;
				case 8: done = 1; break;
			}
			if (!done)
			{
				marsVDP256Start();
				initMainBG();
				Hw32xScreenFlip(0);
			}
			old = scd_pad();
		}
	}
	screenFadeOut(1);
}

void menu_segacd(void)
{
	static const char *items[] = {
		"BIOS CRC and info", "Check HINT Register", "Check Flag Register",
		"Check Comm Registers", "Program RAM Check", "Word RAM Check",
		"Memory Viewer", "PCM RAM Check", "PCM Sound Check",
		"Help", "Back to Main Menu"
	};
	static const int item_y[] = {56, 72, 80, 88, 104, 112, 120, 136, 144, 160, 168};
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0;

	initMainBGwGil();
	Hw32xScreenFlip(0);
	while (!done)
	{
		int i;
		u16 status;
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();
		status = HwMdSegaCDCommand(SCD_OP_STATUS, 0);
		drawTextwHighlight((status & SCD_STATUS_PRESENT) ? "Sega CD Tests" :
			"Sega CD not detected", (status & SCD_STATUS_PRESENT) ? 112 : 88, 32,
			fontColorRed, fontColorRedHighlight);
		for (i = 0; i < 11; i++)
			drawTextwHighlight((char *)items[i], 40, item_y[i],
				selection == i ? fontColorRed : fontColorWhite,
				selection == i ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawResolution();
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_DOWN) selection = selection == 10 ? 0 : selection + 1;
		if (pressed & SEGA_CTRL_UP) selection = selection ? selection - 1 : 10;
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
		if (scd_help_pressed(button, pressed))
		{
			DrawHelp(HELP_SEGACD);
			initMainBGwGil();
			Hw32xScreenFlip(0);
			old = scd_pad();
		}
		if (pressed & SEGA_CTRL_A)
		{
			screenFadeOut(1);
			switch (selection)
			{
				case 0: scd_bios_info(); break;
				case 1: scd_hint_test(); break;
				case 2: scd_flag_test(); break;
				case 3: scd_comm_test(); break;
				case 4: scd_program_ram_test(); break;
				case 5: scd_word_ram_test(); break;
				case 6: scd_memory_viewer(); break;
				case 7: scd_pcm_ram_test(); break;
				case 8: scd_sound_test(); break;
				case 9: DrawHelp(HELP_SEGACD); break;
				case 10: done = 1; break;
			}
			if (!done)
			{
				marsVDP256Start();
				initMainBGwGil();
				Hw32xScreenFlip(0);
			}
			old = scd_pad();
		}
	}
	screenFadeOut(1);
}
