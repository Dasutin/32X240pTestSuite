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
#include "string.h"
#include "32x_images.h"
#include "sound.h"
#include "draw.h"
#include "tests.h"
#include "shared_objects.h"
#include "help.h"
#include "donna_tiles.h"
#include "donna_res.h"
#include "donna_pal.h"
#include "buzz_res.h"
#include "buzz_pal.h"
#include "grid_tiles.h"
#include "grid_res.h"
#include "grid_pal.h"
#include "kiki.h"
#include "kiki_tiles.h"
#include "kiki_tiles_palette.h"
#include "marker_striped_res.h"
#include "background_fill.h"
#include "checkerboard.h"
#include "checkerboard_donna.h"
#include "h_stripes.h"
#include "v_stripes.h"
#include "numbers_res.h"
#include "lagtest_res_palette.h"
#include "lagtest_res.h"
#include "lagtest.h"
#include "sonic_tileset_palette.h"
#include "sonic_tileset.h"
#include "sonic_tilemap.h"
#include "sd_pal.h"
#include "sd_tile.h"
#include "marker_tiles.h"
#include "check_palette.h"
#include "check.h"
#include "check_map.h"
#include "horiz_palette.h"
#include "horiz.h"
#include "horiz_map.h"
#include "vert_palette.h"
#include "vert.h"
#include "vert_map.h"
#include "block_tiles.h"
#include "audiosync_tiles.h"

extern u32 schecksum;
unsigned mars_frtc2msec_frac = 0;
int fpcamera_x, fpcamera_y;
int fpmoveinc_x = 1<<16, fpmoveinc_y = 1<<16; // in 16.16 fixed point
uint16_t canvas_rebuild_id;
int window_canvas_x = 0, window_canvas_y = 0;
const int NTSC_CLOCK_SPEED = 23011360; // HZ
const int PAL_CLOCK_SPEED = 22801467; // HZ
int sec;
tilemap_t tm;

int Mars_FRTCounter2Msec(int c)
{
	return (c * mars_frtc2msec_frac) >> 16;
}

typedef struct bios_data {
	u32 crc;
	char *name;
} BIOSID;

static const BIOSID bioslist[] = {
	{ 0xDD9C46B8, "32X Master SH2 1.0" },	// 32X_M_BIOS.bin
	{ 0xBFDA1FE5, "32X Slave SH2 1.0" },	// 32X_S_BIOS.bin
	{ 0, NULL } };

int Detect32XMBIOS(u32 address)
{
	u8 *bios;

	bios = (u8 *)0+address;
	if (memcmp1(bios + 0x47E, "MARS", 4))
		return 0;
	return 1;
}

/* int Detect32XSBIOS(u32 address)
{
	u8 *bios;

	bios = (u8 *)0+address;
	if (memcmp1(bios + 0x1D4, "M", 1))
		return 0;
	return 1;
} */

char *GetBIOSNamebyCRC(u32 checksum)
{
	int i = 0;

	while (bioslist[i].crc != 0)
	{
		if (checksum == bioslist[i].crc)
			return bioslist[i].name;
		i++;
	}
	return 0;
}

void doMBIOSID(u32 checksum, u32 address)
{
	char *name = NULL;

	name = GetBIOSNamebyCRC(checksum);
	if (name)
	{
		HwMdPuts(name, 0x4000, 11, 20);
		return;
	}

	if (Detect32XMBIOS(address))
	{
		u32 mchecksum = 0;

		mchecksum = CalculateCRC(address, 0x0000800);

		ShowMessageAndData("32X M BIOS CRC32:", mchecksum, 0x2000, 8, 6, 18);
	}

	// No match! check if we find the SEGA string and report
	if (Detect32XMBIOS(address))
		HwMdPuts("Unknown BIOS, please report CRC", 0x2000, 4, 19);
	else
		HwMdPuts("BIOS not recognized", 0x2000, 8, 19);

	return;
}

void doSBIOSID(u32 checksum, u32 address)
{
	char *name = NULL;

	name = GetBIOSNamebyCRC(checksum);
	if (name)
	{
		HwMdPuts(name, 0x2000, 11, 21);
		return;
	}

/* 	if (Detect32XSBIOS(address))
	{
		//u32 schecksum = 0;

		//schecksum = CalculateCRC(address, 0x0000400);

		//ShowMessageAndData("32X S BIOS CRC32:", schecksum, 0x4000, 8, 6, 20);
	} */

	// No match! check if we find the SEGA string and report
/* 	if (Detect32XSBIOS(address))
		HwMdPuts("Unknown BIOS, please report CRC", 0x4000, 4, 21);
	else
		HwMdPuts("BIOS not recognized", 0x4000, 8, 21); */

	return;
}

void ShowMessageAndData(char *message, u32 address, int color, int len, int xpos, int ypos)
{
	int msglen = 0;
	char buffer[40];

	intToHex(address, buffer, len);

	msglen = strlen(message);
	HwMdPuts(message, color, xpos, ypos);
	HwMdPuts(" 0x", color, xpos+msglen, ypos);
	HwMdPuts(buffer, color, xpos+msglen+3, ypos);
}

