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
#include "perf.h"

extern const u8 mdtest_Palette[];
extern u8 *mdtest_Reslist[];
extern const dtilemap_t mdtest_map_Map;

#define SCD_DEFAULT_DATA_LBA 16
#define SCD_INTEGRITY_WORDS  1024
#define SCD_STREAM_SECTORS   64
#define SCD_TEST_BEGIN       0x8000
#define SCD_PROGRAM_BANK_REG 0x0010
#define SCD_PROGRAM_FAST     0x0020
#define SCD_PROGRAM_PATTERN  0x0040
#define SCD_UNCACHED_CURRENT_FB \
	(*(volatile u16 *)((uintptr_t)&currentFB | 0x20000000u))

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
	u32 timeout = 50000000UL;

	while (MARS_SYS_COMM0 && timeout)
		timeout--;
	if (!timeout)
	{
		MARS_SYS_COMM0 = 0;
		return 0;
	}
	MARS_SYS_COMM2 = argument;
	MARS_SYS_COMM0 = 0x1600 | (operation & 0x00FF);
	timeout = 50000000UL;
	while (MARS_SYS_COMM0 && timeout)
		timeout--;
	if (!timeout)
	{
		MARS_SYS_COMM0 = 0;
		return 0;
	}
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

static const char *scd_find_cdda_track(int preferred, int *selected)
{
	u32 disc;
	u16 status;
	int first, last, track;

	if (!HwMdSegaCDCommand(SCD_OP_INIT, 0))
		return "Sega CD initialization failed";
	if (!HwMdSegaCDCommand(SCD_OP_DISC_INFO, 0))
		return "Could not read CD status";
	disc = HwMdSegaCDResult0();
	status = disc >> 16;
	if (status == 0x0B)
		return "CD tray is open";
	if (status == 0x0C)
		return "No CD in tray";
	if (status == 0x0A)
		return "Invalid or unread CD TOC";
	if (status != 1)
		return "CD drive is not ready";

	first = (disc >> 8) & 0xFF;
	last = disc & 0xFF;
	if (!first || first == 0xFF || last < first)
		return "No tracks reported by CD";
	if (preferred >= first && preferred <= last &&
		HwMdSegaCDCommand(SCD_OP_TRACK_INFO, preferred) &&
		(HwMdSegaCDResult2() & 0xFF) == 0)
	{
		*selected = preferred;
		return NULL;
	}
	for (track = first; track <= last; track++)
		if (track != preferred &&
			HwMdSegaCDCommand(SCD_OP_TRACK_INFO, track) &&
			(HwMdSegaCDResult2() & 0xFF) == 0)
		{
			*selected = track;
			return NULL;
		}
	return "No CD-DA tracks found";
}

static void scd_screen_init(void)
{
	marsVDP256Start();
	initMainBG();
	Hw32xScreenFlip(0);
}

static void scd_draw_title(const char *title)
{
	int x = (320 - strlen(title) * 8) / 2;

	drawMainBG();
	drawTextwHighlight((char *)title, x, 32,
		fontColorGreen, fontColorGreenHighlight);
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
	else if (color == fontColorBlue)
		highlight = fontColorBlueHighlight;

	drawTextwHighlight((char *)label, x, y, color, highlight);
	strcpy(buffer, " 0x");
	intToHex(value, buffer + 3, digits);
	drawTextwHighlight(buffer, x + strlen(label) * 8, y,
		fontColorWhite, fontColorWhiteHighlight);
}

static int scd_text_highlight(int color)
{
	if (color == fontColorGreen)
		return fontColorGreenHighlight;
	if (color == fontColorRed)
		return fontColorRedHighlight;
	if (color == fontColorBlue)
		return fontColorBlueHighlight;
	return fontColorWhiteHighlight;
}

static void scd_draw_text_field(const char *label, const char *value,
	int label_x, int value_x, int y, int value_color)
{
	drawTextwHighlight((char *)label, label_x, y,
		fontColorGreen, fontColorGreenHighlight);
	drawTextwHighlight((char *)value, value_x, y,
		value_color, scd_text_highlight(value_color));
}

static void scd_draw_number_field(const char *label, u32 value,
	int label_x, int value_x, int y, int value_color)
{
	char buffer[16];

	uintToStr(value, buffer, 1);
	scd_draw_text_field(label, buffer, label_x, value_x, y, value_color);
}

static void scd_draw_signed_field(const char *label, s32 value,
	int label_x, int value_x, int y, int value_color)
{
	char buffer[16];

	intToStr(value, buffer, 1);
	scd_draw_text_field(label, buffer, label_x, value_x, y, value_color);
}

static void scd_draw_prompt(const char *message)
{
	int color = message ? fontColorRed : fontColorGreen;
	int highlight = message ? fontColorRedHighlight : fontColorGreenHighlight;

	drawTextwHighlight((char *)(message ? message :
		"     PRESS ANY BUTTON     "), 64, 176,
		color, highlight);
}

static int scd_draw_page(void)
{
	return (SCD_UNCACHED_CURRENT_FB ^ 1) & 1;
}

static void scd_restore_region(int x, int y, int w, int h)
{
	draw_dirtyrect(&tm, x, y, w, h);
	draw_tilemap(&tm, 0, 0, 0, NULL, NULL);
	draw_setScissor(0, 0, 320, 224);
}

static void scd_wait_vblank(void)
{
	u32 tick = Hw32xGetTicks();
	while (Hw32xGetTicks() == tick);
}

static void scd_wait_one_second(void)
{
	Hw32xDelay(Hw32xDetectPAL() ? 50 : 60);
}

static int scd_any_button(u16 pressed)
{
	return pressed & (SEGA_CTRL_A | SEGA_CTRL_B | SEGA_CTRL_C |
		SEGA_CTRL_START);
}

static void scd_draw_register_frame(const char *title, u32 address,
	const u16 *patterns, const u16 *reads, const u8 *passed,
	int completed, int digits, int prompt, int state, int page_state[2])
{
	int page = scd_draw_page();
	int i;

	drawMainBG();
	if (page_state[page] == state)
		return;

	scd_restore_region(40, 56, 240, 136);
	scd_draw_genesis_hex(title, address, 8, 56, 64, fontColorGreen);
	for (i = 0; i < completed; i++)
	{
		int y = 80 + i * 16;
		scd_draw_genesis_hex("Setting to", patterns[i], digits, 88, y,
			fontColorGreen);
		if (passed[i])
			drawTextwHighlight("W/R OK", 128, y + 8,
				fontColorWhite, fontColorWhiteHighlight);
		else
			scd_draw_genesis_hex("FAILED", reads[i], digits,
				digits == 2 ? 112 : 104, y + 8, fontColorRed);
	}
	if (prompt)
		scd_draw_prompt(NULL);
	page_state[page] = state;
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
	u16 reads[4] = { 0, 0, 0, 0 };
	u8 passed[4] = { 0, 0, 0, 0 };
	u16 button, pressed, old = 0xFFFF;
	u32 detail;
	int page_state[2] = { -1, -1 };
	int done = 0, completed = 0, good = 0, i;

	scd_screen_init();
	for (i = 0; i < 3; i++)
	{
		passed[i] = HwMdSegaCDCommand(SCD_OP_FLAG_TEST,
			i | (i == 0 ? SCD_TEST_BEGIN : 0)) != 0;
		detail = HwMdSegaCDResult1();
		reads[i] = detail & 0xFF;
		good += passed[i];
		completed++;

		Hw32xFlipWait();
		scd_draw_register_frame("Flag register", 0xA1200E, patterns,
			reads, passed, completed, 2, 0, completed, page_state);
		Hw32xScreenFlip(0);
		scd_wait_one_second();
	}
	if (good)
	{
		passed[3] = HwMdSegaCDCommand(SCD_OP_FLAG_TEST, 3) != 0;
		detail = HwMdSegaCDResult1();
		reads[3] = detail & 0xFF;
		completed++;

		Hw32xFlipWait();
		scd_draw_register_frame("Flag register", 0xA1200E, patterns,
			reads, passed, completed, 2, 0, completed, page_state);
		Hw32xScreenFlip(0);
		scd_wait_one_second();
	}

	for (i = 0; i < 2; i++)
	{
		Hw32xFlipWait();
		scd_draw_register_frame("Flag register", 0xA1200E, patterns,
			reads, passed, completed, 2, 1, 0x100 | completed,
			page_state);
		Hw32xScreenFlip(0);
	}

	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_register_frame("Flag register", 0xA1200E, patterns,
			reads, passed, completed, 2, 1, 0x100 | completed,
			page_state);
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
	u16 reads[4];
	u8 passed[4];
	u16 button, pressed, old = 0xFFFF;
	u32 detail;
	int page, i;

	scd_screen_init();
	for (page = 0; page < 8; page++)
	{
		int page_state[2] = { -1, -1 };
		int completed = 0;
		int good = 0;
		int done = 0;

		for (i = 0; i < 4; i++)
		{
			if (i == 3 && !good)
				break;
			passed[i] = HwMdSegaCDCommand(SCD_OP_COMM_TEST,
				(page << 8) | i |
				(page == 0 && i == 0 ? SCD_TEST_BEGIN : 0)) != 0;
			detail = HwMdSegaCDResult1();
			reads[i] = detail & 0xFFFF;
			if (i < 3)
				good += passed[i];
			completed++;

			Hw32xFlipWait();
			scd_draw_register_frame("Command Register",
				0xA12010 + page * 2, patterns, reads, passed,
				completed, 4, 0, completed, page_state);
			Hw32xScreenFlip(0);
			scd_wait_one_second();
		}

		for (i = 0; i < 2; i++)
		{
			Hw32xFlipWait();
			scd_draw_register_frame("Command Register",
				0xA12010 + page * 2, patterns, reads, passed,
				completed, 4, 1, 0x100 | completed, page_state);
			Hw32xScreenFlip(0);
		}

		old = 0xFFFF;
		while (!done)
		{
			Hw32xFlipWait();
			scd_draw_register_frame("Command Register",
				0xA12010 + page * 2, patterns, reads, passed,
				completed, 4, 1, 0x100 | completed, page_state);
			Hw32xScreenFlip(0);

			button = scd_pad();
			pressed = button & ~old;
			old = button;
			if (scd_any_button(pressed))
				done = 1;
		}
	}
	screenFadeOut(1);
}

