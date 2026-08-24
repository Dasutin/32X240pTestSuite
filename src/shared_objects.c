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
#include "shared_objects.h"
#include "32x_images.h"
#include "draw.h"
#include "bg_tiles.h"
#include "bg_map.h"
#include "bg_palette.h"
#include "phase_check_gillian.h"
#include "gillian_blink.h"
#include "gillian_menu_palette.h"
#include "qrcode_tiles.h"
#include "perf.h"

u8 paused = PAUSED;
u8 segaCDDetectedAtBoot = 0;
u16 currentFB = 0;
vu16 overwriteImg16;
u32 _state = ~0L;
u16 randbase;

#define UNCACHED_CURRENT_FB \
	(*(volatile u16 *)((uintptr_t)&currentFB | 0x20000000u))

#define GILLIAN_WIDTH 56
#define GILLIAN_HEIGHT 104
#define GILLIAN_EYES_X 16
#define GILLIAN_EYES_Y 32
#define GILLIAN_EYES_WIDTH 24
#define GILLIAN_EYES_HEIGHT 8

static u16 gillian_blink_count = 0;
static u8 gillian_is_blinking = 0;
static u16 gillian_random_state = 0xACE1;
static u32 gillian_blink_vblank = ~0UL;
static const u8 *gillian_blink_frame = NULL;
static u8 gillian_open_eyes[GILLIAN_EYES_WIDTH * GILLIAN_EYES_HEIGHT]
	ATTR_CACHE_ALIGNED;
static u8 gillian_eye_dirty_pages = 0;

#define MENU_TEXT_CACHE_SLOTS 32
#define MENU_TEXT_CACHE_CHARS 48
typedef struct menu_text_cache_t {
	u32 hash;
	u16 length;
	u16 tilemapId;
	u16 rebuildId;
	int x;
	int y;
	int textColor;
	int shadowColor;
	u8 valid;
	char text[MENU_TEXT_CACHE_CHARS + 1];
} menu_text_cache_t;

static menu_text_cache_t menu_text_cache[2][MENU_TEXT_CACHE_SLOTS];
static u8 menu_text_index = 0;

volatile unsigned mars_pwdt_ovf_count = 0;
volatile unsigned mars_swdt_ovf_count = 0;