void MDPSG_init()
{
	HwMdPSGSetTone(0,0);
	HwMdPSGSetTone(1,0);
	HwMdPSGSetTone(2,0);
	HwMdPSGSetTone(3,0);

	HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(1, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(2, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(3, PSG_ENVELOPE_MIN);
}

void MDPSG_stop()
{
	HwMdPSGSetChandVol(0,15);
	HwMdPSGSetChandVol(1,15);
	HwMdPSGSetChandVol(2,15);
	HwMdPSGSetChandVol(3,15);
}

void vt_drop_shadow_test()
{
	int done = 0, frameCount = 0, mode = DRAWSPR_OVERWRITE, x = 30, y = 30;
	int otherTests = 1, changeSprite = 0, background = 1, initTilemap = 1, evenFrames = 0;
	u16 frame = 1;
	unsigned short button = 0, pressedButton = 0, oldButton = 0xFFFF;

	Hw32xSetPalette(donna_palette);

	fpcamera_x = 262145;
	fpcamera_y = 0;

	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!otherTests)
		{
			switch (frame)
			{
			case 30:
				Hw32xSetPalette(sonic_tileset_Palette2);
				break;

			case 60:
				Hw32xSetPalette(sonic_tileset_Palette3);
				break;

			case 90:
				Hw32xSetPalette(sonic_tileset_Palette);
				break;
			}
		}

		frame++;
		if (frame > 90)
			frame = 1;

		if (button & SEGA_CTRL_UP)
		{
			y--;
			if (y < 0)
				y = 0;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			y++;
			if (y > 192)
				y = 192;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			mode = DRAWSPR_PRECISE;
			x--;
			if (x < 30)
				x = 30;

			fpcamera_x -= (fpmoveinc_x * 2);

			if (fpcamera_x <= 262145)
				fpcamera_x = 262145;
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			mode = DRAWSPR_HFLIP;
			x++;
			if (x > 288)
				x = 288;

			fpcamera_x += (fpmoveinc_x * 2);

			if (fpcamera_x >= 34340865)
				fpcamera_x = 34340865;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			//TODO Even/Odd shadow code goes here
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			initTilemap = 1;
			background++;

			if (background > 4)
				background = 1;
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			if (changeSprite == 0)
				changeSprite = 1;
			else
				changeSprite = 0;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			canvas_pitch = 320;
			DrawHelp(HELP_SHADOW);
			if (!otherTests)
				canvas_pitch = 384;
			Hw32xSetPalette(donna_palette);
			initTilemap = 1;
		}

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		switch (background)
		{
			case 1:
				if (initTilemap == 1)
				{
					otherTests = 1;
					canvas_pitch = 320;
					Hw32xSetPalette(donna_palette);
					init_tilemap(&tm, &donna_tmx, (uint8_t **)donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 2:
				if (initTilemap == 1)
				{
					init_tilemap(&tm, &checkerboard_donna_tmx, (uint8_t **)checkerboard_donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 3:
				if (initTilemap == 1)
				{
					otherTests = 1;
					init_tilemap(&tm, &h_stripes_tmx, (uint8_t **)h_stripes_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 4:
				if (initTilemap == 1)
				{
					otherTests = !otherTests;
					canvas_pitch = 384;
					Hw32xSetPalette(sonic_tileset_Palette);
					init_tilemap(&tm, &sonic_tilemap_Map, (uint8_t **)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		if (changeSprite == 0)
		{
			if (frameCount % 2 == evenFrames)
				draw_sprite(x, y, 32, 32, buzz_shadow_sprite, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | mode, 1);

			draw_sprite(x-20, y-20, 32, 32, buzz_sprite, DRAWSPR_OVERWRITE | mode | DRAWSPR_PRECISE, 1);
		} else {
			if (frameCount % 2 == evenFrames)
				draw_sprite(x, y, 32, 32, marker_shadow_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | mode, 1);
		}

		frameCount++;

		Hw32xDelay(1);
		Hw32xScreenFlip(0);
	}
	return;
}

void vt_striped_sprite_test()
{
	int done = 0, x = 30, y = 30, otherTests = 1, background = 1, initTilemap = 1;
	u16 frame = 1;
	unsigned short button = 0, pressedButton = 0, oldButton = 0xFFFF;

	Hw32xSetPalette(donna_palette);

	fpcamera_x = 262145;
	fpcamera_y = 0;

	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!otherTests)
		{
			switch (frame)
			{
			case 30:
				Hw32xSetPalette(sonic_tileset_Palette2);
				break;

			case 60:
				Hw32xSetPalette(sonic_tileset_Palette3);
				break;

			case 90:
				Hw32xSetPalette(sonic_tileset_Palette);
				break;
			}
		}

		frame++;
		if (frame > 90)
			frame = 1;

		if (button & SEGA_CTRL_UP)
		{
			y--;
			if (y < 0)
				y = 0;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			y++;
			if (y > 192)
				y = 192;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			x--;
			if (x < 0)
				x = 0;

			fpcamera_x -= fpmoveinc_x;

			if (fpcamera_x <= 262145)
				fpcamera_x = 262145;
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			x++;
			if (x > 288)
				x = 288;

			fpcamera_x += fpmoveinc_x;

			if (fpcamera_x >= 17235969)
				fpcamera_x = 17235969;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			initTilemap = 1;
			background++;

			if (background > 4)
				background = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			initTilemap = 1;
			background--;

			if (background < 1)
				background = 4;
		}

		Hw32xFlipWait();

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				canvas_pitch = 320;
				DrawHelp(HELP_STRIPED);
				if (!otherTests)
					canvas_pitch = 384;

				Hw32xSetPalette(donna_palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			canvas_pitch = 320;
			DrawHelp(HELP_STRIPED);
			if (!otherTests)
				canvas_pitch = 384;

			Hw32xSetPalette(donna_palette);
			initTilemap = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		switch (background)
		{
			case 1:
				if (initTilemap == 1)
				{
					otherTests = 1;
					canvas_pitch = 320;
					Hw32xSetPalette(donna_palette);
					init_tilemap(&tm, &donna_tmx, (uint8_t **)donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 2:
				if (initTilemap == 1)
				{
					init_tilemap(&tm, &checkerboard_donna_tmx, (uint8_t **)checkerboard_donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 3:
				if (initTilemap == 1)
				{
					otherTests = 1;
					canvas_pitch = 320;
					Hw32xSetPalette(donna_palette);
					init_tilemap(&tm, &h_stripes_tmx, (uint8_t **)h_stripes_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 4:
				if (initTilemap == 1)
				{
					otherTests = !otherTests;
					canvas_pitch = 384;
					Hw32xSetPalette(sonic_tileset_Palette);
					init_tilemap(&tm, &sonic_tilemap_Map, (uint8_t **)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);
		draw_sprite(x, y, 32, 32, marker_striped_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);

		Hw32xDelay(1);
		Hw32xScreenFlip(0);
	}
	return;
}

void vt_lag_test()
{
	u16 lsd, msd;
	int frames = 0, seconds = 0, minutes = 0, hours = 0, framecnt = 1;
	u16 done = 0;
	uint8_t *numbers[10] = {tiles_0, tiles_1, tiles_2, tiles_3, tiles_4, tiles_5, tiles_6, tiles_7, tiles_8, tiles_9};
	u16 pause = 0, cposx = 32;
	//u16 cposy = 17;
	u16 button, pressedButton, oldButton = 0xFFFF;
	volatile unsigned short *cram16 = &MARS_CRAM;

	canvas_pitch = 320;
	canvas_yaw = 224;

	Hw32xSetPalette(lagtest_res_Palette);
	loadTextPalette();

	MARS_SYS_COMM4 = 0;
	MARS_SYS_COMM6 = 0;

	fpcamera_x = fpcamera_y = 0;

	init_tilemap(&tm, &lagtest_Map, (uint8_t **)lagtest_res_Reslist);

	Hw32xScreenFlip(0);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (framecnt > 8)
			framecnt = 1;

		if (framecnt == 1)
			cram16[3] = 0x001F;
		else
			cram16[3] = 0x7C00;

		if (framecnt == 2)
			cram16[4] = 0x001F;
		else
			cram16[4] = 0x7C00;

		if (framecnt == 3)
			cram16[5] = 0x001F;
		else
			cram16[5] = 0x7C00;

		if (framecnt == 4)
			cram16[6] = 0x001F;
		else
			cram16[6] = 0x7C00;

		if (framecnt == 5)
			cram16[7] = 0x001F;
		else
			cram16[7] = 0x7C00;

		if (framecnt == 6)
			cram16[8] = 0x001F;
		else
			cram16[8] = 0x7C00;

		if (framecnt == 7)
			cram16[9] = 0x001F;
		else
			cram16[9] = 0x7C00;

		if (framecnt == 8)
			cram16[10] = 0x001F;
		else
			cram16[10] = 0x7C00;

		if (framecnt % 2 == 0)
			cram16[1] = 0x0000;
		else
			cram16[1] = 0x7FFF;

		if (framecnt > 4)
		{
			cposx = framecnt - 4;
			//cposy = 17;
		} else {
			cposx = framecnt;
			//cposy = 9;
		}
		cposx = (cposx - 1) * 10 + 2;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_LAG);
				Hw32xSetPalette(background_fill_palette1);
				init_tilemap(&tm, &lagtest_Map, (uint8_t **)lagtest_res_Reslist);
				canvas_rebuild_id++;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_LAG);
			Hw32xSetPalette(background_fill_palette1);
			init_tilemap(&tm, &lagtest_Map, (uint8_t **)lagtest_res_Reslist);
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_A)
			pause = !pause;

		if (pressedButton & SEGA_CTRL_B && pause)
		{
			frames = hours = minutes = seconds = 0;
			framecnt = 1;
		}

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		drawText("hours", 32, 8, fontColorBlack);
		drawText("minutes", 104, 8, fontColorBlack);
		drawText("seconds", 176, 8, fontColorBlack);
		drawText("frames", 248, 8, fontColorBlack);

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		if (!pause)
		{
			frames ++;
			framecnt ++;
			if (framecnt > 8)
				framecnt = 1;
		}

		if (frames > 59)
		{
			frames = 0;
			seconds ++;
		}

		if (seconds > 59)
		{
			seconds = 0;
			minutes ++;
		}

		if (minutes > 59)
		{
			minutes = 0;
			hours ++;
		}

		if (hours > 99)
			hours = 0;

		draw_sprite(80, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);
		draw_sprite(152, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);
		draw_sprite(224, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);

		// Draw Hours
		lsd = hours % 10;
		msd = hours / 10;
		draw_sprite(32, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(56, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw Minutes
		lsd = minutes % 10;
		msd = minutes / 10;
		draw_sprite(104, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(128, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw Seconds
		lsd = seconds % 10;
		msd = seconds / 10;
		draw_sprite(176, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(200, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw frames
		lsd = frames % 10;
		msd = frames / 10;
		draw_sprite(248, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(272, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_reflex_test()
{
	int fpcamera_x = 0, fpcamera_y = 0;
	char str[10];
	s16 speed = 1, vary = 0, clicks[10];
	u16 pal = 0x0000, change = 1, loadvram = 1, psgoff = 0, usersound = 0;
	u16 x = 0, y = 0, x2 = 0, y2 = 0, done = 0, variation = 1, draw = 1;
	u16 pos = 0, view = 0, audio = 1, drawoffset = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	x = 144, y = 60, x2 = 108, y2 = 96;

	MDPSG_init();
	HwMdPSGSetFrequency(0, 1000);

	// Set screen priority for the 32X
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	setColor(1, 0, 0, 0);
	setColor(2, 31, 31, 31);

	init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);

	while (!done)
	{
		Hw32xFlipWait();

		canvas_rebuild_id++;
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		setColor(1, 0, 0, 0);

		if (loadvram)
		{
			HwMdPuts("Press the \"A\" button when the sprite", 0x4000, 2, 21);
			HwMdPuts("is aligned. A negative value means", 0x4000, 2, 22);
			HwMdPuts("you pressed \"A\" before they intersect.", 0x4000, 2, 23);
			HwMdPuts("\"B\" button toggles horz/vert", 0x4000, 2, 24);
			HwMdPuts("\"C\" button toggles audio", 0x4000, 2, 25);
			HwMdPuts("DOWN toggles random/rhythmic", 0x4000, 2, 26);

			loadvram = 0;
			draw = 1;

			if (pos)
			{
				int i = 0;
				u16 ppos = 0;

				for (i = 0; i < pos; i++)
				{
					pal = 0x0000;

					intToStr(clicks[i], str, 1);

					if (clicks[i] == 0)
						pal = 0x4000;
					if (clicks[i] < 0)
						pal = 0x2000;

					ppos = i + 1;
					intToStr(ppos, str, 1);
					HwMdPuts("Offset", 0x0000, 2, i);
					HwMdPuts(str, 0x0000, 9, i);
					HwMdPuts(":", 0x0000, i == 9 ? 11 : 10, i);
					intToStr(clicks[i], str, 1);
					HwMdPuts(str, pal, i == 9 ? 13 : 12, i);
					ppos = strlen(str);
					if (clicks[i] == 1)
						HwMdPuts(" frame    ", pal, pos == 9 ? 13 : 12 + ppos, i);
					else
						HwMdPuts(" frames   ", pal, pos == 9 ? 13 : 12 + ppos, i);
				}
			}
		}

		if (y == 96) // Screen Flash
		{
			if (audio && !usersound)
				HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);

			setColor(1, 5, 5, 5);
		}

		if (usersound)
		{
			HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
			usersound = 0;
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			MDPSG_stop();
			DrawHelp(HELP_MANUALLAG);
			vt_reflex_test();
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (change)
			{
				clicks[pos] = (y - 96) * speed;
				drawoffset = 1;
				if (clicks[pos] >= 0)
					change = 0;

				if (audio)
				{
					if (clicks[pos] == 0)
					{
						HwMdPSGSetChandVol(0, 0);
						HwMdPSGSetFrequency(0, 1000);
						if (psgoff == 0)
							psgoff = 2;
					} else {
						HwMdPSGSetChandVol(0, 0);
						HwMdPSGSetFrequency(0, 500);
						if (psgoff == 0)
							psgoff = 2;
					}
					usersound = 1;
				}
			}
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			view++;
			if (view > 2)
				view = 0;
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			audio = !audio;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			variation = !variation;
			if (!variation)
				vary = 0;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			MDPSG_stop();
			done = 1;
		}

		if (drawoffset)
		{
			u16 ppos = 0;

			pal = 0x0000;
			intToStr(clicks[pos], str, 1);

			if (clicks[pos] == 0)
				pal = 0x4000;
			if (clicks[pos] < 0)
				pal = 0x2000;

			ppos = pos + 1;
			intToStr(ppos, str, 1);
			HwMdPuts("Offset", 0x0000, 2, pos);
			HwMdPuts(str, 0x0000, 9, pos);
			HwMdPuts(":", 0x0000, pos == 9 ? 11 : 10, pos);
			intToStr(clicks[pos], str, 1);
			HwMdPuts(str, pal, pos == 9 ? 13 : 12, pos);
			ppos = strlen(str);

			if (clicks[pos] == 1)
				HwMdPuts(" frame    ", pal, pos == 9 ? 13 : 12 + ppos, pos);
			else
				HwMdPuts(" frames   ", pal, pos == 9 ? 13 : 12 + ppos, pos);

			if (clicks[pos] >= 0)
				pos++;

			if (pos > 9)
				done = 1;

			drawoffset = 0;
		}

		if (draw)
		{
			HwMdPuts("Audio:", 0x0000, 24, 0);

			if (audio)
				HwMdPuts("on ", 0x0000, 31, 0);
			else
				HwMdPuts("off", 0x0000, 31, 0);

			HwMdPuts("Timing:", 0x0000, 24, 1);

			if (variation)
				HwMdPuts("random  ", 0x0000, 32, 1);
			else
				HwMdPuts("rhythmic", 0x0000, 32, 1);

			draw = 0;
		}

		if (y > 132 + vary)
		{
			speed = -1;
			change = 1;
			if (variation)
			{
				if (random() % 2)
					vary = random() % 7;
				else
					vary = -1 * random() % 7;
			}
		}

		if (y < 60 + vary)
		{
			speed = 1;
			change = 1;

			if (variation)
			{
				if (random() % 2)
					vary = random() % 7;
				else
					vary = -1 * random() % 7;
			}
		}

		y += speed;
		x2 += speed;

		draw_sprite(x, 96, 32, 32, marker3_tile, DRAWSPR_OVERWRITE, 1);
		draw_sprite(x, y, 32, 32, marker1_tile, DRAWSPR_OVERWRITE, 1);

		// Red on the spot
		if (y == 96)
			setColor(2, 31, 0, 0);

		// Green one pixel before or after
		if (y == 95 || y == 97)
			setColor(2, 0, 31, 0);

		// Back to white two pixels before or after
		if (y == 98 || y == 94)
			setColor(2, 31, 31, 31);

		if (view == 0 || view == 2)
			draw_sprite(x, y, 32, 32, marker1_tile, DRAWSPR_OVERWRITE, 1);
		else
			draw_sprite(320, 224, 32, 32, marker1_tile, DRAWSPR_OVERWRITE, 1);

		if (view == 1 || view == 2)
			draw_sprite(x2, y2, 32, 32, marker2_tile, DRAWSPR_OVERWRITE, 1);
		else
			draw_sprite(320, 224, 32, 32, marker2_tile, DRAWSPR_OVERWRITE, 1);

		// Half the screen?
		if (y == 96)
		{
			if (audio)
			{
				HwMdPSGSetChandVol(0, 0);
				HwMdPSGSetFrequency(0, 1000);
				if (psgoff == 0)
					psgoff = 2;
			}
			setColor(1, 5, 5, 5);
		}

		if (psgoff)
		{
			psgoff--;
			if (psgoff == 0)
				MDPSG_stop();
		}

		Hw32xScreenFlip(0);
	}

	if (pos > 9)
	{
		int totald = 0, cnt, tot;
		u16 total = 0, count = 0, c = 0;

		MDPSG_stop();
		HwMdClearScreen();
		initMainBG();
		Hw32xScreenFlip(0);
		Hw32xFlipWait();
		HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
		drawMainBG();

		for (c = 0; c < 10; c++)
		{
			if (clicks[c] != 0xFF)
			{
				intToStr(clicks[c], str, 1);

				pal = 0x0000;
				if (clicks[c] == 0)
					pal = 0x2000;

				if (clicks[c] < 0)
					pal = 0x4000;

				if (clicks[c] >= 0)
				{
					total += clicks[c];
					count++;
				}
			}
			HwMdPuts(str, pal, 10, c + 7);
		}

		HwMdPuts("+", 0x4000, 8, 11);

		if (count > 0)
		{
			u16 h = 10, v = 18;
			fix32 framerate = MARS_VDP_DISPMODE & MARS_NTSC_FORMAT ? FIX32(16.6884) : FIX32(20.1196);

			HwMdPuts("----", 0x0000, h - 2, v++);

			cnt = intToFix32(count);
			tot = intToFix32(total);

			intToStr(total, str, 1);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			HwMdPuts("/", 0x4000, h++, v);

			intToStr(count, str, 1);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			HwMdPuts("=", 0x4000, h++, v);

			totald = fix32Div(tot, cnt);
			fix32ToStr(totald, str, 4);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			h -= 2;
			HwMdPuts("frames", 0x0000, h, v);
			HwMdPuts("These are your reflexes, not a", 0x0000, 5, ++v);
			HwMdPuts("lag test. A frame is", 0x0000, 5, ++v);
			fix32ToStr(framerate, str, 6);
			HwMdPuts(str, 0x0000, 26, v);
			HwMdPuts("ms.", 0x0000, 32, v);

			h = 14, v = 12;
			cnt = fix32Mul(totald, framerate);
			fix32ToStr(cnt, str, 2);
			HwMdPuts(str, 0x2000, h, v);
			h += strlen(str);
			HwMdPuts(" milliseconds", 0x0000, h, v);

			if (total < 5)
				HwMdPuts("EXCELLENT REFLEXES!", 0x4000, 14, 15);

			if (total == 0)
				HwMdPuts("INCREDIBLE REFLEXES!!", 0x4000, 14, 15);
		}

		Hw32xScreenFlip(0);

		done = 0;

		while (!done)
		{
			button = MARS_SYS_COMM8;

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;

			pressedButton = button & ~oldButton;
			oldButton = button;

			if (pressedButton & SEGA_CTRL_START || pressedButton & SEGA_CTRL_B)
				done = 1;
		}
	}
}

void vt_scroll_test()
{
	int done = 0, pause = 0, direction = 0, acc = 1;
	u16 frame = 1, vertical = 0, initTilemap = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	// canvas_width + scrollwidth
	canvas_pitch = 384;

	Hw32xSetPalette(sonic_tileset_Palette);

	fpcamera_x = 262145;
	fpcamera_y = 0;

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &sonic_tilemap_Map, (uint8_t **)sonic_tileset_Reslist);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!vertical)
		{
			switch (frame)
			{
			case 30:
				Hw32xSetPalette(sonic_tileset_Palette2);
				break;

			case 60:
				Hw32xSetPalette(sonic_tileset_Palette3);
				break;

			case 90:
				Hw32xSetPalette(sonic_tileset_Palette);
				break;
			}
		}

		frame++;
		if (frame > 90)
			frame = 1;

		if (!vertical)
		{
			if (!pause)
			{
				if (!direction)
					fpcamera_x += (fpmoveinc_x * acc);
				else
					fpcamera_x -= (fpmoveinc_x * acc);
			}
		} else {
			if (!pause)
			{
				if (!direction)
					fpcamera_y += (fpmoveinc_y * acc);
				else
					fpcamera_y -= (fpmoveinc_y * acc);
			}
		}

		if (pressedButton & SEGA_CTRL_LEFT)
			direction = !direction;

		if (pressedButton & SEGA_CTRL_UP)
		{
			acc++;
			if (acc == 10)
				acc = 10;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			acc--;
			if (acc == 1)
				acc = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
			pause = !pause;

		if (pressedButton & SEGA_CTRL_B)
		{
			if (!vertical)
			{
				canvas_yaw = 256;
				canvas_pitch = 320;
				vertical = 1;
				initTilemap = 1;
			} else {
				canvas_pitch = 384;
				canvas_yaw = 224;
				vertical = 0;
				initTilemap = 1;
			}
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				if (!vertical)
				{
					canvas_pitch = 320;
					DrawHelp(HELP_HSCROLL);
					canvas_pitch = 384;
				} else {
					canvas_yaw = 224;
					DrawHelp(HELP_HSCROLL);
					canvas_yaw = 256;
				}
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			if (!vertical)
			{
				canvas_pitch = 320;
				DrawHelp(HELP_HSCROLL);
				canvas_pitch = 384;
				initTilemap = 1;
			} else {
				canvas_yaw = 224;
				DrawHelp(HELP_HSCROLL);
				canvas_yaw = 256;
				initTilemap = 1;
			}
		}

		if (!vertical)
		{
			if (fpcamera_x < 0) fpcamera_x = sonic_tilemap_Map.wrapX*(1<<16);
		} else {
			if (fpcamera_y < 0) fpcamera_y = kiki_Map.wrapY*(1<<16);
		}

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (!vertical)
		{
			if (initTilemap == 1)
				{
					Hw32xSetPalette(sonic_tileset_Palette);
					init_tilemap(&tm, &sonic_tilemap_Map, (uint8_t **)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
			draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
			draw_setScissor(0, 0, 320, 224);
		} else {
			if (initTilemap == 1)
				{
					marsVDP256Start();
					Hw32xSetPalette(kiki_tiles_Palette);
					initTilemap = 0;
				}
			canvas_rebuild_id++;
			init_tilemap(&tm, &kiki_Map, (uint8_t **)kiki_tiles_Reslist);
			draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
			draw_setScissor(0, 0, 320, 224);
		}

		frame++;

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_gridscroll_test()
{
	int done = 0, acc = 1, pause = 0, direction = 0, horizontal = 0, fpcamera_x = fpcamera_y = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	// canvas_width + scrollwidth
	canvas_pitch = 384;
	// canvas_height + scrollheight
	canvas_yaw = 256;

	Hw32xSetPalette(grid_palette);

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &grid_tmx, (uint8_t **)grid_reslist);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!horizontal)
		{
			if (!pause)
			{
				if (!direction)
					fpcamera_y += (fpmoveinc_y * acc);
				else
					fpcamera_y -= (fpmoveinc_y * acc);
			}
		} else {
			if (!pause)
			{
				if (!direction)
					fpcamera_x += (fpmoveinc_x * acc);
				else
					fpcamera_x -= (fpmoveinc_x * acc);
			}
		}

		if (pressedButton & SEGA_CTRL_A)
			pause = !pause;

		if (pressedButton & SEGA_CTRL_B)
			horizontal = !horizontal;

		if (pressedButton & SEGA_CTRL_LEFT)
			direction = !direction;

		if (pressedButton & SEGA_CTRL_UP)
		{
			acc++;
			if (acc == 10)
				acc = 10;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			acc--;
			if (acc == 1)
				acc = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				canvas_pitch = 320;
				canvas_yaw = 224;
				DrawHelp(HELP_VSCROLL);
				Hw32xSetPalette(grid_palette);
				canvas_pitch = 384;
				canvas_yaw = 256;
				init_tilemap(&tm, &grid_tmx, (uint8_t **)grid_reslist);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			canvas_pitch = 320;
			canvas_yaw = 224;
			DrawHelp(HELP_VSCROLL);
			Hw32xSetPalette(grid_palette);
			canvas_pitch = 384;
			canvas_yaw = 256;
			init_tilemap(&tm, &grid_tmx, (uint8_t **)grid_reslist);
		}

		if (!pause)

		if (fpcamera_x < 0)
			fpcamera_x = grid_tmx.wrapX * (1 << 16);
		if (fpcamera_y < 0)
			fpcamera_y = grid_tmx.wrapY * (1 << 16);

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		canvas_rebuild_id++;
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_stripes()
{
	char cntstr[4];
	//u16 hor1 = 0, hor2 = 0, ver1 = 0, ver2 = 0, size = 0;
	u16 count = 0, docounter = 0;
	u16 field = 1, alternate = 0, done = 0, vertical = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	Hw32xSetPalette(horiz_Palette);
	init_tilemap(&tm, &vert_map_Map, (uint8_t **)vert_Reslist);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		if (alternate)
		{
			if (field == 0)
			{
				if (vertical)
				{
					init_tilemap(&tm, &vert_map_Map, (uint8_t **)vert_Reslist);
				} else {
					init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
				}
				field = 1;
			} else {
				if (vertical)
				{
					init_tilemap(&tm, &vert_map_Map, (uint8_t **)vert_Reslist);
				} else {
					init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
				}
				field = 0;
			}
			canvas_rebuild_id++;
		}

		if (docounter)
		{
			count++;
			if (count > 59)
				count = 0;

			intToStr(count, cntstr, 2);
			drawTextwBackground("Frame: ", 16, 200, fontColorWhite);
			drawTextwBackground(cntstr, 64, 200, fontColorWhite);
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_STRIPES);
				init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
				canvas_rebuild_id++;
				setColor(0, 31, 31, 31);
				setColor(1, 0, 0, 0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_STRIPES);
			init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
			canvas_rebuild_id++;
			setColor(0, 31, 31, 31);
			setColor(1, 0, 0, 0);
		}

		if (pressedButton & SEGA_CTRL_A)
			alternate = ~alternate;

		if (!alternate && (pressedButton & SEGA_CTRL_UP || pressedButton & SEGA_CTRL_DOWN))
		{
			if (field == 0)
			{
				if (vertical)
				{
					init_tilemap(&tm, &vert_map_Map, (uint8_t **)vert_Reslist);
				} else {
					init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
				}
				field = 1;
			} else {
				if (vertical)
				{
					init_tilemap(&tm, &vert_map_Map, (uint8_t **)vert_Reslist);
				} else {
					init_tilemap(&tm, &horiz_map_Map, (uint8_t **)horiz_Reslist);
				}
				field = 0;
			}
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_LEFT || pressedButton & SEGA_CTRL_RIGHT)
		{
			vertical = ~vertical;
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_B)
			docounter = ~docounter;

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_checkerboard()
{
	int done = 0, test = 1, pal = 1, manualtest = 1, fpcamera_x = 0, fpcamera_y = 0;
	int docounter = 0, count = 0;
	char str[2];
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	Hw32xSetPalette(check_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		if (docounter)
		{
			count++;
				if (count > 59)
					count = 0;

			drawTextwBackground("Frame: ", 16, 200, fontColorWhite);
			intToStr(count, str, 2);
			drawTextwBackground(str, 64, 200, fontColorWhite);
		} else if (!docounter) {
			canvas_rebuild_id++;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		switch (test)
		{
			case 1:
				init_tilemap(&tm, &check_map_Map, (uint8_t **)check_Reslist);
				break;

			case 2:
				switch (pal)
				{
					case 1:
						setColor(0, 0, 0, 0);
						setColor(1, 31, 31, 31);
						break;

					case 2:
						setColor(0, 31, 31, 31);
						setColor(1, 0, 0, 0);
						break;
				}
				break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (test == 1)
				test = 2;
			else
				test = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			docounter = !docounter;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			if (manualtest > 2)
				manualtest = 1;

			switch (manualtest)
			{
				case 1:
					test = 1;
					setColor(0, 31, 31, 31);
					setColor(1, 0, 0, 0);
					break;

				case 2:
					test = 1;
					setColor(0, 0, 0, 0);
					setColor(1, 31, 31, 31);
					break;
			}
			manualtest++;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			if (manualtest > 2)
				manualtest = 1;

			switch (manualtest)
			{
				case 1:
					test = 1;
					setColor(0, 31, 31, 31);
					setColor(1, 0, 0, 0);
					break;

				case 2:
					test = 1;
					setColor(0, 0, 0, 0);
					setColor(1, 31, 31, 31);
					break;
			}
			manualtest++;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_CHECK);
				init_tilemap(&tm, &check_map_Map, (uint8_t **)check_Reslist);
				canvas_rebuild_id++;
				setColor(0, 31, 31, 31);
				setColor(1, 0, 0, 0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CHECK);
			init_tilemap(&tm, &check_map_Map, (uint8_t **)check_Reslist);
			canvas_rebuild_id++;
			setColor(0, 31, 31, 31);
			setColor(1, 0, 0, 0);
		}

		pal++;

		if (pal > 2)
			pal = 1;

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_backlitzone_test()
{
	int done = 0, x = 160, y = 112, block = 2;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(background_fill_palette);

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			block++;
			if (block > 5)
				block = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
			block = 6;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_LED);
				Hw32xSetPalette(background_fill_palette);
				init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_LED);
			Hw32xSetPalette(background_fill_palette);
			init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);
		}

		if (button & SEGA_CTRL_UP)
		{
			y--;
			if (y < 1)
				y = 1;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			y++;
			if (y > 222)
				y = 222;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			x--;
			if (x < 1)
				x = 1;
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			x++;
			if (x > 320)
				x = 320;
		}

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		canvas_rebuild_id++;
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		switch (block)
		{
			case 1:
				draw_sprite(x, y, 8, 8, block_1x1_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;

			case 2:
				draw_sprite(x, y, 8, 8, block_2x2_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;

			case 3:
				draw_sprite(x, y, 8, 8, block_4x4_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;

			case 4:
				draw_sprite(x, y, 8, 8, block_6x6_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;

			case 5:
				draw_sprite(x, y, 8, 8, block_8x8_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;

			case 6:
				draw_sprite(x, y, 8, 8, block_0x0_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE, 1);
				break;
		}

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_DisappearingLogo()
{
	u16 lsd, msd;
	u16 done = 0, draw = 1, reload = 1, redraw = 1;
	u16 frame = 2, frames = 0;
	int seconds = 0, minutes = 0, hours = 0, framecnt = 1;
	uint8_t *numbers[10] = {tiles_0, tiles_1, tiles_2, tiles_3, tiles_4, tiles_5, tiles_6, tiles_7, tiles_8, tiles_9};
	//timecode tc;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(sd_palette_original);

	MARS_SYS_COMM4 = 0;
	MARS_SYS_COMM6 = 0;

	fpcamera_x = fpcamera_y = 0;

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);

	while (!done)
	{
		if (reload)
			redraw = 1;

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_DISAPPEAR);
			Hw32xSetPalette(sd_palette_original);
			init_tilemap(&tm, &background_fill_tmx, (uint8_t **)background_fill_reslist);
		}

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_A)
		{
			draw = !draw;
			redraw = 1;
		}

		if (frame)
		{
			frame--;
			if (!frame)
			{
				Hw32xSetPalette(sd_palette_original);
			}
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			if (!frame)
			{
				setColor(1, 31, 31, 31);
				frame = 2;
			}
		}

		Hw32xFlipWait();

		canvas_rebuild_id++;
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		frames ++;
		framecnt ++;

		if (frames > 59)
		{
			frames = 0;
			seconds ++;
		}

		if (seconds > 59)
		{
			seconds = 0;
			minutes ++;
		}

		if (minutes > 59)
		{
			minutes = 0;
			hours ++;
		}

		if (hours > 99)
			hours = 0;

		draw_sprite(80, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);
		draw_sprite(152, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);
		draw_sprite(224, 19, 32, 32, separator_tile, DRAWSPR_OVERWRITE, 1);

		// Draw Hours
		lsd = hours % 10;
		msd = hours / 10;
		draw_sprite(32, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(56, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw Minutes
		lsd = minutes % 10;
		msd = minutes / 10;
		draw_sprite(104, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(128, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw Seconds
		lsd = seconds % 10;
		msd = seconds / 10;
		draw_sprite(176, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(200, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		// Draw frames
		lsd = frames % 10;
		msd = frames / 10;
		draw_sprite(248, 19, 32, 32, numbers[msd], DRAWSPR_OVERWRITE, 1);
		draw_sprite(272, 19, 32, 32, numbers[lsd], DRAWSPR_OVERWRITE, 1);

		if (redraw)
		{
			if (draw)
				draw_sprite(128, 70, 64, 128, sd_sprite, DRAWSPR_OVERWRITE, 1);
			redraw = 0;
		}

		Hw32xScreenFlip(0);
	}
	return;
}

void at_sound_test()
{
	u16 done = 0, xcurse = 2, ycurse = 1, psgoff = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	sound_t JUMP;

	Mars_InitSoundDMA();
	sound_load(&JUMP, "jump");

	MDPSG_init();
	initMainBG();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		drawMainBG();
		loadTextPalette();

		drawTextwHighlight("Sound Test", 119, 35, fontColorGreen, fontColorGreenHighlight);

		drawTextwHighlight("32X PCM", 129, 66, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Left", 85, 80, ycurse == 1 && xcurse == 1 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Center", 133, 80, ycurse == 1 && xcurse == 2 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Right", 195, 80, ycurse == 1 && xcurse == 3 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawTextwHighlight("Genesis PSG Channel", 83, 110, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("0", 139, 124, ycurse == 2 && xcurse == 1 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("1", 149, 124, ycurse == 2 && xcurse == 2 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("2", 159, 124, ycurse == 2 && xcurse == 3 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("3", 169, 124, ycurse == 2 && xcurse == 4 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			xcurse++;
			if (xcurse > 3 && ycurse == 1)
				xcurse = 1;
			else if (xcurse > 4 && ycurse == 2)
				xcurse = 1;
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			xcurse--;
			if (xcurse < 1 && ycurse == 1)
				xcurse = 3;
			else if (xcurse < 1 && ycurse == 2)
				xcurse = 4;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			ycurse++;
			if (ycurse > 2)
				ycurse = 1;
			else if (ycurse == 2 && xcurse > 4)
			{
				ycurse = 1;
				xcurse = 3;
			}
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			ycurse--;
			if (ycurse < 1)
				ycurse = 2;
			else if (ycurse == 2 && xcurse > 4)
			{
				ycurse = 1;
				xcurse = 3;
			}
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_SOUND);
				initMainBG();
				loadTextPalette();
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_SOUND);
			initMainBG();
			loadTextPalette();
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			sound_free(&JUMP);
			MDPSG_stop();
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (xcurse == 1 && ycurse == 1)
				// Left Channel Only
				sound_play(&JUMP, 1, 1);
			if (xcurse == 2 && ycurse == 1)
				// Center
				sound_play(&JUMP, 1, 3);
			if (xcurse == 3 && ycurse == 1)
				// Right Channel Only
				sound_play(&JUMP, 1, 2);
			if (xcurse == 1 && ycurse == 2)
			{
				HwMdPSGSetChandVol(0, 0);
				HwMdPSGSetFrequency(0, 200);
				if (psgoff == 0)
					psgoff = 30;
			}
			if (xcurse == 2 && ycurse == 2)
			{
				HwMdPSGSetChandVol(1, 0);
				HwMdPSGSetFrequency(1, 2000);
				if (psgoff == 0)
					psgoff = 30;
			}
			if (xcurse == 3 && ycurse == 2)
			{
				HwMdPSGSetChandVol(2, 0);
				HwMdPSGSetFrequency(2, 4095);
				if (psgoff == 0)
					psgoff = 30;
			}
			if (xcurse == 4 && ycurse == 2)
			{
				HwMdPSGSetChandVol(3, 0);
				HwMdPSGSetNoise(PSG_NOISE_TYPE_WHITE, PSG_NOISE_FREQ_TONE3);
				HwMdPSGSetFrequency(3, 500);
				if (psgoff == 0)
					psgoff = 30;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		Hw32xScreenFlip(0);

		if (psgoff)
		{
			psgoff--;
			if (psgoff == 0)
				MDPSG_stop();
		}
	}
	return;
}

void at_audiosync_test()
{
	int done = 0, cycle = 0, x = 160, y = 160;
	s16 acc = 1, status = -1;
	u16 button, pressedButton, oldButton = 0xFFFF;

	marsVDP256Start();

	MDPSG_init();
	HwMdPSGSetFrequency(0, 1000);

	setColor(0, 0, 0, 0);
	setColor(1, 31, 31, 31);
	setColor(2, 0, 0, 0);
	setColor(3, 0, 0, 0);
	setColor(4, 0, 0, 0);
	setColor(5, 0, 0, 0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		if (status == 120)
		{
			HwMdPSGSetChandVol(0, 0);
			HwMdPSGSetFrequency(0, 1000);
			Hw32xSetBGColor(0, 31, 31, 31);
		}

		if (status == 121)
		{
			MDPSG_stop();

			setColor(0, 0, 0, 0);
			setColor(2, 0, 0, 0);
			setColor(3, 0, 0, 0);
			setColor(4, 0, 0, 0);
			setColor(5, 0, 0, 0);

			status = -1;
		}

		clearScreen_Fill8bit();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_AUDIOSYNC);
				at_audiosync_test();
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_AUDIOSYNC);
			at_audiosync_test();
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			cycle = !cycle;
			if (!cycle)
				status = 121;
			else
				y = 160;
		}

		if (cycle == 1 && status == -1)
		{
			status = 0;
			acc = -1;
		}

		if (status > -1)
		{
			status++;
			if (status <= 120)
			{
				y += acc;
				drawSprite(block_2x2_tile, x, y, 8, 8, 0, 0);
			}
		}

		drawSprite(block_32x32_tile1, 0, 48, 32, 32, 0, 0);
		drawSprite(block_32x32_tile1, 288, 48, 32, 32, 0, 0);

		drawSprite(block_32x32_tile2, 32, 48, 32, 32, 0, 0);
		drawSprite(block_32x32_tile2, 256, 48, 32, 32, 0, 0);

		drawSprite(block_32x32_tile3, 64, 48, 32, 32, 0, 0);
		drawSprite(block_32x32_tile3, 224, 48, 32, 32, 0, 0);

		drawSprite(block_32x32_tile4, 96, 48, 32, 32, 0, 0);
		drawSprite(block_32x32_tile4, 192, 48, 32, 32, 0, 0);

		for (int i = 0; i <= 320; i = i + 8)
			drawSprite(block_8x8_tile, i, 160, 8, 8, 0, 0);

		if (status >= 20 && status <= 120)
		{
			switch (status)
			{
				case 20:
					break;

				case 40:
					setColor(2, 31, 31, 31);
					break;

				case 60:
					acc = 1;
					setColor(3, 31, 31, 31);
					break;

				case 80:
					setColor(4, 31, 31, 31);
					break;

				case 100:
					setColor(5, 31, 31, 31);
					break;

				case 120:
					break;
			}
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			MDPSG_stop();
			screenFadeOut(1);
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void ht_controller_test()
{
	u16 done = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 button2, pressedButton2, oldButton2 = 0xFFFF;

	initMainBG();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;
		button2 = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		pressedButton2 = button2 & ~oldButton2;
		oldButton2 = button2;

		if (pressedButton & SEGA_CTRL_START && pressedButton & SEGA_CTRL_LEFT)
		{
			screenFadeOut(1);
			done = !done;
		}

		if (pressedButton2 & SEGA_CTRL_START && pressedButton2 & SEGA_CTRL_LEFT)
		{
			screenFadeOut(1);
			done = !done;
		}

		drawMainBG();
		loadTextPalette();

		drawTextwHighlight("Controller Test", 100, 35, fontColorGreen, fontColorGreenHighlight);

		// Controller 1
		if ((button & SEGA_CTRL_TYPE) != SEGA_CTRL_NONE)
		{
			drawTextwHighlight("Up", 74, 80, pressedButton & SEGA_CTRL_UP ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_UP ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Left", 44, 90, pressedButton & SEGA_CTRL_LEFT ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_LEFT ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Right", 89, 90, pressedButton & SEGA_CTRL_RIGHT ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_RIGHT ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Down", 68, 100, pressedButton & SEGA_CTRL_DOWN ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_DOWN ? fontColorRedHighlight : fontColorWhiteHighlight);

			drawTextwHighlight("Start", 149, 90, pressedButton & SEGA_CTRL_START ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_START ? fontColorRedHighlight : fontColorWhiteHighlight);

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
			{
				drawTextwHighlight("A", 219, 90, pressedButton & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_A ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("B", 239, 90, pressedButton & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_B ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("C", 259, 90, pressedButton & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_C ? fontColorRedHighlight : fontColorWhiteHighlight);
			} else {
				drawTextwHighlight("M", 275, 72, pressedButton & SEGA_CTRL_MODE ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_MODE ? fontColorRedHighlight : fontColorWhiteHighlight);

				drawTextwHighlight("X", 219, 80, pressedButton & SEGA_CTRL_X ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_X ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("Y", 239, 80, pressedButton & SEGA_CTRL_Y ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_Y ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("Z", 259, 80, pressedButton & SEGA_CTRL_Z ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_Z ? fontColorRedHighlight : fontColorWhiteHighlight);

				drawTextwHighlight("A", 219, 100, pressedButton & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_A ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("B", 239, 100, pressedButton & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_B ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("C", 259, 100, pressedButton & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_C ? fontColorRedHighlight : fontColorWhiteHighlight);
			}
		} else {
			drawTextwHighlight("Controller 1 not detected", 60, 90, fontColorRed, fontColorRedHighlight);
		}

		// Controller 2
		if ((button2 & SEGA_CTRL_TYPE) != SEGA_CTRL_NONE)
		{
			drawTextwHighlight("Up", 74, 130, pressedButton2 & SEGA_CTRL_UP ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_UP ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Left", 44, 140, pressedButton2 & SEGA_CTRL_LEFT ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_LEFT ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Right", 89, 140, pressedButton2 & SEGA_CTRL_RIGHT ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_RIGHT ? fontColorRedHighlight : fontColorWhiteHighlight);
			drawTextwHighlight("Down", 68, 150, pressedButton2 & SEGA_CTRL_DOWN ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_DOWN ? fontColorRedHighlight : fontColorWhiteHighlight);

			drawTextwHighlight("Start", 149, 140, pressedButton2 & SEGA_CTRL_START ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_START ? fontColorRedHighlight : fontColorWhiteHighlight);

			if ((button2 & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
			{
				drawTextwHighlight("A", 219, 140, pressedButton2 & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_A ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("B", 239, 140, pressedButton2 & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_B ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("C", 259, 140, pressedButton2 & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_C ? fontColorRedHighlight : fontColorWhiteHighlight);
			} else {
				drawTextwHighlight("M", 275, 122, pressedButton2 & SEGA_CTRL_MODE ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_MODE ? fontColorRedHighlight : fontColorWhiteHighlight);

				drawTextwHighlight("X", 219, 130, pressedButton2 & SEGA_CTRL_X ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_X ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("Y", 239, 130, pressedButton2 & SEGA_CTRL_Y ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_Y ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("Z", 259, 130, pressedButton2 & SEGA_CTRL_Z ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_Z ? fontColorRedHighlight : fontColorWhiteHighlight);

				drawTextwHighlight("A", 219, 150, pressedButton2 & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_A ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("B", 239, 150, pressedButton2 & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_B ? fontColorRedHighlight : fontColorWhiteHighlight);
				drawTextwHighlight("C", 259, 150, pressedButton2 & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_C ? fontColorRedHighlight : fontColorWhiteHighlight);
			}
		} else {
			drawTextwHighlight("Controller 2 not detected", 60, 140, fontColorRed, fontColorRedHighlight);
		}

		drawTextwHighlight("Use START+LEFT to exit", 70, 192, fontColorGreen, fontColorGreenHighlight);

		Hw32xScreenFlip(0);
	}
	return;
}

#define VISIBLE_HORZ	16
#define VISIBLE_VERT	28
#define MAX_LOCATIONS	9

void ht_memory_viewer(u32 address)
{
	u8 done = 0, frameDelay = 0, redraw = 1, docrc = 0, locpos = 1, pos = 0, ascii = 0;
	int crc = 0, locations[MAX_LOCATIONS] = { 0, 0x0004000, 0x0004100, 0x0004200, 0x0004400, 0x2000000, 0x4000000, 0x4020000, 0x6000000 };
	u16 button, pressedButton, oldButton = 0xFFFF;

	// Clear the 32X CRAM
	for (int i = 0; i < 255; i++)
		setColor(i, 0, 0, 0);

	// Set screen priority for the 32X
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	for (pos = 0; pos < MAX_LOCATIONS; pos++)
	{
		if (locations[pos] == address)
		{
			locpos = pos;
			break;
		}
	}

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		if (redraw)
		{
			int i = 0, j = 0;
			u8 *mem = NULL;
			char buffer[10];

			myMemSet(buffer, 0, sizeof(char)*10);
			mem = (u8*)address;

			if (docrc)
				crc = CalculateCRC(address, VISIBLE_HORZ*VISIBLE_VERT);

			intToHex(address, buffer, 8);
			HwMdPuts(buffer, 0x2000, 32, 0);
			intToHex(address+448, buffer, 8);
			HwMdPuts(buffer, 0x2000, 32, 27);

			if (docrc)
			{
				intToHex(crc, buffer, 8);
				HwMdPuts(buffer, 0x4000, 32, 14);
			}

			for (i = 0; i < VISIBLE_VERT; i++)
			{
				for (j = 0; j < VISIBLE_HORZ; j++)
				{
					if (!ascii)
						intToHex(mem[i * VISIBLE_HORZ + j], buffer, 2);
					else
					{
						uint16_t c;
						// Space
						buffer[0] = 32;
						buffer[1] = 0;
						c = mem[i*VISIBLE_HORZ+j];
						// ASCII range
						if (c >= 32 && c <= 126)
							buffer[0] = c;
					}
					HwMdPuts(buffer, 0x0000, j * 2, i);
				}
			}

			Hw32xScreenFlip(0);

			Hw32xDelay(frameDelay);

			redraw = 0;
		}

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_A)
		{
			docrc = !docrc;
			HwMdClearScreen();
			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			locpos ++;
			if (locpos == MAX_LOCATIONS)
				locpos = 0;
			address = locations[locpos];
			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			ascii = !ascii;
			HwMdClearScreen();
			redraw = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				HwMdClearScreen();
				DrawHelp(HELP_MEMVIEW);
				ht_memory_viewer(0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			DrawHelp(HELP_MEMVIEW);
			ht_memory_viewer(0);
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			if (address >= 0x1C0)
				address -= 0x1C0;
			else
				address = 0;

			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			address += 0x1C0;
			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			if (address >= 0x10000)
				address -= 0x10000;
			else
				address = 0;

			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			address += 0x10000;
			redraw = 1;
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void PrintBIOSInfo(u32 address)
{
	u8 *bios = NULL;
	char buffer[50];
	int i = 0, j = 0, data[] = { 16, 16, 17, 15, 16, 48, 14, -2, 16, -4, -4, -4, -4, -2, -2, -4, -4, -12, -40, 16, 0};

	bios = (u8*)(address+0x047E);

	while (data[i] != 0)
	{
		if (data[i] > 0)
		{
			memcpy(buffer, bios, sizeof(u8)*data[i]);
			buffer[data[i]] = '\0';
			HwMdPuts(buffer, 0x0000, 12, 8 + j);
			j++;
		}
		bios += data[i] > 0 ? data[i] : -1*data[i];
		i++;
	}
}

void PrintSBIOSInfo(u32 saddress)
{
	u8 *sbios = NULL;
	char sbuffer[50];
	int i = 0, j = 0, sdata[] = { 16, 16, 17, 15, 16, 48, 14, -2, 16, -4, -4, -4, -4, -2, -2, -4, -4, -12, -40, 16, 0};

	sbios = (u8*)(saddress+0x047E);

	while (sdata[i] != 0)
	{
		if (sdata[i] > 0)
		{
			memcpy(sbuffer, sbios, sizeof(u8)*sdata[i]);
			sbuffer[sdata[i]] = '\0';
			HwMdPuts(sbuffer, 0x0000, 12, 8 + j);
			j++;
		}
		sbios += sdata[i] > 0 ? sdata[i] : -1 * sdata[i];
		i++;
	}
}

void ht_check_32x_bios_crc(u32 address)
{
	u16 done = 0;
	int checksum = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	initMainBG();

	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		drawMainBG();
		loadTextPalette();

		ShowMessageAndData("Sega 32X BIOS Data at", address, 0x0000, 8, 4, 4);
		PrintBIOSInfo(address);

		checksum = CalculateCRC(address, 0x0000800);

		doMBIOSID(checksum, address);

		HwMdPuts("32X Slave SH2 1.0", 0x4000, 12, 22);

		////ShowMessageAndData("32X S BIOS CRC32:", schecksum, 0x4000, 8, 6, 22);
		//ShowMessageAndData("", schecksum, 0x4000, 8, 0, 193);

		////HwMdPuts(schecksum, 0x4000, 11, 21);

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_B)
			done = 1;

		Hw32xScreenFlip(0);
	}
	return;
}

void Set32XRegion(u8 value, u32 startaddress, u32 size)
{
	u8 *ram = NULL;
	u32 address = 0;

	ram = (u8*)startaddress;
	for (address = 0; address < size; address++)
		ram[address] = value;
}

int Read32XRegion(u8 value, u32 startaddress, u32 size)
{
	u8 *ram = NULL;
	u32 address = 0;

	ram = (u8*)startaddress;
	for (address = 0; address < size; address++)
	{
		if (ram[address] != value)
			return address;
	}

	return MEMORY_OK;
}

int Check32XRegion(u8 value, u32 startaddress, u32 size)
{
	Set32XRegion(value, startaddress, size);

	return(Read32XRegion(value, startaddress, size));
}

int Check32XRAM(void *start, u32 size)
{
	u32 *sdram = start;

	while (size--)
	{
		u16 result, value;
		value = *sdram;
		*sdram ^= UCHAR_MAX;
		result = value ^ *sdram;
		*sdram++ = value;
		if (result != UCHAR_MAX)
		{
			HwMdPuts("FAILED", 0x2000, 12, 10);
			return 0;
		}
	}
	HwMdPuts("ALL OK", 0x4000, 12, 10);

	return 1;
}

int Check32XRAMWithValue(char * message, u32 start, u32 end, u8 value, int pos)
{
	int memoryFail = 0;

	HwMdPuts(message, 0x0000, 12, pos);

	memoryFail = Check32XRegion(value, start, end - start);

	if (memoryFail != MEMORY_OK)
	{
		//ShowMessageAndData("FAILED", memoryFail, 6, 0x2000, 12, pos+1);
		HwMdPuts("FAILED", 0x2000, 16, pos+1);
		return 0;
	}

	HwMdPuts("ALL OK", 0x4000, 16, pos+1);
	return 1;
}

void ht_test_32x_sdram()
{
	u16 done = 0, draw = 1, test = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	initMainBG();

	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	MARS_SYS_COMM4 = 0;
	MARS_SYS_COMM6 = 0;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
		oldButton = button;

		drawMainBG();
		loadTextPalette();

		HwMdPuts("32X SDRAM 0x6000000", 0x4000, 10, 4);

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_B)
			done = 1;

		if (draw == 1)
		{
			switch (test)
			{
				case 1:
					Check32XRAMWithValue("Setting to 0x00", 0x06000000, 0x060000FF, 0x00, 10);
					Hw32xSleep(1000);

				case 2:
					Check32XRAMWithValue("Setting to 0xFF", 0x06000000, 0x060000FF, 0xFF, 12);
					Hw32xSleep(1000);

				case 3:
					Check32XRAMWithValue("Setting to 0x55", 0x06000000, 0x060000FF, 0x55, 14);
					Hw32xSleep(500);

				case 4:
					Check32XRAMWithValue("Setting to 0xAA", 0x06000000, 0x060000FF, 0xAA, 16);

				case 5:
					HwMdPuts("PRESS START OR B TO EXIT TEST", 0x4000, 5, 24);
					draw = 0;
					test = 0;
			}
		}

		Hw32xScreenFlip(0);

		test++;
	}
	return;
}