static void scd_draw_ram_patterns(const int status[4], const u32 address[4],
	const u16 read[4], int completed)
{
	static const u16 patterns[] = { 0xAAAA, 0x5555, 0xFFFF, 0x0000 };
	int i;

	for (i = 0; i < completed; i++)
	{
		int y = 64 + i * 24;
		scd_draw_genesis_hex("Setting to", patterns[i], 4, 96, y,
			fontColorGreen);
		if (status[i] > 0)
			drawTextwHighlight("W/R OK", 128, y + 8,
				fontColorWhite, fontColorWhiteHighlight);
		else
		{
			scd_draw_genesis_hex("Failed @", address[i], 7, 80, y + 8,
				fontColorRed);
			scd_draw_genesis_hex("GOT", read[i], 4, 112, y + 16,
				fontColorRed);
		}
	}
}

static void scd_draw_ram_frame(const char *title, u32 title_address,
	int title_digits, int title_x, int bank, const int status[4],
	const u32 address[4], const u16 read[4], int completed, int prompt,
	int state, int page_state[2])
{
	int page = scd_draw_page();

	drawMainBG();
	if (page_state[page] == state)
		return;
	scd_restore_region(32, 24, 272, 168);
	scd_draw_genesis_hex(title, title_address, title_digits, title_x, 32,
		fontColorGreen);
	if (bank >= 0)
		scd_draw_genesis_hex("Bank Test", bank, 2, 104, 48,
			fontColorGreen);
	scd_draw_ram_patterns(status, address, read, completed);
	if (prompt)
		scd_draw_prompt(NULL);
	page_state[page] = state;
}

static void scd_word_ram_test(void)
{
	int status[4] = { -1, -1, -1, -1 };
	u32 address[4] = { 0, 0, 0, 0 };
	u16 read[4] = { 0, 0, 0, 0 };
	u16 button, pressed, old = 0xFFFF;
	int page_state[2] = { -1, -1 };
	int done = 0, i;

	scd_screen_init();
	for (i = 0; i < 4; i++)
	{
		u32 detail;
		status[i] = HwMdSegaCDCommand(SCD_OP_WORD_RAM_TEST,
			i | (i == 0 ? SCD_TEST_BEGIN : 0)) ? 1 : 0;
		address[i] = HwMdSegaCDResult0();
		detail = HwMdSegaCDResult1();
		read[i] = detail & 0xFFFF;

		Hw32xFlipWait();
		scd_draw_ram_frame("WORD RAM", 0x600000, 7, 88, -1,
			status, address, read, i + 1, 0, i + 1, page_state);
		Hw32xScreenFlip(0);
	}

	for (i = 0; i < 2; i++)
	{
		Hw32xFlipWait();
		scd_draw_ram_frame("WORD RAM", 0x600000, 7, 88, -1,
			status, address, read, 4, 1, 0x104, page_state);
		Hw32xScreenFlip(0);
	}

	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_ram_frame("WORD RAM", 0x600000, 7, 88, -1,
			status, address, read, 4, 1, 0x104, page_state);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			done = 1;
	}
	screenFadeOut(1);
}

static void scd_memory_viewer(void);

static u16 scd_wait_program_bank_status(int passed, u32 detail,
	int page_state[2])
{
	u16 button, pressed, old = 0xFFFF;
	int i;

	for (i = 0; i < 2; i++)
	{
		int page;
		Hw32xFlipWait();
		drawMainBG();
		page = scd_draw_page();
		if (page_state[page] != passed)
		{
			scd_restore_region(32, 24, 272, 168);
			scd_draw_genesis_hex("Program RAM", 0x420000, 6, 80, 32,
				fontColorGreen);
			if (passed)
				scd_draw_genesis_hex("Bank Register OK", 0xA12002, 7,
					56, 112, fontColorWhite);
			else
			{
				scd_draw_genesis_hex("Bank Switch FAIL", 0xA12002, 8,
					40, 64, fontColorRed);
				scd_draw_genesis_hex("SET", detail >> 16, 4, 112, 72,
					fontColorRed);
				scd_draw_genesis_hex("GOT", detail & 0xFFFF, 4, 112, 80,
					fontColorRed);
				drawTextwHighlight("Only one bank will be checked", 40, 88,
					fontColorWhite, fontColorWhiteHighlight);
			}
			scd_draw_prompt(NULL);
			page_state[page] = passed;
		}
		Hw32xScreenFlip(0);
	}

	for (;;)
	{
		Hw32xFlipWait();
		drawMainBG();
		Hw32xScreenFlip(0);
		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			return pressed;
	}
}

