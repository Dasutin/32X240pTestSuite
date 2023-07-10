/*
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2023 Artemio Urbina
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
#include "sd_pal.h"
#include "sd_tile.h"
#include "qrcode_tiles.h"

u8 paused = PAUSED;
u16 currentFB = 0;
vu16 overwriteImg16;
u32 _state = ~0L;
u16 randbase;

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
	unsigned int cnt = SH2_WDT_RTCNT;
	return (int)((mars_pwdt_ovf_count << 8) | cnt);
}

void initMainBG()
{
	init_tilemap(&tm, &bg_map_Map, (uint8_t **)bg_Reslist);
	Hw32xSetPalette(bg_Palette);
}

void initMainBGwGil()
{
	init_tilemap(&tm, &bg_map_Map, (uint8_t **)bg_Reslist);
	Hw32xSetPalette(bg_Palette);
	Hw32xSetPalette(sd_palette);
}

void drawMainBG()
{
	int fpcamera_x = 0;
	int fpcamera_y = 0;

	canvas_rebuild_id++;
	draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
	draw_setScissor(0, 0, 320, 224);
}

void drawBGwGil()
{
	int fpcamera_x = 0;
	int fpcamera_y = 0;

	canvas_rebuild_id++;
	draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
	draw_setScissor(0, 0, 320, 224);

	draw_sprite(216, 72, 64, 128, sd_sprite, DRAWSPR_OVERWRITE, 1);
}

void redrawBGwGil()
{
	Hw32xSetPalette(bg_Palette);
	Hw32xSetPalette(sd_palette);

	int fpcamera_x = 0;
	int fpcamera_y = 0;

	canvas_rebuild_id++;
	draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
	draw_setScissor(0, 0, 320, 224);

	draw_sprite(216, 72, 64, 128, sd_sprite, DRAWSPR_OVERWRITE, 1);
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
		drawTextwHighlight("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("Genesis 32X", 208, 208, fontColorWhite, fontColorWhiteHighlight);
	} else {
		drawTextwHighlight("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("Mega Drive 32X", 184, 208, fontColorWhite, fontColorWhiteHighlight);
	}
}

void loadTextPalette()
{
	setColor(203, 0, 0, 0); 	// Black Background
	setColor(205, 31, 31, 31);	// 204 is White
	setColor(206, 31, 0, 0);	// 205 is Red
	setColor(207, 0, 31, 0);	// 206 is Green
	setColor(208, 5, 5, 5);		// 207 is Gray
	setColor(209, 0, 0, 0);		// 208 is Black
	setColor(210, 8, 8, 8);		// 209 is White Shadow Highlight
	setColor(211, 8, 0, 0);		// 210 is Red Shadow Highlight
	setColor(212, 0, 8, 0);		// 211 is Green Shadow Highlight
}

void loadMainBGwGilPalette()
{
	Hw32xSetPalette(bg_Palette);
	Hw32xSetPalette(sd_palette);
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

u16 random()
{
	randbase ^= (randbase >> 1) ^ MARS_SYS_COMM12;
	randbase ^= (randbase << 1);

	return randbase;
}