static const u32 crc32_table[] = {
	0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

int Mars_GetFRTCounter(void)
{
	unsigned int high = SH2_FRT_FRCH;
	unsigned int low = SH2_FRT_FRCL;

	return (int)((high << 8) | low);
}

static void drawMainBGPage(int withGillian)
{
	draw_tilemap(&tm, 0, 0, 0, NULL, NULL);
	draw_setScissor(0, 0, 320, 224);
	if (withGillian)
		drawGillian(216, 72);
}

static void beginMenuTextFrame(void)
{
	menu_text_index = 0;
}

static void redrawMenuTextTileOverlaps(int page, int skipSlot,
	int x, int y, int w, int h)
{
	int slot;
	int tileLeft = (x / tm.tw) * tm.tw;
	int tileTop = (y / tm.th) * tm.th;
	int tileRight = ((x + w - 1) / tm.tw + 1) * tm.tw;
	int tileBottom = ((y + h - 1) / tm.th + 1) * tm.th;

	for (slot = 0; slot < MENU_TEXT_CACHE_SLOTS; slot++)
	{
		menu_text_cache_t *cached = &menu_text_cache[page][slot];
		int textRight;
		int textBottom;

		if (slot == skipSlot || !cached->valid || !cached->text[0] ||
			cached->tilemapId != tm.id ||
			cached->rebuildId != canvas_rebuild_id)
			continue;

		textRight = cached->x + cached->length * 8 + 1;
		textBottom = cached->y + 10;
		if (cached->x < tileRight && textRight > tileLeft &&
			cached->y < tileBottom && textBottom > tileTop)
			drawTextwHighlight(cached->text, cached->x, cached->y,
				cached->textColor, cached->shadowColor);
	}
}

void invalidateMenuText(void)
{
	int page;
	int slot;

	for (page = 0; page < 2; page++)
		for (slot = 0; slot < MENU_TEXT_CACHE_SLOTS; slot++)
			menu_text_cache[page][slot].valid = 0;
	menu_text_index = 0;
}

void drawMenuTextwHighlight(const char *text, int x, int y,
	int textColor, int shadowColor)
{
	int page = (UNCACHED_CURRENT_FB ^ 1) & 1;
	u32 hash = 2166136261UL;
	u16 length = 0;
	u16 copyLength;
	int currentSlot;
	int i;
	menu_text_cache_t *entry;
	const char *p = text;

	while (*p)
	{
		hash ^= (u8)*p++;
		hash *= 16777619UL;
		length++;
	}

	if (menu_text_index >= MENU_TEXT_CACHE_SLOTS)
	{
		drawTextwHighlight(text, x, y, textColor, shadowColor);
		return;
	}

	currentSlot = menu_text_index++;
	entry = &menu_text_cache[page][currentSlot];
	if (entry->valid && entry->hash == hash && entry->length == length &&
		entry->tilemapId == tm.id && entry->rebuildId == canvas_rebuild_id &&
		entry->x == x && entry->y == y &&
		entry->textColor == textColor && entry->shadowColor == shadowColor)
		return;

	if (entry->valid && entry->tilemapId == tm.id &&
		entry->rebuildId == canvas_rebuild_id && entry->x == x && entry->y == y &&
		(entry->hash != hash || entry->length != length))
	{
		int clearLength = entry->length > length ? entry->length : length;
		draw_dirtyrect(&tm, x, y, clearLength * 8 + 1, 10);
		draw_tilemap(&tm, 0, 0, 0, NULL, NULL);
		redrawMenuTextTileOverlaps(page, currentSlot, x, y,
			clearLength * 8 + 1, 10);
	}

	drawTextwHighlight(text, x, y, textColor, shadowColor);
	entry->hash = hash;
	entry->length = length;
	entry->tilemapId = tm.id;
	entry->rebuildId = canvas_rebuild_id;
	entry->x = x;
	entry->y = y;
	entry->textColor = textColor;
	entry->shadowColor = shadowColor;
	copyLength = length > MENU_TEXT_CACHE_CHARS ?
		MENU_TEXT_CACHE_CHARS : length;
	for (i = 0; i < copyLength; i++)
		entry->text[i] = text[i];
	entry->text[copyLength] = 0;
	entry->valid = 1;
}

static void cacheMainBGBothPages(int withGillian)
{
	canvas_rebuild_id++;
	drawMainBGPage(withGillian);
	Hw32xScreenFlip(1);
	drawMainBGPage(withGillian);
	Hw32xScreenFlip(1);
}

void initMainBG()
{
	init_tilemap(&tm, &bg_map_Map, (const uint8_t * const *)bg_Reslist);
	Hw32xSetPalette(bg_Palette);
	loadTextPalette();
	invalidateMenuText();
	cacheMainBGBothPages(0);
}

void initMainBGwGil()
{
	int row;
	int col;

	init_tilemap(&tm, &bg_map_Map, (const uint8_t * const *)bg_Reslist);
	Hw32xSetPalette(gillian_menu_palette);
	loadTextPalette();
	for (row = 0; row < GILLIAN_EYES_HEIGHT; row++)
		for (col = 0; col < GILLIAN_EYES_WIDTH; col++)
			gillian_open_eyes[row * GILLIAN_EYES_WIDTH + col] =
				phase_check_gillian[(row + GILLIAN_EYES_Y) * GILLIAN_WIDTH +
					col + GILLIAN_EYES_X];
	gillian_blink_frame = NULL;
	gillian_eye_dirty_pages = 0;
	invalidateMenuText();
	cacheMainBGBothPages(1);
}

static u16 gillianRandom16(void)
{
	u16 carry = gillian_random_state & 1;
	gillian_random_state >>= 1;
	if (carry)
		gillian_random_state ^= 0xB400;
	return gillian_random_state;
}

int updateGillianBlink(void)
{
	u32 vblank = Hw32xGetTicks();

	if (vblank == gillian_blink_vblank)
		return 0;
	gillian_blink_vblank = vblank;

	gillian_blink_count++;
	if (gillian_blink_count <= 230)
		return 0;

	if (!gillian_is_blinking)
	{
		if (gillianRandom16() % 10 == 7)
		{
			gillian_blink_frame = gillian_blink_half;
			gillian_is_blinking = 1;
			gillian_blink_count = 230;
			return 1;
		}
		return 0;
	}

	if (gillian_blink_count == 232)
	{
		gillian_blink_frame = gillian_blink_half;
		return 1;
	}
	else if (gillian_blink_count == 234)
	{
		gillian_blink_frame = gillian_blink_closed;
		return 1;
	}
	else if (gillian_blink_count >= 236)
	{
		gillian_blink_frame = NULL;
		gillian_blink_count = 0;
		gillian_is_blinking = 0;
		return 1;
	}
	return 0;
}

void drawGillian(s16 x, s16 y)
{
	draw_sprite(x, y, GILLIAN_WIDTH, GILLIAN_HEIGHT,
		phase_check_gillian, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);

	if (gillian_blink_frame)
		draw_sprite(x + GILLIAN_EYES_X, y + GILLIAN_EYES_Y,
			GILLIAN_EYES_WIDTH, GILLIAN_EYES_HEIGHT, gillian_blink_frame,
			DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
}

void drawMainBG()
{
	beginMenuTextFrame();
	draw_setScissor(0, 0, 320, 224);
}

void drawBGwGil()
{
	perf_set_scene(PERF_SCENE_MENU);
	beginMenuTextFrame();
	draw_setScissor(0, 0, 320, 224);
	if (updateGillianBlink())
		gillian_eye_dirty_pages = 2;
	if (gillian_eye_dirty_pages)
	{
		const u8 *eyes = gillian_blink_frame ?
			gillian_blink_frame : gillian_open_eyes;
		draw_sprite(216 + GILLIAN_EYES_X, 72 + GILLIAN_EYES_Y,
			GILLIAN_EYES_WIDTH, GILLIAN_EYES_HEIGHT, eyes,
			DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
		gillian_eye_dirty_pages--;
	}
}

void redrawBGwGil()
{
	init_tilemap(&tm, &bg_map_Map, (const uint8_t * const *)bg_Reslist);
	Hw32xSetPalette(gillian_menu_palette);
	loadTextPalette();

	invalidateMenuText();
	cacheMainBGBothPages(1);
	gillian_eye_dirty_pages = 0;
}

void drawQRCode(u16 x, u16 y, u16 xWidth, u16 yWidth)
{
	// White
	setColor(32, 31, 31, 31);
	// Black
	setColor(33, 0, 0, 0);
	// Gray
	setColor(34, 23, 23,23);

	draw_sprite(248, 88, 32, 32, qrcode_tiles, DRAWSPR_OVERWRITE, 1);

}

void drawResolution()
{
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
	{
		drawMenuTextwHighlight("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorWhiteHighlight);
		if (segaCDDetectedAtBoot)
			drawMenuTextwHighlight("Sega CD 32X", 208, 208,
				fontColorWhite, fontColorWhiteHighlight);
		else
			drawMenuTextwHighlight("Genesis 32X", 208, 208,
				fontColorWhite, fontColorWhiteHighlight);
	} else {
		drawMenuTextwHighlight("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorWhiteHighlight);
		if (segaCDDetectedAtBoot)
			drawMenuTextwHighlight("Sega Mega-CD 32X", 168, 208,
				fontColorWhite, fontColorWhiteHighlight);
		else
			drawMenuTextwHighlight("Mega Drive 32X", 184, 208,
				fontColorWhite, fontColorWhiteHighlight);
	}
}

void loadTextPalette()
{
	Hw32xEnableTextPalette();
}

void loadMainBGwGilPalette()
{
	Hw32xSetPalette(gillian_menu_palette);
}

void cleanup()
{
	Hw32xScreenClear();
	Hw32xSetBGColor(0,0,0,0);
}

void marsVDP256Start(void)
{
	Hw32xInit(MARS_VDP_MODE_256, 0);
}

void marsVDP32KStart(void)
{
	Hw32xInit(MARS_VDP_MODE_32K, 0);
}

void swapBuffers()
{
	MARS_VDP_FBCTL = currentFB ^ 1;
	while ((MARS_VDP_FBCTL & MARS_VDP_FS) == currentFB) {}
	currentFB ^= 1;
}

/*
* CRC 32 based on work by Christopher Baker <https://christopherbaker.net>
*/

void CRC32_reset()
{
	_state = ~0L;
}

void CRC32_update(u8 data)
{
	u8 tbl_idx = 0;

	tbl_idx = _state ^ (data >> (0 * 4));
	_state = (*(u32*)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4));
	tbl_idx = _state ^ (data >> (1 * 4));
	_state = (*(u32*)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4));
}

u32 CRC32_finalize()
{
	return ~_state;
}

int memcmp1(const void *s1, const void *s2, int n)
{
	unsigned char u1, u2;

	for ( ; n-- ; s1++, s2++)
	{
		u1 = * (unsigned char *) s1;
		u2 = * (unsigned char *) s2;
		if ( u1 != u2) {
			return (u1-u2);
		}
	}
	return 0;
}

u32 CalculateCRC(u32 startAddress, u32 size)
{
	u8 *bios = NULL;
	u32 address = 0, checksum = 0;

	CRC32_reset();

	bios = (void*)startAddress;
	for (address = 0; address < size; address ++)
	{
		u8 data;

		data = bios[address];

		CRC32_update(data);
	}

	checksum = CRC32_finalize();
	return checksum;
}

void setRandomSeed(u16 seed)
{
	// xor it with a random value to avoid 0 value
	randbase = seed ^ 0xD94B;
}

u16 random16(void)
{
	randbase ^= (randbase >> 1) ^ MARS_SYS_COMM12;
	randbase ^= (randbase << 1);

	return randbase;
}