static u16 scd_wait_program_fast_failure(int page_state[2])
{
	u16 button, pressed, old = 0xFFFF;
	int state = 2;

	for (;;)
	{
		int page;
		Hw32xFlipWait();
		drawMainBG();
		page = scd_draw_page();
		if (page_state[page] != state)
		{
			scd_restore_region(32, 56, 272, 136);
			drawTextwHighlight("Fast Check Failed. The register", 32, 80,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("did W/R OK, but bank switch was", 32, 88,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("either not done or bad RAM/lines", 32, 96,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("Try Memory Viewer by pressing A", 32, 112,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("and use C to switch banks. Must", 32, 120,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("have AA, BB, CC & DD at corners", 32, 128,
				fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("and 00 elsewhere.", 32, 136,
				fontColorWhite, fontColorWhiteHighlight);
			scd_draw_prompt("A:memview B:exit C:test");
			page_state[page] = state;
		}
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			return pressed;
	}
}

static void scd_program_ram_test(void)
{
	int status[4];
	u32 address[4];
	u16 read[4];
	int page_state[2] = { -1, -1 };
	u16 pressed;
	int banks, bank, i, passed;

	scd_screen_init();
	passed = HwMdSegaCDCommand(SCD_OP_PROGRAM_RAM_TEST,
		SCD_TEST_BEGIN | SCD_PROGRAM_BANK_REG) != 0;
	banks = passed ? 4 : 1;
	scd_wait_program_bank_status(passed, HwMdSegaCDResult1(), page_state);
	page_state[0] = page_state[1] = -1;

	if (passed && !HwMdSegaCDCommand(SCD_OP_PROGRAM_RAM_TEST,
		SCD_PROGRAM_FAST))
	{
		pressed = scd_wait_program_fast_failure(page_state);
		if (pressed & SEGA_CTRL_A)
		{
			screenFadeOut(1);
			scd_memory_viewer();
			return;
		}
		if (pressed & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			return;
		}
	}

	for (bank = 0; bank < banks; bank++)
	{
		u16 button, old = 0xFFFF;
		int done = 0;

		for (i = 0; i < 4; i++)
		{
			u32 detail;
			status[i] = HwMdSegaCDCommand(SCD_OP_PROGRAM_RAM_TEST,
				SCD_PROGRAM_PATTERN | (bank << 8) | i) ? 1 : 0;
			address[i] = HwMdSegaCDResult0();
			detail = HwMdSegaCDResult1();
			read[i] = detail & 0xFFFF;

			Hw32xFlipWait();
			scd_draw_ram_frame("Program RAM", 0x420000, 6, 80, bank,
				status, address, read, i + 1, 0,
				0x200 + bank * 8 + i, page_state);
			Hw32xScreenFlip(0);
		}

		for (i = 0; i < 2; i++)
		{
			Hw32xFlipWait();
			scd_draw_ram_frame("Program RAM", 0x420000, 6, 80, bank,
				status, address, read, 4, 1, 0x300 + bank, page_state);
			Hw32xScreenFlip(0);
		}

		while (!done)
		{
			Hw32xFlipWait();
			scd_draw_ram_frame("Program RAM", 0x420000, 6, 80, bank,
				status, address, read, 4, 1, 0x300 + bank, page_state);
			Hw32xScreenFlip(0);
			button = scd_pad();
			pressed = button & ~old;
			old = button;
			if (scd_any_button(pressed))
				done = 1;
		}
		page_state[0] = page_state[1] = -1;
	}
	screenFadeOut(1);
}

typedef struct scd_bios_id_t {
	u32 crc;
	const char *name;
} scd_bios_id_t;

typedef struct scd_bios_swapped_t {
	u32 original_crc;
	u32 mame_crc;
} scd_bios_swapped_t;

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

static const scd_bios_swapped_t scd_bios_swapped[] = {
	{ 0x9BCE40B2, 0x69ED6CCD }, { 0x2EA250C0, 0xDFA95EE9 },
	{ 0x9D2DA8F2, 0xE2E70BC8 }, { 0x550F30BB, 0x47FA8EAC },
	{ 0xD21FE71D, 0xCF1D926B }, { 0x290F8E33, 0x6AEDAEA7 },
	{ 0x00EEDB3A, 0x885B4AD2 }, { 0x3B10CF41, 0x3885B28C },
	{ 0x50CD3D23, 0x2365BFBB }, { 0x474AAA44, 0x046B1332 },
	{ 0x1493522C, 0xFA64FC8E }, { 0x529AC15A, 0x2BF760AF },
	{ 0xF18DDE5B, 0x03134289 }, { 0x79F85384, 0x3773D5AA },
	{ 0xC6D10268, 0x2461B5ED }, { 0x4BE18FF6, 0xE0A6179B },
	{ 0xDD6CC972, 0x1E4344E6 }, { 0x0507B590, 0xCB76F114 },
	{ 0x4D5CB8DA, 0x53F1757C }, { 0xC1AA217F, 0x391A80D2 },
	{ 0x2E49D72C, 0x9AAB8FE3 }, { 0x8052C7A0, 0x1E628066 },
	{ 0xAACB851E, 0x527E310B }, { 0xD48C44B5, 0xD1EE6282 },
	{ 0x8AF65F58, 0x0C653035 }, { 0x2B19972F, 0x9E13FDA6 },
	{ 0, 0 }
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

static u32 scd_bios_mame_crc(u32 crc)
{
	int i = 0;
	while (scd_bios_swapped[i].original_crc)
	{
		if (scd_bios_swapped[i].original_crc == crc)
			return scd_bios_swapped[i].mame_crc;
		i++;
	}
	return 0;
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
		16, 16, 17, 15, 16, 48, 15, -1, 16, -4, -4, -4, -4,
		-2, -2, -4, -4, -12, -40, 16, 0
	};
	int panel_state[2] = { -1, -1 };
	int i, j = 0, offset = 0x0100;

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
		int state = present ? (calculated ? 2 : 0) : 3;
		int page;

		Hw32xFlipWait();
		drawMainBG();
		page = scd_draw_page();
		if (panel_state[page] != state)
		{
			scd_restore_region(32, 144, 256, 48);
			panel_state[page] = state;
		}
		scd_draw_genesis_hex("Sega CD BIOS Data at", 0x400000, 8, 48, 32,
			fontColorWhite);
		if (present)
		{
			const char *name;
			for (i = 0; i < 9; i++)
				drawTextwHighlight(fields[i], 96, 64 + i * 8,
					fontColorWhite, fontColorWhiteHighlight);
			if (calculated)
			{
				u32 mame_crc;
				scd_draw_genesis_hex("CD BIOS CRC32:", crc, 8, 48, 152,
					fontColorGreen);
				name = scd_bios_name(crc);
				mame_crc = scd_bios_mame_crc(crc);
				if (name)
				{
					if (mame_crc)
						scd_draw_genesis_hex("MAME CRC32:", mame_crc, 8,
							72, 160, fontColorGreen);
					drawTextwHighlight((char *)name, 112,
						mame_crc ? 168 : 160,
						fontColorWhite, fontColorWhiteHighlight);
				}
				else
					drawTextwHighlight("Unknown BIOS, please report CRC", 32, 160,
						fontColorRed, fontColorRedHighlight);
				scd_draw_prompt(NULL);
			}
			else
				drawTextwHighlight("'A' for CRC 'B' to exit", 64, 176,
					fontColorGreen, fontColorGreenHighlight);
		}
		else
		{
			drawTextwHighlight("No BIOS found", 96, 160,
				fontColorRed, fontColorRedHighlight);
			scd_draw_prompt(NULL);
		}
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (present && (pressed & SEGA_CTRL_A) && !calculated)
		{
			int page;

			Hw32xFlipWait();
			drawMainBG();
			page = scd_draw_page();
			if (panel_state[page] != 1)
			{
				scd_restore_region(32, 144, 256, 48);
				panel_state[page] = 1;
			}
			scd_draw_genesis_hex("Sega CD BIOS Data at", 0x400000, 8,
				48, 32, fontColorWhite);
			for (i = 0; i < 9; i++)
				drawTextwHighlight(fields[i], 96, 64 + i * 8,
					fontColorWhite, fontColorWhiteHighlight);
			drawTextwHighlight("Calculating, please wait", 56, 152,
				fontColorGreen, fontColorGreenHighlight);
			Hw32xScreenFlip(0);

			if (HwMdSegaCDCommand(SCD_OP_BIOS_WORD, 0xFFFF))
			{
				crc = HwMdSegaCDResult0();
				calculated = 1;
				old = scd_pad();
			}
		}
		else if (calculated && panel_state[0] == 2 && panel_state[1] == 2 &&
			scd_any_button(pressed))
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
	HwMdSegaCDCommand(SCD_OP_RESET, 0);
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

		scd_wait_vblank();

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

static int scd_initialize_test(int load_pcm);
static int scd_initialize_pcm_sound(void);

#define SCD_FLOAT_BORDER    214
#define SCD_FLOAT_DARK      215
#define SCD_FLOAT_FILL      216
#define SCD_FLOAT_HIGHLIGHT 217

static void scd_set_float_palette(void)
{
	setColor(SCD_FLOAT_BORDER, 9, 9, 9);
	setColor(SCD_FLOAT_DARK, 13, 13, 13);
	setColor(SCD_FLOAT_FILL, 18, 18, 18);
	setColor(SCD_FLOAT_HIGHLIGHT, 31, 31, 31);
}

static void scd_draw_float_box(void)
{
	static u8 pixels[128 * 80] ATTR_CACHE_ALIGNED;
	static int initialized = 0;
	int row, column;

	if (!initialized)
	{
		for (row = 0; row < 80; row++)
			for (column = 0; column < 128; column++)
			{
				u8 color = SCD_FLOAT_FILL;

				if (row == 0 || row == 79 || column == 0 || column == 127)
					color = SCD_FLOAT_BORDER;
				else if (row < 8 || row >= 72)
					color = SCD_FLOAT_DARK;
				else if (row == 9 || row == 70)
					color = SCD_FLOAT_HIGHLIGHT;
				pixels[row * 128 + column] = color;
			}
		initialized = 1;
	}
	draw_sprite(96, 72, 128, 80, pixels,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
}

static int scd_pcm_ram_menu(int selection, int page_state[2])
{
	static const char *items[] = {
		"PCM RAM Check", "Full Test", "Full w/1s wait",
		"Per Bank", "Bank w/1s wait", "return"
	};
	static const int item_x[] = { 104, 120, 104, 128, 104, 136 };
	u16 button, pressed, old = 0xFFFF;
	int i, done = 0;

	scd_set_float_palette();

	while (!done)
	{
		int page;
		Hw32xFlipWait();
		drawMainBG();
		page = scd_draw_page();
		if (page_state[page] != 0)
		{
			fillRect8(0, 0, 320, 224, fontBackgroundColorBlack);
			page_state[page] = 0;
		}
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
			selection = 0;
			done = 1;
		}
	}
	return selection;
}

static int scd_not_detected_menu(void)
{
	static const char *title = "SCD not detected";
	static const char *items[] = { "Continue", "Return" };
	static const int item_x[] = { 128, 136 };
	u16 button, pressed, old = 0xFFFF;
	int selection = 2;
	int page_state[2] = { -1, -1 };

	scd_screen_init();
	scd_set_float_palette();
	for (;;)
	{
		int i;
		int page;

		Hw32xFlipWait();
		drawMainBG();
		page = scd_draw_page();
		if (page_state[page] != 0)
		{
			fillRect8(0, 0, 320, 224, fontBackgroundColorBlack);
			page_state[page] = 0;
		}
		scd_draw_float_box();
		drawTextwHighlight((char *)title, 96, 72,
			fontColorGreen, fontColorGreenHighlight);
		for (i = 0; i < 2; i++)
			drawTextwHighlight((char *)items[i], item_x[i], 104 + i * 8,
				selection == i + 1 ? fontColorRed : fontColorWhite,
				selection == i + 1 ? fontColorRedHighlight :
					fontColorWhiteHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & (SEGA_CTRL_UP | SEGA_CTRL_DOWN))
			selection = selection == 1 ? 2 : 1;
		if (pressed & SEGA_CTRL_A)
			return selection == 1;
		if (pressed & SEGA_CTRL_START)
			return 1;
	}
}

static void scd_pcm_run_pattern(int target, int wait, u16 pattern,
	scd_pcm_result_t *result)
{
	u16 argument = ((target & 0xFF) << 8) | pattern;

	result->status = -1;
	result->bank = target == 0xFF ? 0 : target;
	result->offset = 0;
	result->read = 0;
	if (HwMdSegaCDCommand(SCD_OP_PCM_RAM_SET, argument))
	{
		u32 detail;
		if (wait)
			Hw32xDelay(60);
		result->status = HwMdSegaCDCommand(
			SCD_OP_PCM_RAM_COMPARE, argument) ? 1 : 0;
		if (!result->status)
		{
			detail = HwMdSegaCDResult0();
			result->bank = detail >> 16;
			result->offset = detail & 0xFFFF;
			result->read = HwMdSegaCDResult2();
		}
	}
}

static void scd_draw_pcm_results(const scd_pcm_result_t *results, int completed)
{
	static const u16 patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
	int i;

	for (i = 0; i < completed; i++)
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

static void scd_draw_pcm_test_frame(int bank,
	const scd_pcm_result_t *results, int completed, int prompt,
	int state, int page_state[2])
{
	int page = scd_draw_page();

	drawMainBG();
	if (page_state[page] != state)
	{
		scd_restore_region(0, 0, 320, 224);
		page_state[page] = state;
	}
	if (bank < 0)
		scd_draw_genesis_hex("PCM RAM Test (Sega CD)", 0x2000, 4,
			40, 32, fontColorGreen);
	else
		scd_draw_genesis_hex("PCM RAM Test (Sega CD) Bank", bank, 2,
			32, 32, fontColorGreen);
	scd_draw_pcm_results(results, completed);
	if (prompt)
		drawTextwHighlight(bank < 0 ? "'A' to test 'B' to exit" :
			"'A' for next 'B' to exit", 64, 176,
			fontColorGreen, fontColorGreenHighlight);
}

static void scd_run_pcm_test_page(int bank, int wait,
	scd_pcm_result_t *results, int page_state[2])
{
	static const u16 patterns[] = { 0x55, 0xAA, 0xFF, 0x00 };
	int i;

	myMemSet(results, 0, sizeof(scd_pcm_result_t) * 4);
	page_state[0] = page_state[1] = -1;
	Hw32xFlipWait();
	scd_draw_pcm_test_frame(bank, results, 0, 0,
		0x300 + (bank + 1) * 8, page_state);
	Hw32xScreenFlip(0);
	for (i = 0; i < 4; i++)
	{
		scd_pcm_run_pattern(bank < 0 ? 0xFF : bank, wait,
			patterns[i], &results[i]);
		Hw32xFlipWait();
		scd_draw_pcm_test_frame(bank, results, i + 1, 0,
			0x300 + (bank + 1) * 8 + i + 1, page_state);
		Hw32xScreenFlip(0);
	}
}

static void scd_pcm_ram_test(void)
{
	u16 button, pressed, old;
	int type = 1, exit_menu = 0;
	int page_state[2] = { -1, -1 };
	scd_pcm_result_t results[4];

	if (!scd_initialize_pcm_sound())
	{
		screenFadeOut(1);
		return;
	}

	scd_screen_init();
	while (!exit_menu)
	{
		type = scd_pcm_ram_menu(type, page_state);
		if (type == 5)
			break;
		if (type == 0)
			continue;

		if (type == 1 || type == 2)
		{
			int exit_test = 0;
			while (!exit_test)
			{
				int done = 0;
				scd_run_pcm_test_page(-1, type == 2, results, page_state);
				old = 0xFFFF;
				while (!done)
				{
					Hw32xFlipWait();
					scd_draw_pcm_test_frame(-1, results, 4, 1,
						0x400, page_state);
					Hw32xScreenFlip(0);
					button = scd_pad();
					pressed = button & ~old;
					old = button;
					if (pressed & SEGA_CTRL_B)
					{
						done = 1;
						exit_test = 1;
					}
					else if (scd_any_button(pressed))
						done = 1;
				}
			}
		}
		else
		{
			int bank = 0, exit_test = 0;
			while (!exit_test && bank < 16)
			{
				int done = 0;
				scd_run_pcm_test_page(bank, type == 4, results, page_state);
				old = 0xFFFF;
				while (!done)
				{
					Hw32xFlipWait();
					scd_draw_pcm_test_frame(bank, results, 4, 1,
						0x500 + bank, page_state);
					Hw32xScreenFlip(0);
					button = scd_pad();
					pressed = button & ~old;
					old = button;
					if (pressed & SEGA_CTRL_B)
					{
						done = 1;
						exit_test = 1;
					}
					else if (scd_any_button(pressed))
					{
						bank++;
						done = 1;
					}
				}
			}
		}
	}
	HwMdSegaCDCommand(SCD_OP_RESET, 0);
	screenFadeOut(1);
}

static u16 scd_wait_boot_button(void)
{
	u16 button, pressed, old = 0xFFFF;

	for (;;)
	{
		scd_wait_vblank();
		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (scd_any_button(pressed))
			return pressed;
	}
}

static void scd_draw_pcm_boot_frame(int stage, u32 bios_address,
	u32 program_size, u32 sp_init_wait, u32 sp_main_wait,
	int communication_ok, const char *failure)
{
	drawMainBG();
	drawTextwHighlight("Sega CD Mode 1 Boot", 80, 32,
		fontColorGreen, fontColorGreenHighlight);
	if (stage >= 1)
		scd_draw_genesis_hex("SCD BIOS Detected at", bios_address, 8,
			32, 48, fontColorGreen);
	if (stage >= 2)
		scd_draw_genesis_hex("Zero&Check WORD RAM ", 0x600000, 8,
			32, 56, fontColorGreen);
	if (stage >= 3)
		scd_draw_genesis_hex("Zero&Check PRGM RAM ", 0x420000, 8,
			32, 64, fontColorGreen);
	if (stage >= 4)
		drawTextwHighlight("Sending Reset to SCD", 32, 72,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 5)
		drawTextwHighlight("Assigning WORD RAM to SCD", 32, 80,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 6)
		drawTextwHighlight("Decompressing SCD BIOS", 32, 88,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 7)
		scd_draw_genesis_hex("Copying SCD Program", program_size, 4,
			32, 96, fontColorGreen);
	if (stage >= 8)
		drawTextwHighlight("Starting SCD CPU", 32, 104,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 9)
		drawTextwHighlight("SCD CPU Started", 32, 112,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 10)
		drawTextwHighlight("Enabling Interrupts", 32, 120,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 11)
		scd_draw_genesis_hex("Got SCD SP_Init in", sp_init_wait, 3,
			32, 128, fontColorGreen);
	if (stage >= 12)
		scd_draw_genesis_hex("Got SCD SP_Main in", sp_main_wait, 3,
			32, 136, fontColorGreen);
	if (stage >= 13)
		drawTextwHighlight("Wating for reply from SCD", 32, 144,
			fontColorGreen, fontColorGreenHighlight);
	if (stage >= 14)
		drawTextwHighlight(communication_ok ?
			"Communication OK with Sega CD" : "Communication issue with SCD",
			32, 152, communication_ok ? fontColorGreen : fontColorRed,
			communication_ok ? fontColorGreenHighlight : fontColorRedHighlight);
	if (stage >= 15)
	{
		drawTextwHighlight("SCD initialized. Ready for test", 40, 168,
			fontColorWhite, fontColorWhiteHighlight);
		scd_draw_prompt(NULL);
	}
	if (failure)
	{
		int width = strlen(failure) * 8;
		int x = width < 320 ? (320 - width) / 2 : 0;
		drawTextwHighlight((char *)failure, x, stage == 0 ? 104 : 184,
			fontColorRed, fontColorRedHighlight);
		if (stage == 0)
			scd_draw_prompt(NULL);
	}
}

static void scd_show_pcm_boot(int stage, u32 bios_address, u32 program_size,
	u32 sp_init_wait, u32 sp_main_wait, int communication_ok,
	const char *failure)
{
	int i;
	for (i = 0; i < 2; i++)
	{
		Hw32xFlipWait();
		scd_draw_pcm_boot_frame(stage, bios_address, program_size,
			sp_init_wait, sp_main_wait, communication_ok, failure);
		Hw32xScreenFlip(0);
	}
}

static int scd_initialize_test(int load_pcm)
{
	u32 bios_address, program_size, sp_init_wait = 0, sp_main_wait = 0;
	u32 ping_result;
	u16 pressed;
	int initialized, communication_ok;
	const char *failure = NULL;

	scd_screen_init();
	scd_show_pcm_boot(0, 0, 0, 0, 0, 0, NULL);
	if (!HwMdSegaCDCommand(SCD_OP_INIT_INFO, SCD_INIT_INFO_BIOS_ADDRESS))
	{
		scd_show_pcm_boot(0, 0, 0, 0, 0, 0, "SCD BIOS not found");
		scd_wait_boot_button();
		return 0;
	}
	bios_address = HwMdSegaCDResult0();
	HwMdSegaCDCommand(SCD_OP_INIT_INFO, SCD_INIT_INFO_PROGRAM_SIZE);
	program_size = HwMdSegaCDResult0();
	scd_show_pcm_boot(1, bios_address, program_size, 0, 0, 0, NULL);

	scd_show_pcm_boot(2, bios_address, program_size, 0, 0, 0, NULL);
	if (!HwMdSegaCDCommand(SCD_OP_WORD_RAM_TEST, SCD_TEST_BEGIN | 3))
	{
		scd_show_pcm_boot(2, bios_address, program_size, 0, 0, 0,
			"WORD RAM failed. A to continue");
		pressed = scd_wait_boot_button();
		if (!(pressed & SEGA_CTRL_A))
			return 0;
		scd_screen_init();
		scd_show_pcm_boot(2, bios_address, program_size, 0, 0, 0, NULL);
	}

	scd_show_pcm_boot(3, bios_address, program_size, 0, 0, 0, NULL);
	if (!HwMdSegaCDCommand(SCD_OP_PROGRAM_RAM_TEST,
		SCD_TEST_BEGIN | SCD_PROGRAM_PATTERN | 3))
	{
		scd_show_pcm_boot(3, bios_address, program_size, 0, 0, 0,
			"PROGRAM RAM failed. A to continue");
		pressed = scd_wait_boot_button();
		if (!(pressed & SEGA_CTRL_A))
			return 0;
		scd_screen_init();
		scd_show_pcm_boot(3, bios_address, program_size, 0, 0, 0, NULL);
	}

	scd_show_pcm_boot(4, bios_address, program_size, 0, 0, 0, NULL);
	scd_show_pcm_boot(5, bios_address, program_size, 0, 0, 0, NULL);
	scd_show_pcm_boot(6, bios_address, program_size, 0, 0, 0, NULL);
	scd_show_pcm_boot(7, bios_address, program_size, 0, 0, 0, NULL);
	scd_show_pcm_boot(8, bios_address, program_size, 0, 0, 0, NULL);
	initialized = HwMdSegaCDCommand(SCD_OP_INIT, 0) != 0;
	HwMdSegaCDCommand(SCD_OP_INIT_INFO, SCD_INIT_INFO_SP_INIT_WAIT);
	sp_init_wait = HwMdSegaCDResult0();
	HwMdSegaCDCommand(SCD_OP_INIT_INFO, SCD_INIT_INFO_SP_MAIN_WAIT);
	sp_main_wait = HwMdSegaCDResult0();
	if (!initialized)
	{
		u32 init_failure;
		HwMdSegaCDCommand(SCD_OP_INIT_INFO, SCD_INIT_INFO_FAILURE);
		init_failure = HwMdSegaCDResult0();
		if (init_failure == 3)
			failure = "SCD SP_Init failed";
		else if (init_failure == 4)
			failure = "SCD SP_Main failed";
		else if (init_failure == 5)
			failure = "Failed Program RAM";
		else
			failure = "Sega CD initialization failed";
		scd_show_pcm_boot(8, bios_address, program_size, sp_init_wait,
			sp_main_wait, 0, failure);
		scd_wait_boot_button();
		return 0;
	}

	scd_show_pcm_boot(9, bios_address, program_size, sp_init_wait,
		sp_main_wait, 0, NULL);
	scd_show_pcm_boot(10, bios_address, program_size, sp_init_wait,
		sp_main_wait, 0, NULL);
	scd_show_pcm_boot(11, bios_address, program_size, sp_init_wait,
		sp_main_wait, 0, NULL);
	scd_show_pcm_boot(12, bios_address, program_size, sp_init_wait,
		sp_main_wait, 0, NULL);
	scd_show_pcm_boot(13, bios_address, program_size, sp_init_wait,
		sp_main_wait, 0, NULL);
	communication_ok = HwMdSegaCDCommand(SCD_OP_SUBCPU_PING, 0) != 0;
	ping_result = communication_ok ? HwMdSegaCDResult0() : 0;
	if (!communication_ok || (ping_result & 0xFFFF) != 0xE715)
	{
		communication_ok = HwMdSegaCDCommand(SCD_OP_SUBCPU_PING, 0) != 0;
		ping_result = communication_ok ? HwMdSegaCDResult0() : 0;
		communication_ok = communication_ok &&
			(ping_result & 0xFFFF) == 0xE715;
	}
	scd_show_pcm_boot(14, bios_address, program_size, sp_init_wait,
		sp_main_wait, communication_ok, NULL);
	scd_show_pcm_boot(15, bios_address, program_size, sp_init_wait,
		sp_main_wait, communication_ok, NULL);
	scd_wait_boot_button();

	if (load_pcm && !HwMdSegaCDCommand(SCD_OP_PCM_LOAD, 0))
	{
		int i;
		scd_screen_init();
		for (i = 0; i < 2; i++)
		{
			Hw32xFlipWait();
			drawMainBG();
			scd_draw_genesis_hex("PCM data load failed", 0x2000, 4,
				40, 80, fontColorGreen);
			drawTextwHighlight("Press A to continue", 80, 112,
				fontColorWhite, fontColorWhiteHighlight);
			Hw32xScreenFlip(0);
		}
		pressed = scd_wait_boot_button();
		if (!(pressed & SEGA_CTRL_A))
		{
			HwMdSegaCDCommand(SCD_OP_RESET, 0);
			return 0;
		}
	}

	return 1;
}

static int scd_initialize_pcm_sound(void)
{
	return scd_initialize_test(1);
}

static void scd_sound_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0, option = 1, track = 0, tracks = 0;
	int track_type = 0xFF, enable_cd = 0, pcm_timer = 0;
	int warning = 0, model, timer = 0;
	int tray_closed = 0, last_tray_closed = -1;
	int dynamic_state[2] = { -1, -1 };
	u16 cd_status = 0, drive_version = 0;
	int read_drive_version = 0;
	const char *warning_text = NULL;

	if (!scd_initialize_pcm_sound())
	{
		screenFadeOut(1);
		return;
	}

	scd_screen_init();
	model = scd_bios_byte(0x18A) == '1' ? 1 : 2;
	while (!done)
	{
		char track_text[8] = "  NONE ";
		u32 dynamic_key;
		int draw_page;
		int hash_index;
		int max_option = selection == 0 ? 2 :
			(selection == 1 ? 3 : 0);

		if (tracks && track)
		{
			strcpy(track_text, track_type == 0 ? "CDDA   " : "DATA   ");
			uintToStr(track, track_text + 5, 2);
		}
		Hw32xFlipWait();
		drawMainBG();
		draw_page = scd_draw_page();
		dynamic_key = (u32)track | ((u32)tracks << 8) |
			((u32)(track_type & 0xFF) << 16) |
			((u32)enable_cd << 24) | ((u32)(cd_status & 0x0F) << 25) |
			((u32)(warning && warning_text) << 29) |
			((u32)tray_closed << 30) |
			((u32)read_drive_version << 31);
		dynamic_key ^= (u32)drive_version * 257u;
		if (warning && warning_text)
			for (hash_index = 0; warning_text[hash_index]; hash_index++)
				dynamic_key = dynamic_key * 33u ^
					(u32)(u8)warning_text[hash_index];
		if (dynamic_state[draw_page] != (int)dynamic_key)
		{
			scd_restore_region(64, 128, 240, 72);
			dynamic_state[draw_page] = (int)dynamic_key;
		}
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
					fontColorRed, fontColorRedHighlight);
			else if (cd_status == 0x0C)
				drawTextwHighlight("No CD in tray", 112, 160,
					fontColorRed, fontColorRedHighlight);
			else if (cd_status == 0x0A)
				drawTextwHighlight("TOC reported 0 tracks", 80, 160,
					fontColorRed, fontColorRedHighlight);
		}
		if (warning && warning_text)
			drawTextwHighlight((char *)warning_text, 72, 152,
				fontColorWhite, fontColorWhiteHighlight);
		if (read_drive_version)
			scd_draw_genesis_hex("Drive Version", drive_version, 2,
				144, 192, fontColorRed);
		Hw32xScreenFlip(0);

		if (enable_cd && ++timer > 60)
		{
			u32 disc;
			int new_tray_closed = 0;

			if (HwMdSegaCDCommand(SCD_OP_DISC_INFO, 0))
			{
				disc = HwMdSegaCDResult0();
				cd_status = disc >> 16;
				new_tray_closed = cd_status != 0x0A &&
					cd_status != 0x0B && cd_status != 0x0C;
				if (new_tray_closed)
				{
					int new_tracks = disc & 0xFF;
					drive_version = HwMdSegaCDResult2();
					read_drive_version = 1;
					if (last_tray_closed != new_tray_closed)
					{
						if (pcm_timer)
						{
							HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
							pcm_timer = 0;
						}
						warning_text = "Please wait while reading CD TOC";
						warning = 2;
						tracks = new_tracks;
						track = tracks ? 1 : 0;
						track_type = 0xFF;
						while (track && track <= tracks)
						{
							track_type = HwMdSegaCDCommand(
								SCD_OP_TRACK_INFO, track) ?
								(HwMdSegaCDResult2() & 0xFF) : 0xFF;
							if (track_type == 0 || track == tracks)
								break;
							track++;
						}
					}
					else
						tracks = new_tracks;
				}
			}

			if (!new_tray_closed)
			{
				tracks = 0;
				track = 0;
				track_type = 0xFF;
			}
			tray_closed = new_tray_closed;
			last_tray_closed = new_tray_closed;
			timer = 0;
		}

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
		if (pressed & SEGA_CTRL_A)
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
					if (cd_status == 0x0B)
					{
						warning_text = "Tray already opened";
						warning = 120;
					}
					else if (model == 2)
					{
						warning_text = "Model 2 open manually";
						warning = 120;
					}
					else
					{
						HwMdSegaCDCommand(SCD_OP_TRAY_OPEN, 0);
						timer = 61;
					}
				}
				else if (tray_closed)
				{
					warning_text = "Tray already closed";
					warning = 120;
				}
				else if (model == 2)
				{
					warning_text = "Model 2 close manually";
					warning = 120;
				}
				else
				{
					HwMdSegaCDCommand(SCD_OP_TRAY_CLOSE, 0);
					timer = 61;
				}
			}
		}
		if (pressed & SEGA_CTRL_C)
		{
			enable_cd ^= 1;
			if (!enable_cd)
				timer = 0;
		}
		if (pressed & SEGA_CTRL_START)
			done = 1;
	}
	HwMdSegaCDCommand(SCD_OP_PCM_CENTER, 0);
	if (pcm_timer)
		HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
	if (tracks)
		HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	HwMdSegaCDCommand(SCD_OP_RESET, 0);
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

enum SegaCDDiscIDState {
	SCD_DISC_ID_INITIAL = 0,
	SCD_DISC_ID_READING,
	SCD_DISC_ID_SUCCESS,
	SCD_DISC_ID_NO_DISC,
	SCD_DISC_ID_ERROR
};

typedef struct SegaCDDiscID {
	char domestic[SCD_DISC_ID_TITLE_SIZE];
	char overseas[SCD_DISC_ID_TITLE_SIZE];
	char product[SCD_DISC_ID_PRODUCT_SIZE];
	char region[SCD_DISC_ID_REGION_SIZE];
	u16 tracks;
	u16 first_data_track;
	u32 boot_crc;
} SegaCDDiscID;

static int scd_centered_x(const char *text)
{
	int width = strlen(text) * 8;
	return width < 320 ? (320 - width) / 2 : 0;
}

static void scd_disc_id_fetch_field(u16 field, char *destination, int size)
{
	int offset;

	for (offset = 0; offset + 1 < size; offset += 2)
	{
		u16 word = HwMdSegaCDCommand(SCD_OP_DISC_ID_FIELD,
			(field << 8) | (offset >> 1));
		destination[offset] = word >> 8;
		if (!destination[offset])
			break;
		if (offset + 1 < size - 1)
		{
			destination[offset + 1] = word & 0xFF;
			if (!destination[offset + 1])
				break;
		}
	}
	destination[size - 1] = 0;
}

static int scd_read_disc_id(SegaCDDiscID *disc, const char **diagnostic)
{
	u32 info;
	u16 status;
	u16 poll = 0;
	int first, last, track, wait;

	myMemSet(disc, 0, sizeof(*disc));
	*diagnostic = NULL;
	if (!HwMdSegaCDCommand(SCD_OP_INIT, 0))
	{
		*diagnostic = "Sega CD initialization failed";
		return SCD_DISC_ID_ERROR;
	}
	if (!HwMdSegaCDCommand(SCD_OP_DISC_INFO, 0))
	{
		*diagnostic = "Could not read disc status";
		return SCD_DISC_ID_ERROR;
	}
	info = HwMdSegaCDResult0();
	status = info >> 16;
	if (status == 0x0B || status == 0x0C)
		return SCD_DISC_ID_NO_DISC;
	if (status == 0x0A)
	{
		*diagnostic = "Invalid or unreadable disc TOC";
		return SCD_DISC_ID_ERROR;
	}
	if (status == 0x0D)
	{
		*diagnostic = "CD drive did not become ready";
		return SCD_DISC_ID_ERROR;
	}
	if (status != 1)
	{
		*diagnostic = "CD drive is not ready";
		return SCD_DISC_ID_ERROR;
	}

	first = (info >> 8) & 0xFF;
	last = info & 0xFF;
	if (!first || first == 0xFF || last < first)
	{
		*diagnostic = "Invalid or unreadable disc TOC";
		return SCD_DISC_ID_ERROR;
	}
	disc->tracks = last - first + 1;
	for (track = first; track <= last; track++)
	{
		if (!HwMdSegaCDCommand(SCD_OP_TRACK_INFO, track))
		{
			*diagnostic = "Could not read disc track data";
			return SCD_DISC_ID_ERROR;
		}
		if (!disc->first_data_track &&
			(HwMdSegaCDResult2() & 0xFF) == 0xFF)
			disc->first_data_track = track;
	}

	HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, 0);
	if (!HwMdSegaCDCommand(SCD_OP_DATA_START, 1))
	{
		*diagnostic = "Could not start boot sector read";
		return SCD_DISC_ID_ERROR;
	}
	for (wait = 0; wait < 600; wait++)
	{
		scd_wait_vblank();
		poll = HwMdSegaCDCommand(SCD_OP_DATA_POLL, 0);
		if (poll)
			break;
	}
	if (poll != 1)
	{
		*diagnostic = poll ? "Disc boot sector unreadable" :
			"Boot sector read timed out";
		if (!poll)
			HwMdSegaCDCommand(SCD_OP_RESET, 0);
		return SCD_DISC_ID_ERROR;
	}

	status = HwMdSegaCDCommand(SCD_OP_DISC_ID_PARSE, 0);
	if (status == 2)
	{
		*diagnostic = "Invalid Sega CD boot sector";
		return SCD_DISC_ID_ERROR;
	}
	if (status != 1)
	{
		*diagnostic = "Could not parse boot sector";
		return SCD_DISC_ID_ERROR;
	}
	disc->boot_crc = HwMdSegaCDResult0();
	scd_disc_id_fetch_field(SCD_DISC_ID_DOMESTIC, disc->domestic,
		SCD_DISC_ID_TITLE_SIZE);
	scd_disc_id_fetch_field(SCD_DISC_ID_OVERSEAS, disc->overseas,
		SCD_DISC_ID_TITLE_SIZE);
	scd_disc_id_fetch_field(SCD_DISC_ID_PRODUCT, disc->product,
		SCD_DISC_ID_PRODUCT_SIZE);
	scd_disc_id_fetch_field(SCD_DISC_ID_REGION, disc->region,
		SCD_DISC_ID_REGION_SIZE);
	return SCD_DISC_ID_SUCCESS;
}

static void scd_draw_disc_id_value(const char *label, const char *value,
	int y, int wrap)
{
	char line[25];
	int i = 0;
	int length = strlen(value);

	drawTextwHighlight((char *)label, 40, y,
		fontColorGreen, fontColorGreenHighlight);
	while (i < 24 && i < length)
	{
		line[i] = value[i];
		i++;
	}
	line[i] = 0;
	drawTextwHighlight(line, 136, y,
		fontColorWhite, fontColorWhiteHighlight);
	if (wrap && length > 24)
	{
		int source = 24;
		i = 0;
		while (i < 24 && source < length)
			line[i++] = value[source++];
		line[i] = 0;
		drawTextwHighlight(line, 136, y + 8,
			fontColorWhite, fontColorWhiteHighlight);
	}
}

static void scd_draw_disc_id_frame(int state, const SegaCDDiscID *disc,
	const char *diagnostic, int page_state[2])
{
	static const char *title = "SEGA CD DISC IDENTIFICATION";
	static const char *retry = "Press A to read again, B to exit";
	char tracks[4];
	char data[9];
	char crc[9];
	const char *game;
	const char *domestic;
	int page = scd_draw_page();

	drawMainBG();
	if (page_state[page] != state)
	{
		scd_restore_region(8, 24, 304, 176);
		page_state[page] = state;
	}
	drawTextwHighlight((char *)title, scd_centered_x(title), 32,
		fontColorGreen, fontColorGreenHighlight);
	if (state == SCD_DISC_ID_INITIAL)
	{
		static const char *prompt = "Press A read disc, B to exit";
		drawTextwHighlight((char *)prompt, scd_centered_x(prompt), 104,
			fontColorGreen, fontColorGreenHighlight);
	}
	else if (state == SCD_DISC_ID_READING)
	{
		static const char *reading = "Reading disc...";
		drawTextwHighlight((char *)reading, scd_centered_x(reading), 104,
			fontColorGreen, fontColorGreenHighlight);
	}
	else if (state == SCD_DISC_ID_SUCCESS)
	{
		game = disc->overseas[0] ? disc->overseas :
			(disc->domestic[0] ? disc->domestic : "Unknown");
		domestic = disc->domestic[0] ? disc->domestic : "Unknown";
		uintToStr(disc->tracks, tracks, 1);
		if (disc->first_data_track)
		{
			strcpy(data, "Track 00");
			data[6] = '0' + (disc->first_data_track / 10) % 10;
			data[7] = '0' + disc->first_data_track % 10;
		}
		else
			strcpy(data, "None");
		intToHex(disc->boot_crc, crc, 8);
		scd_draw_disc_id_value("Game:", game, 52, 1);
		scd_draw_disc_id_value("Domestic:", domestic, 72, 1);
		scd_draw_disc_id_value("Product:", disc->product, 96, 0);
		scd_draw_disc_id_value("Region:", disc->region, 112, 0);
		scd_draw_disc_id_value("Tracks:", tracks, 128, 0);
		scd_draw_disc_id_value("Data:", data, 144, 0);
		scd_draw_disc_id_value("Boot CRC32:", crc, 160, 0);
		drawTextwHighlight((char *)retry, scd_centered_x(retry), 192,
			fontColorGreen, fontColorGreenHighlight);
	}
	else
	{
		const char *message = state == SCD_DISC_ID_NO_DISC ?
			"No disc detected" : diagnostic;
		if (!message)
			message = "Disc read failed";
		drawTextwHighlight((char *)message, scd_centered_x(message), 88,
			fontColorRed, fontColorRedHighlight);
		drawTextwHighlight((char *)retry, scd_centered_x(retry), 112,
			fontColorGreen, fontColorGreenHighlight);
	}
}

static void scd_disc_id_test(void)
{
	u16 button, pressed, old;
	int done = 0;
	int pending = 0;
	int state = SCD_DISC_ID_INITIAL;
	int page_state[2] = { -1, -1 };
	const char *diagnostic = NULL;
	SegaCDDiscID disc;

	myMemSet(&disc, 0, sizeof(disc));
	if (!scd_initialize_test(0))
		return;
	scd_screen_init();
	old = scd_pad();
	while (!done)
	{
		Hw32xFlipWait();
		scd_draw_disc_id_frame(state, &disc, diagnostic, page_state);
		Hw32xScreenFlip(0);

		if (pending)
		{
			pending = 0;
			state = scd_read_disc_id(&disc, &diagnostic);
			old = scd_pad();
			continue;
		}
		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_A)
		{
			myMemSet(&disc, 0, sizeof(disc));
			diagnostic = NULL;
			state = SCD_DISC_ID_READING;
			pending = 1;
		}
		if (pressed & SEGA_CTRL_B)
			done = 1;
	}
	HwMdSegaCDCommand(SCD_OP_RESET, 0);
	screenFadeOut(1);
}

static void scd_integrity_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, passed = 0, failed = 0;
	int dynamic_state[2] = { -1, -1 };
	u16 poll = 0;
	u32 sub_crc = 0, main_crc = 0, sh2_crc = 0;
	u32 lba = SCD_DEFAULT_DATA_LBA;

	scd_screen_init();
	while (!done)
	{
		u32 dynamic_key;
		int page;

		Hw32xFlipWait();
		scd_draw_title("CD Transfer Check");
		page = scd_draw_page();
		dynamic_key = lba ^ sub_crc ^ (main_crc << 1) ^ (sh2_crc >> 1) ^
			((u32)running << 24) ^ ((u32)complete << 25) ^
			((u32)passed << 26) ^ ((u32)failed << 27);
		if (dynamic_state[page] != (int)dynamic_key)
		{
			scd_restore_region(24, 56, 280, 96);
			dynamic_state[page] = (int)dynamic_key;
		}
		scd_draw_number_field("Logical sector:", lba, 40, 176, 56,
			fontColorRed);
		if (running)
			scd_draw_text_field("Status:", "Reading", 40, 112, 80,
				fontColorWhite);
		else if (!complete)
			scd_draw_text_field("Status:", "Ready", 40, 112, 80,
				fontColorWhite);
		else if (failed)
			scd_draw_text_field("Status:", "Read failed", 40, 112, 80,
				fontColorRed);
		else
		{
			scd_draw_text_field("Result:", passed ? "PASS" : "FAIL",
				40, 112, 80, passed ? fontColorWhite : fontColorRed);
			scd_draw_genesis_hex("Sega CD CRC32:", sub_crc, 8, 40, 104,
				fontColorGreen);
			scd_draw_genesis_hex("68000 CRC32:", main_crc, 8, 40, 120,
				fontColorGreen);
			scd_draw_genesis_hex("SH2 CRC32:", sh2_crc, 8, 40, 136,
				fontColorGreen);
		}
		drawTextwHighlight("Left/Right selects LBA", 72, 160,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("'A' to test 'B' to exit", 64, 176,
			fontColorGreen, fontColorGreenHighlight);
		Hw32xScreenFlip(0);

		if (running)
		{
			poll = HwMdSegaCDCommand(SCD_OP_DATA_POLL, 1);
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
				{
					passed = 0;
					failed = 1;
				}
				old = scd_pad();
			}
		}

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (!running && (pressed & SEGA_CTRL_LEFT) && lba) { lba--; complete = 0; }
		if (!running && (pressed & SEGA_CTRL_RIGHT) && lba < 0xFFFF)
			{ lba++; complete = 0; }
		if (!running && (pressed & SEGA_CTRL_A))
		{
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, lba);
				running = HwMdSegaCDCommand(SCD_OP_DATA_START, 1);
				complete = !running;
				failed = !running;
			}
			else
			{
				complete = 1;
				failed = 1;
			}
		}
		if (!running && (pressed & SEGA_CTRL_B)) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stream_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, failed = 0;
	int dynamic_state[2] = { -1, -1 };
	u32 last_tick = 0, frames = 0, missed = 0;
	u32 rate = 0;
	u16 poll = 0;

	scd_screen_init();
	while (!done)
	{
		u32 tick = Hw32xGetTicks();
		u32 dynamic_key;
		int page;

		Hw32xFlipWait();
		scd_draw_title("CD Streaming Test");
		page = scd_draw_page();
		dynamic_key = frames ^ (missed * 33u) ^ (rate * 257u) ^
			((u32)running << 24) ^ ((u32)complete << 25) ^
			((u32)failed << 26);
		if (dynamic_state[page] != (int)dynamic_key)
		{
			scd_restore_region(24, 56, 280, 96);
			dynamic_state[page] = (int)dynamic_key;
		}
		if (running)
			scd_draw_text_field("Status:", "Streaming", 40, 120, 64,
				fontColorWhite);
		else if (!complete)
			scd_draw_text_field("Status:", "Ready", 40, 120, 64,
				fontColorWhite);
		else if (failed)
			scd_draw_text_field("Status:", "Read failed", 40, 120, 64,
				fontColorRed);
		else
		{
			scd_draw_text_field("Status:", "Complete", 40, 120, 64,
				fontColorWhite);
			scd_draw_number_field("Frames:", frames, 40, 192, 88,
				fontColorWhite);
			scd_draw_number_field("Missed frames:", missed, 40, 192, 104,
				missed ? fontColorRed : fontColorWhite);
			scd_draw_number_field("Sectors/second:", rate, 40, 192, 120,
				fontColorWhite);
		}
		drawTextwHighlight("'A' to test 'B' to exit", 64, 176,
			fontColorGreen, fontColorGreenHighlight);
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
			if (!running)
			{
				complete = 1;
				failed = 1;
			}
		}
		if (!running && (pressed & SEGA_CTRL_B)) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stop_audio_source(int source)
{
	if (source == 0) HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, 0);
	if (source == 1) HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	if (source == 2) sound_test_pwm_stop();
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
	int dynamic_state[2] = { -1, -1 };
	const char *error = NULL;

	MDPSG_init();
	scd_screen_init();

	while (!done)
	{
		u32 dynamic_key;
		int page;

		Hw32xFlipWait();
		scd_draw_title("Audio Mixer Test");
		page = scd_draw_page();
		dynamic_key = ((u32)source | ((u32)pan << 4) |
			((u32)playing << 8) | ((u32)track << 16)) ^
			(u32)(uintptr_t)error;
		if (dynamic_state[page] != (int)dynamic_key)
		{
			scd_restore_region(24, 56, 280, 106);
			dynamic_state[page] = (int)dynamic_key;
		}
		scd_draw_text_field("Source:", sources[source], 40, 112, 64,
			fontColorRed);
		if (source == 1 || source == 3)
			scd_draw_text_field("Output:", "Fixed", 40, 112, 88,
				fontColorRed);
		else
			scd_draw_text_field("Output:", pans[pan], 40, 112, 88,
				fontColorRed);
		if (source == 1)
			scd_draw_number_field("Track:", track, 40, 112, 112,
				fontColorRed);
		scd_draw_text_field("Status:", playing ? "Playing" : "Stopped",
			40, 112, 136, fontColorWhite);
		if (error)
			drawTextwHighlight((char *)error, 40, 152,
				fontColorRed, fontColorRedHighlight);
		drawTextwHighlight("D-Pad selects source and setting", 32, 168,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("'A' plays/stops  'START' exits", 32, 184,
			fontColorGreen, fontColorGreenHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_UP)
		{
			if (playing) scd_stop_audio_source(source);
			playing = 0;
			source = source ? source - 1 : 4;
			error = NULL;
		}
		if (pressed & SEGA_CTRL_DOWN)
		{
			if (playing) scd_stop_audio_source(source);
			playing = 0;
			source = source == 4 ? 0 : source + 1;
			error = NULL;
		}
		if (source == 1)
		{
			if ((pressed & SEGA_CTRL_LEFT) && track > 1)
			{
				track--;
				error = NULL;
			}
			if ((pressed & SEGA_CTRL_RIGHT) && track < 99)
			{
				track++;
				error = NULL;
			}
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
				scd_stop_audio_source(source);
				playing = 0;
				error = NULL;
			}
			else
			{
				error = NULL;
				if (source == 0)
				{
					if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
					{
						playing = HwMdSegaCDCommand(SCD_OP_PCM_CONTROL, pan + 1);
						if (!playing)
							error = "Sega CD PCM playback failed";
					}
					else
						error = "Sega CD initialization failed";
				}
				if (source == 1)
				{
					error = scd_find_cdda_track(track, &track);
					if (!error)
					{
						playing = HwMdSegaCDCommand(SCD_OP_CDDA_PLAY, track);
						if (!playing)
							error = "CD-DA playback failed";
					}
				}
				if (source == 2)
				{
					if (!sound_isInitialized())
						Mars_InitSoundDMA();
					playing = sound_test_pwm_start(1000,
						pan == 0 ? 1 : (pan == 2 ? 2 : 3));
					if (!playing)
						error = "32X PWM playback failed";
				}
				if (source == 3)
				{
					HwMdPSGSetChandVol(0, 0);
					HwMdPSGSetFrequency(0, 1000);
					playing = 1;
				}
				if (source == 4)
				{
					playing = HwMdSegaCDCommand(SCD_OP_YM_CONTROL,
						1 | (pan << 8));
					if (!playing)
						error = "YM2612 playback failed";
				}
			}
		}
		if (pressed & SEGA_CTRL_START) done = 1;
	}
	if (playing) scd_stop_audio_source(source);
	MDPSG_stop();
	screenFadeOut(1);
}

static void scd_av_sync(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, frame = 0, offset = 0, track = 3;
	int audio_started = 0, initialized = 0;
	int dynamic_state[2] = { -1, -1 };
	int restore_page[2] = { 0, 0 };
	const char *error = NULL;

	scd_screen_init();
	while (!done)
	{
		int flash = running && ((offset >= 0 && frame == offset) ||
			(offset < 0 && frame == 0));
		int page;
		u32 dynamic_key;

		Hw32xFlipWait();
		page = scd_draw_page();
		if (flash)
		{
			volatile u8 *fb = (volatile u8 *)((u16 *)&MARS_FRAMEBUFFER + 0x100);
			int i;
			for (i = 0; i < 320 * 224; i++) fb[i] = fontColorWhite;
			restore_page[page] = 1;
		}
		else
		{
			if (restore_page[page])
			{
				scd_restore_region(0, 0, 320, 224);
				restore_page[page] = 0;
				dynamic_state[page] = -1;
			}
			scd_draw_title("CD-DA / 32X Sync Test");
			dynamic_key = ((u32)track | ((u32)(offset + 30) << 8) |
				((u32)running << 16) | ((u32)frame << 17)) ^
				(u32)(uintptr_t)error;
			if (dynamic_state[page] != (int)dynamic_key)
			{
				scd_restore_region(24, 56, 280, 96);
				dynamic_state[page] = (int)dynamic_key;
			}
			scd_draw_number_field("Track:", track, 40, 224, 64,
				fontColorRed);
			scd_draw_signed_field("Video offset:", offset, 40, 224, 80,
				fontColorRed);
			scd_draw_text_field("Status:", error ? "Failed" :
				(running ? "Running" : (frame > 300 ? "Complete" : "Ready")),
				40, 224, 104, error ? fontColorRed : fontColorWhite);
			if (running)
				scd_draw_number_field("Frame:", frame, 40, 224, 120,
					fontColorWhite);
			if (error)
				drawTextwHighlight((char *)error, 40, 136,
					fontColorRed, fontColorRedHighlight);
			drawTextwHighlight("D-Pad selects track and offset", 40, 168,
				fontColorGreen, fontColorGreenHighlight);
			drawTextwHighlight("'A' starts  'START' exits", 56, 184,
				fontColorGreen, fontColorGreenHighlight);
		}
		Hw32xScreenFlip(0);

		if (running)
		{
			if (!audio_started && ((offset >= 0 && frame == 0) ||
				(offset < 0 && frame == -offset)))
			{
				if (HwMdSegaCDCommand(SCD_OP_CDDA_PLAY, track))
					audio_started = 1;
				else
				{
					error = "CD-DA playback failed";
					running = 0;
				}
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
		if (!running && (pressed & SEGA_CTRL_LEFT) && offset > -30)
			{ offset--; frame = 0; error = NULL; }
		if (!running && (pressed & SEGA_CTRL_RIGHT) && offset < 30)
			{ offset++; frame = 0; error = NULL; }
		if (!running && (pressed & SEGA_CTRL_UP) && track < 99)
			{ track++; frame = 0; error = NULL; }
		if (!running && (pressed & SEGA_CTRL_DOWN) && track > 1)
			{ track--; frame = 0; error = NULL; }
		if (!running && (pressed & SEGA_CTRL_A))
		{
			error = scd_find_cdda_track(track, &track);
			if (!error)
			{
				initialized = 1;
				frame = 0;
				audio_started = 0;
				running = 1;
			}
		}
		if (pressed & SEGA_CTRL_START) done = 1;
	}
	if (initialized)
		HwMdSegaCDCommand(SCD_OP_CDDA_STOP, 0);
	screenFadeOut(1);
}

static void scd_wordram_stress(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, running = 0, complete = 0, passed = 0;
	int dynamic_state[2] = { -1, -1 };
	u16 pass = 0;

	scd_screen_init();
	while (!done)
	{
		u32 dynamic_key;
		int page;

		Hw32xFlipWait();
		scd_draw_title("Word RAM Stress Test");
		page = scd_draw_page();
		dynamic_key = (u32)pass | ((u32)running << 16) |
			((u32)complete << 17) | ((u32)passed << 18);
		if (dynamic_state[page] != (int)dynamic_key)
		{
			scd_restore_region(24, 56, 280, 88);
			dynamic_state[page] = (int)dynamic_key;
		}
		scd_draw_text_field("Status:", running ? "Testing" :
			(complete ? (passed ? "Complete" : "Failed") : "Ready"),
			40, 192, 64, complete && !passed ? fontColorRed : fontColorWhite);
		scd_draw_number_field("Completed passes:", pass, 40, 192, 88,
			fontColorWhite);
		if (complete)
			scd_draw_text_field("Result:", passed ? "PASS" : "FAIL",
				40, 192, 112, passed ? fontColorWhite : fontColorRed);
		drawTextwHighlight("'A' to test 'B' to exit", 64, 176,
			fontColorGreen, fontColorGreenHighlight);
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
		if (!running && (pressed & SEGA_CTRL_A))
		{
			pass = 0;
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				complete = 0;
				passed = 1;
				running = 1;
			}
			else
			{
				complete = 1;
				passed = 0;
			}
		}
		if (!running && (pressed & SEGA_CTRL_B)) done = 1;
	}
	screenFadeOut(1);
}

static void scd_latency_test(void)
{
	u16 button, pressed, old = 0xFFFF;
	int done = 0, complete = 0, failed = 0;
	int dynamic_state[2] = { -1, -1 };
	u32 minimum = 0, maximum = 0, average = 0;

	scd_screen_init();
	while (!done)
	{
		u32 dynamic_key;
		int page;

		Hw32xFlipWait();
		scd_draw_title("CPU Communication Test");
		page = scd_draw_page();
		dynamic_key = minimum ^ (maximum << 1) ^ (average << 2) ^
			((u32)complete << 28) ^ ((u32)failed << 29);
		if (dynamic_state[page] != (int)dynamic_key)
		{
			scd_restore_region(24, 56, 280, 96);
			dynamic_state[page] = (int)dynamic_key;
		}
		scd_draw_text_field("Status:", !complete ? "Ready" :
			(failed ? "Failed" : "Complete"), 40, 192, 64,
			failed ? fontColorRed : fontColorWhite);
		if (failed)
			drawTextwHighlight("Communication failed", 40, 88,
				fontColorRed, fontColorRedHighlight);
		else if (complete)
		{
			scd_draw_number_field("Minimum ticks:", minimum, 40, 192, 88,
				fontColorWhite);
			scd_draw_number_field("Maximum ticks:", maximum, 40, 192, 104,
				fontColorWhite);
			scd_draw_number_field("Average ticks:", average, 40, 192, 120,
				fontColorWhite);
		}
		drawTextwHighlight("'A' to test 'B' to exit", 64, 176,
			fontColorGreen, fontColorGreenHighlight);
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (pressed & SEGA_CTRL_A)
		{
			failed = 0;
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				u32 total = 0;
				int i;
				minimum = 0xFFFFFFFFUL;
				maximum = 0;
				for (i = 0; i < 64; i++)
				{
					u16 start = (u16)Mars_GetFRTCounter();
					u16 elapsed;
					u16 response = HwMdSegaCDCommand(SCD_OP_SUBCPU_PING, i);
					u16 end = (u16)Mars_GetFRTCounter();
					u32 reply = response ? HwMdSegaCDResult0() : 0;
					elapsed = (u16)(end - start);
					if (!response || (reply & 0xFFFF) != 0xE715 ||
						(reply >> 16) != (u16)i)
					{
						failed = 1;
						break;
					}
					if (elapsed < minimum) minimum = elapsed;
					if (elapsed > maximum) maximum = elapsed;
					total += elapsed;
				}
				if (!failed)
					average = total / 64;
				complete = 1;
				old = scd_pad();
			}
			else
			{
				failed = 1;
				complete = 1;
			}
		}
		if (pressed & SEGA_CTRL_B) done = 1;
	}
	screenFadeOut(1);
}

static void scd_stream_overlay(void)
{
	perf_set_scene(PERF_SCENE_SCD_OVERLAY);
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
	loadTextPalette();
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_68K | MARS_224_LINES | MARS_VDP_MODE_256;
	Hw32xScreenFlip(0);
	init_tilemap(&tm, &mdtest_map_Map,
		(const uint8_t * const *)mdtest_Reslist);
	Hw32xSetBGOverlayPriorityBit(0);
	Hw32xSetFGOverlayPriorityBit(1);
	Hw32xSetPalettePriorityAliases(17, 1, 10, 0);

	while (!done)
	{
		Hw32xFlipWait();
		camera += direction;
		if (camera <= 0 || camera >= 128) direction = -direction;
		draw_tilemap(&tm, camera << 16, 0, 0, NULL, NULL);
		drawTextwHighlight("CD Overlay Test", 100, 32,
			fontColorGreen, fontColorGreenHighlight);
		scd_draw_text_field("Status:", running ? "Streaming" :
			(complete ? (failed ? "Read failed" : "Complete") : "Ready"),
			72, 160, 48, failed ? fontColorRed : fontColorWhite);
		scd_draw_text_field("Priority:", priority ? "32X" : "Genesis",
			72, 160, 160, fontColorRed);
		drawTextwHighlight("'A' starts streaming", 80, 176,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("'B' changes priority  'START' exits", 16, 188,
			fontColorGreen, fontColorGreenHighlight);
		draw_dirtyrect(&tm, 40, 32, 240, 10);
		draw_dirtyrect(&tm, 72, 48, 200, 10);
		draw_dirtyrect(&tm, 72, 160, 176, 10);
		draw_dirtyrect(&tm, 80, 176, 160, 10);
		draw_dirtyrect(&tm, 16, 188, 288, 10);
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
		if (!running && (pressed & SEGA_CTRL_A))
		{
			if (HwMdSegaCDCommand(SCD_OP_INIT, 0))
			{
				HwMdSegaCDCommand(SCD_OP_DATA_SET_LBA, SCD_DEFAULT_DATA_LBA);
				running = HwMdSegaCDCommand(SCD_OP_DATA_START,
					SCD_STREAM_SECTORS);
			}
			complete = !running;
			failed = !running;
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

void menu_segacd32x(void)
{
	static const char *items[] = {
		"Disc ID", "CD Transfer Check", "CD Streaming Test",
		"Audio Mixer Test", "CD-DA / 32X Sync Test", "Word RAM Stress Test",
		"CPU Communication Test", "CD Overlay Test", "Help", "Back to Main Menu"
	};
	static const int item_y[] = {56, 72, 80, 88, 96, 112, 120, 128, 152, 160};
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0;

	if (!(HwMdSegaCDCommand(SCD_OP_STATUS, 0) & SCD_STATUS_PRESENT) &&
		!scd_not_detected_menu())
	{
		screenFadeOut(1);
		return;
	}
	perf_set_scene(PERF_SCENE_MENU);
	initMainBGwGil();
	Hw32xScreenFlip(0);
	while (!done)
	{
		int i;
		u16 status;
		Hw32xFlipWait();
		drawBGwGil();
		status = HwMdSegaCDCommand(SCD_OP_STATUS, 0);
		drawTextwHighlight((status & SCD_STATUS_PRESENT) ?
			"Sega CD 32X Tests" : "Sega CD not detected",
			(status & SCD_STATUS_PRESENT) ? 92 : 80, 32,
			(status & SCD_STATUS_PRESENT) ? fontColorGreen : fontColorRed,
			(status & SCD_STATUS_PRESENT) ? fontColorGreenHighlight : fontColorRedHighlight);
		for (i = 0; i < 10; i++)
			drawTextwHighlight((char *)items[i], 40, item_y[i],
				selection == i ? fontColorRed : fontColorWhite,
				selection == i ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawResolution();
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (openOptionsShortcut(&button, &pressed))
		{
			perf_set_scene(PERF_SCENE_MENU);
			initMainBGwGil();
			Hw32xScreenFlip(0);
			old = scd_pad();
		}
		if (pressed & SEGA_CTRL_DOWN) selection = selection == 9 ? 0 : selection + 1;
		if (pressed & SEGA_CTRL_UP) selection = selection ? selection - 1 : 9;
		if (pressed & (SEGA_CTRL_B | SEGA_CTRL_START)) done = 1;
		if (scd_help_pressed(button, pressed))
		{
			DrawHelp(HELP_SEGACD32X);
			initMainBGwGil();
			Hw32xScreenFlip(0);
			old = scd_pad();
		}
		if (pressed & SEGA_CTRL_A)
		{
			screenFadeOut(1);
			switch (selection)
			{
				case 0: scd_disc_id_test(); break;
				case 1: scd_integrity_test(); break;
				case 2: scd_stream_test(); break;
				case 3: scd_combined_audio(); break;
				case 4: scd_av_sync(); break;
				case 5: scd_wordram_stress(); break;
				case 6: scd_latency_test(); break;
				case 7: scd_stream_overlay(); break;
				case 8: DrawHelp(HELP_SEGACD32X); break;
				case 9: done = 1; break;
			}
			if (!done)
			{
				perf_set_scene(PERF_SCENE_MENU);
				marsVDP256Start();
				initMainBGwGil();
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
		"BIOS CRC and info", "Disc ID", "Check HINT Register",
		"Check Flag Register", "Check Comm Registers", "Program RAM Check", "Word RAM Check",
		"Memory Viewer", "PCM RAM Check", "PCM Sound Check",
		"Help", "Back to Main Menu"
	};
	static const int item_y[] = {56, 64, 72, 80, 88, 104, 112, 120, 136, 144, 160, 168};
	u16 button, pressed, old = 0xFFFF;
	int done = 0, selection = 0;

	if (!(HwMdSegaCDCommand(SCD_OP_STATUS, 0) & SCD_STATUS_PRESENT) &&
		!scd_not_detected_menu())
	{
		screenFadeOut(1);
		return;
	}
	initMainBGwGil();
	Hw32xScreenFlip(0);
	while (!done)
	{
		int i;
		u16 status;
		Hw32xFlipWait();
		drawBGwGil();
		status = HwMdSegaCDCommand(SCD_OP_STATUS, 0);
		drawTextwHighlight((status & SCD_STATUS_PRESENT) ? "Sega CD Tests" :
			"Sega CD not detected", (status & SCD_STATUS_PRESENT) ? 112 : 88, 32,
			(status & SCD_STATUS_PRESENT) ? fontColorGreen : fontColorRed,
			(status & SCD_STATUS_PRESENT) ? fontColorGreenHighlight : fontColorRedHighlight);
		for (i = 0; i < 12; i++)
			drawTextwHighlight((char *)items[i], 40, item_y[i],
				selection == i ? fontColorRed : fontColorWhite,
				selection == i ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawResolution();
		Hw32xScreenFlip(0);

		button = scd_pad();
		pressed = button & ~old;
		old = button;
		if (openOptionsShortcut(&button, &pressed))
		{
			initMainBGwGil();
			Hw32xScreenFlip(0);
			old = scd_pad();
		}
		if (pressed & SEGA_CTRL_DOWN) selection = selection == 11 ? 0 : selection + 1;
		if (pressed & SEGA_CTRL_UP) selection = selection ? selection - 1 : 11;
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
				case 1: scd_disc_id_test(); break;
				case 2: scd_hint_test(); break;
				case 3: scd_flag_test(); break;
				case 4: scd_comm_test(); break;
				case 5: scd_program_ram_test(); break;
				case 6: scd_word_ram_test(); break;
				case 7: scd_memory_viewer(); break;
				case 8: scd_pcm_ram_test(); break;
				case 9: scd_sound_test(); break;
				case 10: DrawHelp(HELP_SEGACD); break;
				case 11: done = 1; break;
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
