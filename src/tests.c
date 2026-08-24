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
#include "perf.h"
#include "segacd.h"

#define drawTextwHighlight drawMenuTextwHighlight
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
#include "mdtest_palette.h"
#include "mdtest.h"
#include "mdtest_map.h"
#include "phase_check.h"
#include "phase_check_map.h"
#include "phase_check_palette.h"

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

static int video_refresh_rate(void)
{
	return Hw32xDetectPAL() ? 50 : 60;
}

static void wait_for_video_tick(u32 *lastTick)
{
	u32 tick;

	do
		tick = Hw32xGetTicks();
	while (tick == *lastTick);
	*lastTick = tick;
}

static int scale_ntsc_frames(int frames)
{
	return (frames * video_refresh_rate() + 30) / 60;
}

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

static void drop_shadow_clear_message(void)
{
	HwMdPuts("                     ", 0x4000, 19, 0);
}

static void set_md_text_priority(int visible)
{
	Hw32xSetBGOverlayPriorityBit(visible ? 0 : 1);
	Hw32xSetFGOverlayPriorityBit(visible ? 0 : 1);
	MARS_VDP_DISPMODE = (visible ? MARS_VDP_PRIO_68K : MARS_VDP_PRIO_32X) |
		MARS_224_LINES | MARS_VDP_MODE_256;
}

static void reflex_prepare_md_text(void)
{
	HwMdClearPlanes();
	HwMdSetPlaneScrolls(0, 0, 0, 0);
	HwMdReloadFont();
	set_md_text_priority(1);
}

void vt_drop_shadow_test()
{
	int done = 0, frameCount = 0, mode = DRAWSPR_OVERWRITE, x = 30, y = 30;
	int otherTests = 1, changeSprite = 0, background = 1, initTilemap = 1;
	int shadowOnOddFrames = 0, messageFrames = 0;
	u16 frame = 1;
	unsigned short button = 0, pressedButton = 0, oldButton = 0xFFFF;

	Hw32xSetPalette(donna_palette);
	drop_shadow_clear_message();
	set_md_text_priority(0);

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

		if (messageFrames)
		{
			messageFrames--;
			if (!messageFrames)
			{
				drop_shadow_clear_message();
				set_md_text_priority(0);
			}
		}

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
			shadowOnOddFrames = !shadowOnOddFrames;
			messageFrames = (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) ? 180 : 150;
			set_md_text_priority(1);
			if (shadowOnOddFrames)
				HwMdPuts("Shadow on odd frames ", 0x4000, 19, 0);
			else
				HwMdPuts("Shadow on even frames", 0x4000, 19, 0);
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
			messageFrames = 0;
			drop_shadow_clear_message();
			set_md_text_priority(0);
			canvas_pitch = 320;
			DrawHelp(HELP_SHADOW);
			if (!otherTests)
				canvas_pitch = 384;
			Hw32xSetPalette(donna_palette);
			set_md_text_priority(0);
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
					init_tilemap(&tm, &donna_tmx, (const uint8_t * const *)donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 2:
				if (initTilemap == 1)
				{
					otherTests = 1;
					canvas_pitch = 320;
					Hw32xSetPalette(donna_palette);
					init_tilemap(&tm, &checkerboard_donna_tmx, (const uint8_t * const *)checkerboard_donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 3:
				if (initTilemap == 1)
				{
					otherTests = 1;
					init_tilemap(&tm, &h_stripes_tmx, (const uint8_t * const *)h_stripes_reslist);
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
					init_tilemap(&tm, &sonic_tilemap_Map, (const uint8_t * const *)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		if (changeSprite == 0)
		{
			if ((frameCount & 1) == shadowOnOddFrames)
				draw_sprite(x, y, 32, 32, buzz_shadow_sprite, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | mode, 1);

			draw_sprite(x-20, y-20, 32, 32, buzz_sprite, DRAWSPR_OVERWRITE | mode | DRAWSPR_PRECISE, 1);
		} else {
			if ((frameCount & 1) == shadowOnOddFrames)
				draw_sprite(x, y, 32, 32, marker_shadow_tile, DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | mode, 1);
		}

		frameCount++;

		Hw32xDelay(1);
		Hw32xScreenFlip(0);
	}
	drop_shadow_clear_message();
	set_md_text_priority(0);
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
					init_tilemap(&tm, &donna_tmx, (const uint8_t * const *)donna_reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;

			case 2:
				if (initTilemap == 1)
				{
					otherTests = 1;
					canvas_pitch = 320;
					Hw32xSetPalette(donna_palette);
					init_tilemap(&tm, &checkerboard_donna_tmx, (const uint8_t * const *)checkerboard_donna_reslist);
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
					init_tilemap(&tm, &h_stripes_tmx, (const uint8_t * const *)h_stripes_reslist);
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
					init_tilemap(&tm, &sonic_tilemap_Map, (const uint8_t * const *)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
				break;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
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
	const uint8_t *numbers[10] = {tiles_0, tiles_1, tiles_2, tiles_3, tiles_4, tiles_5, tiles_6, tiles_7, tiles_8, tiles_9};
	u16 pause = 0, cposx = 32;
	//u16 cposy = 17;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u32 lastUpdateTick;
	volatile unsigned short *cram16 = &MARS_CRAM;

	canvas_pitch = 320;
	canvas_yaw = 224;

	Hw32xSetPalette(lagtest_res_Palette);
	loadTextPalette();

	MARS_SYS_COMM4 = 0;

	fpcamera_x = fpcamera_y = 0;

	init_tilemap(&tm, &lagtest_Map, (const uint8_t * const *)lagtest_res_Reslist);

	lastUpdateTick = Hw32xGetTicks();
	Hw32xScreenFlip(0);

	while (!done)
	{
		wait_for_video_tick(&lastUpdateTick);

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
				init_tilemap(&tm, &lagtest_Map, (const uint8_t * const *)lagtest_res_Reslist);
				canvas_rebuild_id++;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_LAG);
			Hw32xSetPalette(background_fill_palette1);
			init_tilemap(&tm, &lagtest_Map, (const uint8_t * const *)lagtest_res_Reslist);
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

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		if (!pause)
		{
			frames ++;
			framecnt ++;
			if (framecnt > 8)
				framecnt = 1;
		}

		if (frames >= video_refresh_rate())
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
	u32 lastUpdateTick;

	x = 144, y = 60, x2 = 108, y2 = 96;

	MDPSG_init();
	HwMdPSGSetFrequency(0, 1000);

	Hw32xScreenFlip(0);

	setColor(1, 0, 0, 0);
	setColor(2, 31, 31, 31);

	init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);
	reflex_prepare_md_text();
	lastUpdateTick = Hw32xGetTicks();

	while (!done)
	{
		wait_for_video_tick(&lastUpdateTick);
		Hw32xFlipWait();

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
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

			MDPSG_init();
			HwMdPSGSetFrequency(0, 1000);
			Hw32xScreenFlip(0);
			setColor(1, 0, 0, 0);
			setColor(2, 31, 31, 31);
			init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);
			reflex_prepare_md_text();
			lastUpdateTick = Hw32xGetTicks();
			loadvram = 1;
			draw = 1;

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
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
				if (random16() % 2)
					vary = random16() % 7;
				else
					vary = -1 * random16() % 7;
			}
		}

		if (y < 60 + vary)
		{
			speed = 1;
			change = 1;

			if (variation)
			{
				if (random16() % 2)
					vary = random16() % 7;
				else
					vary = -1 * random16() % 7;
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
	perf_set_scene(PERF_SCENE_SCROLL);
	int done = 0, pause = 0, direction = 0, acc = 1;
	u16 frame = 1, vertical = 0, initTilemap = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	// canvas_width + scrollwidth
	canvas_pitch = 384;

	Hw32xSetPalette(sonic_tileset_Palette);

	fpcamera_x = 262145;
	fpcamera_y = 0;

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &sonic_tilemap_Map, (const uint8_t * const *)sonic_tileset_Reslist);
	canvas_rebuild_id++;

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
					init_tilemap(&tm, &sonic_tilemap_Map, (const uint8_t * const *)sonic_tileset_Reslist);
					canvas_rebuild_id++;
					initTilemap = 0;
				}
			draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
			draw_setScissor(0, 0, 320, 224);
		} else {
			if (initTilemap == 1)
			{
				marsVDP256Start();
				Hw32xSetPalette(kiki_tiles_Palette);
				init_tilemap(&tm, &kiki_Map, (const uint8_t * const *)kiki_tiles_Reslist);
				canvas_rebuild_id++;
				initTilemap = 0;
			}
			draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
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
	u32 lastUpdateTick;

	// canvas_width + scrollwidth
	canvas_pitch = 384;
	// canvas_height + scrollheight
	canvas_yaw = 256;

	Hw32xSetPalette(grid_palette);

	lastUpdateTick = Hw32xGetTicks();
	Hw32xScreenFlip(0);

	init_tilemap(&tm, &grid_tmx, (const uint8_t * const *)grid_reslist);

	while (!done)
	{
		wait_for_video_tick(&lastUpdateTick);

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
				init_tilemap(&tm, &grid_tmx, (const uint8_t * const *)grid_reslist);
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
			init_tilemap(&tm, &grid_tmx, (const uint8_t * const *)grid_reslist);
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

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

static void pattern_counter_set_visible(int visible)
{
	if (visible)
	{
		set_md_text_priority(1);
		HwMdPuts("Frame:", 0x8000, 2, 25);
	} else {
		HwMdPuts("        ", 0x8000, 2, 25);
		set_md_text_priority(0);
	}
}

static void pattern_counter_prepare(void)
{
	HwMdClearPlanes();
	HwMdSetPlaneScrolls(0, 0, 0, 0);
	HwMdReloadFont();
	set_md_text_priority(0);
}

static void pattern_counter_update(int count)
{
	char cntstr[4];

	intToStr(count, cntstr, 2);
	HwMdPuts(cntstr, 0x8000, 8, 25);
}

static void pattern_counter_draw_background(void)
{
	static u16 black_pixels[4] = { 0xFEFE, 0xFEFE, 0xFEFE, 0xFEFE };
	volatile u16 *cram = &MARS_CRAM;
	vu8 *framebuffer = (vu8 *)&MARS_FRAMEBUFFER;
	int x, y;

	cram[254] = 0;
	for (y = 0; y < 8; y++)
	{
		int offset = 0x200 + ((200 + y) * canvas_pitch) + 16;

		for (x = 0; x < 64; x += 8)
			word_8byte_copy((void *)(framebuffer + offset + x),
				(void *)black_pixels, 1);
	}
}

static void stripes_set_orientation(int vertical)
{
	if (vertical)
		init_tilemap(&tm, &vert_map_Map, (const uint8_t * const *)vert_Reslist);
	else
		init_tilemap(&tm, &horiz_map_Map, (const uint8_t * const *)horiz_Reslist);
	canvas_rebuild_id++;
}

static void pattern_set_color(int index, int r, int g, int b, int counter_visible)
{
	volatile u16 *cram = &MARS_CRAM;

	cram[index] = COLOR(r, g, b) & (counter_visible ? 0x7FFF : 0xFFFF);
}

static void stripes_set_phase(int phase, int counter_visible)
{
	if (phase)
	{
		pattern_set_color(0, 0, 0, 0, counter_visible);
		pattern_set_color(1, 31, 31, 31, counter_visible);
	} else {
		pattern_set_color(0, 31, 31, 31, counter_visible);
		pattern_set_color(1, 0, 0, 0, counter_visible);
	}
}

void vt_stripes()
{
	u16 count = 0, docounter = 0;
	u16 phase = 0, alternate = 0, done = 0, vertical = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	u32 alternate_start = 0;

	canvas_pitch = 320;
	canvas_yaw = 224;
	fpcamera_x = 0;
	fpcamera_y = 0;

	Hw32xSetPalette(horiz_Palette);
	stripes_set_orientation(vertical);
	pattern_counter_prepare();
	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		if (alternate)
			phase = (Hw32xGetTicks() - alternate_start) & 1;

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) &&
			(pressedButton & SEGA_CTRL_C)) || (pressedButton & SEGA_CTRL_Z))
		{
			pattern_counter_set_visible(0);
			DrawHelp(HELP_STRIPES);
			stripes_set_orientation(vertical);
			pattern_counter_set_visible(docounter);

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			alternate = !alternate;
			if (alternate)
			{
				alternate_start = Hw32xGetTicks();
				phase = 0;
			}
		}

		if (!alternate &&
			(pressedButton & (SEGA_CTRL_UP | SEGA_CTRL_DOWN)))
			phase = !phase;

		if (pressedButton & (SEGA_CTRL_LEFT | SEGA_CTRL_RIGHT))
		{
			vertical = !vertical;
			stripes_set_orientation(vertical);
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			docounter = !docounter;
			pattern_counter_set_visible(docounter);
			if (!docounter)
				canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		stripes_set_phase(phase, docounter);
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		if (docounter)
		{
			pattern_counter_draw_background();
			count++;
			if (count >= ((MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) ? 60 : 50))
				count = 0;
			pattern_counter_update(count);
		}

		Hw32xScreenFlip(0);
	}
	pattern_counter_set_visible(0);
	return;
}

static void checkerboard_init_pattern(void)
{
	init_tilemap(&tm, &check_map_Map, (const uint8_t * const *)check_Reslist);
	canvas_rebuild_id++;
}

static void checkerboard_set_phase(int phase, int counter_visible)
{
	if (phase)
	{
		pattern_set_color(2, 31, 31, 31, counter_visible);
		pattern_set_color(3, 0, 0, 0, counter_visible);
	} else {
		pattern_set_color(2, 0, 0, 0, counter_visible);
		pattern_set_color(3, 31, 31, 31, counter_visible);
	}
}

void vt_checkerboard()
{
	int done = 0, phase = 0, alternate = 0;
	int docounter = 0, count = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	u32 alternate_start = 0;

	canvas_pitch = 320;
	canvas_yaw = 224;
	fpcamera_x = 0;
	fpcamera_y = 0;

	Hw32xSetPalette(check_Palette);
	checkerboard_init_pattern();
	pattern_counter_prepare();
	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		if (alternate)
			phase = (Hw32xGetTicks() - alternate_start) & 1;

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) &&
			(pressedButton & SEGA_CTRL_C)) || (pressedButton & SEGA_CTRL_Z))
		{
			pattern_counter_set_visible(0);
			DrawHelp(HELP_CHECK);
			checkerboard_init_pattern();
			pattern_counter_set_visible(docounter);

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			alternate = !alternate;
			if (alternate)
			{
				alternate_start = Hw32xGetTicks();
				phase = 0;
			}
		}

		if (!alternate &&
			(pressedButton & (SEGA_CTRL_UP | SEGA_CTRL_DOWN)))
			phase = !phase;

		if (pressedButton & SEGA_CTRL_B)
		{
			docounter = !docounter;
			pattern_counter_set_visible(docounter);
			if (!docounter)
				canvas_rebuild_id++;
		}

		checkerboard_set_phase(phase, docounter);
		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		if (docounter)
		{
			pattern_counter_draw_background();
			count++;
			if (count >= ((MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) ? 60 : 50))
				count = 0;
			pattern_counter_update(count);
		}

		Hw32xScreenFlip(0);
	}
	pattern_counter_set_visible(0);
	return;
}

static void phase_init_background(int checkerboard)
{
	Hw32xSetPalette(phase_check_Palette);
	if (checkerboard)
		init_tilemap(&tm, &check_map_Map, (const uint8_t * const *)check_Reslist);
	else
		init_tilemap(&tm, &phase_check_tmx, (const uint8_t * const *)phase_check_reslist);
	canvas_rebuild_id++;
}

void vt_phase_check(void)
{
	perf_set_scene(PERF_SCENE_PHASE_CHECK);
	int done = 0, checkerboard = 0, align = -3;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	canvas_pitch = 320;
	canvas_yaw = 224;
	window_canvas_x = 0;
	window_canvas_y = 0;

	marsVDP256Start();
	phase_init_background(checkerboard);
	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) &&
			(pressedButton & SEGA_CTRL_C)) || (pressedButton & SEGA_CTRL_Z))
		{
			DrawHelp(HELP_PHASE);
			phase_init_background(checkerboard);

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
			align = -3;

		if (pressedButton & SEGA_CTRL_B)
		{
			checkerboard = !checkerboard;
			phase_init_background(checkerboard);
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			align--;
			if (align < -24)
				align = -24;
		}

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			align++;
			if (align > 18)
				align = 18;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		updateGillianBlink();
		for (int figure = 0; figure < 5; figure++)
			drawGillian(24 + figure * 56 + align, 64);

		Hw32xScreenFlip(0);
	}
}

void vt_backlitzone_test()
{
	int done = 0, x = 160, y = 112, block = 2;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(background_fill_palette);

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);

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
				init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_LED);
			Hw32xSetPalette(background_fill_palette);
			init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);
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

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
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
	u16 done = 0, draw = 1;
	u16 frame = 2, frames = 0;
	u8 logo_page_state[2] = {2, 2};
	int draw_page;
	int seconds = 0, minutes = 0, hours = 0, framecnt = 1;
	const uint8_t *numbers[10] = {tiles_0, tiles_1, tiles_2, tiles_3, tiles_4, tiles_5, tiles_6, tiles_7, tiles_8, tiles_9};
	//timecode tc;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(sd_palette_original);

	MARS_SYS_COMM4 = 0;

	fpcamera_x = fpcamera_y = 0;

	Hw32xScreenFlip(0);

	init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_DISAPPEAR);
			Hw32xSetPalette(sd_palette_original);
			init_tilemap(&tm, &background_fill_tmx, (const uint8_t * const *)background_fill_reslist);
			logo_page_state[0] = logo_page_state[1] = 2;
		}

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_A)
			draw = !draw;

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
		draw_page = ((*(volatile u16 *)((uintptr_t)&currentFB |
			0x20000000u)) ^ 1) & 1;
		if (!draw && logo_page_state[draw_page] == 1)
			draw_dirtyrect(&tm, 128, 70, 64, 128);

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0, NULL, NULL);
		draw_setScissor(0, 0, 320, 224);

		frames ++;
		framecnt ++;

		if (frames >= video_refresh_rate())
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

		if (draw && logo_page_state[draw_page] != 1)
		{
			draw_sprite(128, 70, 64, 128, sd_sprite,
				DRAWSPR_OVERWRITE | DRAWSPR_NODIRTY, 1);
		}
		logo_page_state[draw_page] = draw;

		Hw32xScreenFlip(0);
	}
	return;
}

typedef struct {
	uint8_t *destination;
	const uint8_t *map;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
} remap8_context_t;

static uint8_t layers_priority_remap[256] ATTR_CACHE_ALIGNED;
static int layers_priority_remap_ready;

extern void sh2_remap8_rect(const remap8_context_t *context);

static void layers_mask_32x_behind_front_seed(int x, int y)
{
	uint8_t *fb = (uint8_t *)((uint16_t *)&MARS_FRAMEBUFFER + 0x100);
	remap8_context_t context;
	int width = 48;
	int height = 48;
	int i;

	if (!layers_priority_remap_ready) {
		for (i = 0; i < 256; i++)
			layers_priority_remap[i] = i;
		for (i = 1; i <= 10; i++)
			layers_priority_remap[i] = i + 16;
		layers_priority_remap_ready = 1;
	}

	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x + width > canvas_width)
		width = canvas_width - x;
	if (y + height > canvas_height)
		height = canvas_height - y;
	if (width <= 0 || height <= 0)
		return;

	context.destination = fb + y * canvas_pitch + x;
	context.map = layers_priority_remap;
	context.width = width;
	context.height = height;
	context.pitch = canvas_pitch;
	sh2_remap8_rect(&context);
}

static dtilelayer_t layers_manually_scrolled_md_plane = {
	{0, 0}, {65536, 65536}, NULL, NULL, 0
};

static void layers_set_md_planes(int md_layer_state)
{
	static const uint8_t visible_planes[] = {
		0x03, /* Both planes on. */
		0x02, /* Plane A off. */
		0x01, /* Plane A on, plane B off. */
		0x00  /* Both planes off. */
	};
	uint8_t visible = visible_planes[md_layer_state & 3];

	HwMdClearPlanes();
	if (visible & 0x01)
		HwMdSetPlaneBitmap(0, mdtest_map_Map.mdPlaneA.bitmap);
	if (visible & 0x02)
		HwMdSetPlaneBitmap(1, mdtest_map_Map.mdPlaneB.bitmap);
}

static void layers_setup_video(int show_32x, int md_layer_state)
{
	Hw32xSetPalette(mdtest_Palette);
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_68K | MARS_224_LINES |
		(show_32x ? MARS_VDP_MODE_256 : MARS_VDP_MODE_OFF);
	Hw32xScreenFlip(0);

	init_tilemap(&tm, &mdtest_map_Map, (const uint8_t * const *)mdtest_Reslist);
	if (md_layer_state)
		layers_set_md_planes(md_layer_state);
	Hw32xSetBGOverlayPriorityBit(0);
	Hw32xSetFGOverlayPriorityBit(1);
	Hw32xSetPalettePriorityAliases(17, 1, 10, 0);

	tm.mdPlane[0] = &layers_manually_scrolled_md_plane;
	tm.mdPlane[1] = &layers_manually_scrolled_md_plane;
	canvas_rebuild_id++;
}

static void layers_random_bounce(int *position, int *velocity, int limit,
	int *perpendicular_velocity)
{
	int perpendicular_sign;

	if (*position <= 0)
	{
		*position = 0;
		*velocity = 1 + (random16() & 1);
	}
	else if (*position >= limit)
	{
		*position = limit;
		*velocity = -(1 + (random16() & 1));
	}
	else
	{
		return;
	}

	perpendicular_sign = *perpendicular_velocity < 0 ? -1 : 1;
	*perpendicular_velocity = perpendicular_sign * (1 + (random16() & 1));
}

void vt_layers_test(void)
{
	perf_set_scene(PERF_SCENE_LAYERS);
	int done = 0;
	int front_x = 16, front_y = 16;
	int front_dx = 1, front_dy = 1;
	int rear_x = 256, rear_y = 144;
	int rear_dx = -1, rear_dy = -1;
	int show_32x = 1;
	int md_layer_state = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	canvas_pitch = 320;
	canvas_yaw = 224;
	fpcamera_x = fpcamera_y = 0;

	layers_setup_video(show_32x, md_layer_state);
	setRandomSeed((u16)(Hw32xGetTicks() ^ MARS_SYS_COMM12));

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearPlanes();
			screenFadeOut(1);
			done = 1;
			continue;
		}

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) &&
			(pressedButton & SEGA_CTRL_C)) || (pressedButton & SEGA_CTRL_Z))
		{
			DrawHelp(HELP_LAYERS);
			layers_setup_video(show_32x, md_layer_state);
			HwMdSetPlaneScrolls(front_x, -front_y, rear_x, -rear_y);

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			show_32x = !show_32x;
			MARS_VDP_DISPMODE = MARS_VDP_PRIO_68K | MARS_224_LINES |
				(show_32x ? MARS_VDP_MODE_256 : MARS_VDP_MODE_OFF);
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			md_layer_state = (md_layer_state + 1) & 3;
			layers_set_md_planes(md_layer_state);
		}

		front_x += front_dx;
		front_y += front_dy;
		rear_x += rear_dx;
		rear_y += rear_dy;

		layers_random_bounce(&front_x, &front_dx, 320 - 48, &front_dy);
		layers_random_bounce(&front_y, &front_dy, 224 - 48, &front_dx);
		layers_random_bounce(&rear_x, &rear_dx, 320 - 48, &rear_dy);
		layers_random_bounce(&rear_y, &rear_dy, 224 - 48, &rear_dx);

		HwMdSetPlaneScrolls(front_x, -front_y, rear_x, -rear_y);

		draw_tilemap(&tm, 0, 0, 0, NULL, NULL);
		layers_mask_32x_behind_front_seed(front_x, front_y);
		draw_dirtyrect(&tm, front_x, front_y, 48, 48);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}

	HwMdClearPlanes();
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
}

enum SoundTestSelection
{
	SOUND_TEST_PWM = 0,
	SOUND_TEST_FM,
	SOUND_TEST_FM_OCTAVE,
	SOUND_TEST_FM_PAN,
	SOUND_TEST_PSG
};

static void sound_test_draw_option(char *text, int x, int y,
	int selected, int applied)
{
	u8 color = selected ? fontColorRed :
		(applied ? fontColorGreen : fontColorWhite);
	u8 highlight = selected ? fontColorRedHighlight :
		(applied ? fontColorGreenHighlight : fontColorWhiteHighlight);

	drawTextwHighlight(text, x, y, color, highlight);
}

static u16 sound_test_ym_argument(int note, int octave, int pan, int channel)
{
	return SCD_YM_TEST_PLAY | (((channel - 3) & 3) << 12) |
		((pan & 3) << 10) | ((octave & 0x3F) << 4) | (note & 0x0F);
}

void at_sound_test()
{
	static char *notes[] = {
		"1", "2", "3", "4", "5", "6",
		"7", "8", "9", "A", "B", "C"
	};
	int done = 0, selection = 1, type = SOUND_TEST_PWM;
	int octave = 24, fmPan = 1, fmChannel = 3;
	int pwmPlaying = 0, pwmSelection = -1;
	u16 psgoff = 0;
	u16 toneFrames = scale_ntsc_frames(120);
	u16 button, pressedButton, oldButton = 0xFFFF;
	int i;

	Mars_InitSoundDMA();

	MDPSG_init();
	HwMdSegaCDCommand(SCD_OP_YM_TEST, SCD_YM_TEST_INIT);
	initMainBG();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		drawMainBG();

		drawTextwHighlight("Sound Test", 119, 35, fontColorGreen, fontColorGreenHighlight);

		drawTextwHighlight("32X PWM", 132, 56,
			fontColorGreen, fontColorGreenHighlight);
		sound_test_draw_option("Left", 85, 70,
			type == SOUND_TEST_PWM && selection == 0,
			pwmPlaying && pwmSelection == 0);
		sound_test_draw_option("Center", 133, 70,
			type == SOUND_TEST_PWM && selection == 1,
			pwmPlaying && pwmSelection == 1);
		sound_test_draw_option("Right", 195, 70,
			type == SOUND_TEST_PWM && selection == 2,
			pwmPlaying && pwmSelection == 2);

		drawTextwHighlight("Genesis YM 2612 FM", 88, 91,
			fontColorGreen, fontColorGreenHighlight);
		for (i = 0; i < 12; i++)
			sound_test_draw_option(notes[i], 68 + i * 16, 105,
				type == SOUND_TEST_FM && selection == i, 0);
		{
			char octaveText[4];
			int selected = type == SOUND_TEST_FM_OCTAVE;
			intToStr(octave, octaveText, 2);
			sound_test_draw_option("FM Octave:", 104, 119, selected, 0);
			sound_test_draw_option(octaveText, 200, 119, selected, 0);
		}
		sound_test_draw_option("Left", 85, 133,
			type == SOUND_TEST_FM_PAN && selection == 0, fmPan == 0);
		sound_test_draw_option("Center", 133, 133,
			type == SOUND_TEST_FM_PAN && selection == 1, fmPan == 1);
		sound_test_draw_option("Right", 195, 133,
			type == SOUND_TEST_FM_PAN && selection == 2, fmPan == 2);

		drawTextwHighlight("Genesis PSG Channel", 83, 158,
			fontColorGreen, fontColorGreenHighlight);
		for (i = 0; i < 4; i++)
			sound_test_draw_option(notes[i], 139 + i * 10, 172,
				type == SOUND_TEST_PSG && selection == i, 0);

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			if (type == SOUND_TEST_FM_OCTAVE)
			{
				octave += 8;
				if (octave > 56)
					octave = 0;
			}
			else
			{
				int maximum = type == SOUND_TEST_FM ? 11 :
					(type == SOUND_TEST_PSG ? 3 : 2);
				if (++selection > maximum)
					selection = 0;
			}
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			if (type == SOUND_TEST_FM_OCTAVE)
			{
				octave -= 8;
				if (octave < 0)
					octave = 56;
			}
			else
			{
				int maximum = type == SOUND_TEST_FM ? 11 :
					(type == SOUND_TEST_PSG ? 3 : 2);
				if (--selection < 0)
					selection = maximum;
			}
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			if (--type < SOUND_TEST_PWM)
				type = SOUND_TEST_PSG;
			selection = type == SOUND_TEST_FM ? 1 :
				(type == SOUND_TEST_FM_PAN || type == SOUND_TEST_PWM ? 1 : 0);
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			if (++type > SOUND_TEST_PSG)
				type = SOUND_TEST_PWM;
			selection = type == SOUND_TEST_FM ? 1 :
				(type == SOUND_TEST_FM_PAN || type == SOUND_TEST_PWM ? 1 : 0);
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_SOUND);
				initMainBG();
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
	{
		DrawHelp(HELP_SOUND);
		initMainBG();
	}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			sound_test_pwm_stop();
			MDPSG_stop();
			HwMdSegaCDCommand(SCD_OP_YM_TEST, 0);
			Mars_StopSoundMixer();
			done = 1;
		}

		if (!done && (pressedButton & SEGA_CTRL_A))
		{
			if (type == SOUND_TEST_PWM)
			{
				if (pwmPlaying && pwmSelection == selection)
				{
					sound_test_pwm_stop();
					pwmPlaying = 0;
					pwmSelection = -1;
				}
				else
				{
					if (pwmPlaying)
						sound_test_pwm_stop();
					pwmPlaying = sound_test_pwm_start(1000,
						selection == 0 ? 1 : (selection == 2 ? 2 : 3));
					pwmSelection = pwmPlaying ? selection : -1;
				}
			}
			else if (type == SOUND_TEST_FM)
			{
				HwMdSegaCDCommand(SCD_OP_YM_TEST,
					sound_test_ym_argument(selection, octave, fmPan, fmChannel));
				if (++fmChannel > 5)
					fmChannel = 3;
			}
			else if (type == SOUND_TEST_FM_OCTAVE)
			{
				octave += 8;
				if (octave > 56)
					octave = 0;
			}
			else if (type == SOUND_TEST_FM_PAN)
				fmPan = selection;
			else if (type == SOUND_TEST_PSG)
			{
				if (selection == 0)
					HwMdPSGSetFrequency(0, 200);
				else if (selection == 1)
					HwMdPSGSetFrequency(1, 2000);
				else if (selection == 2)
					HwMdPSGSetFrequency(2, 4095);
				else
				{
					HwMdPSGSetNoise(PSG_NOISE_TYPE_WHITE,
						PSG_NOISE_FREQ_TONE3);
					HwMdPSGSetFrequency(3, 500);
				}
				HwMdPSGSetEnvelope(selection, PSG_ENVELOPE_MAX);
				psgoff = toneFrames;
			}
		}

		if (!done && (pressedButton & SEGA_CTRL_B) && type == SOUND_TEST_FM)
		{
			octave += 8;
			if (octave > 56)
				octave = 0;
		}

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

static void audiosync_draw_static(void)
{
	int i;

	draw_sprite(0, 48, 32, 32, block_32x32_tile1,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(288, 48, 32, 32, block_32x32_tile1,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(32, 48, 32, 32, block_32x32_tile2,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(256, 48, 32, 32, block_32x32_tile2,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(64, 48, 32, 32, block_32x32_tile3,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(224, 48, 32, 32, block_32x32_tile3,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(96, 48, 32, 32, block_32x32_tile4,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
	draw_sprite(192, 48, 32, 32, block_32x32_tile4,
		DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);

	for (i = 0; i <= 320; i += 8)
		draw_sprite(i, 160, 8, 8, block_8x8_tile,
			DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
}

static void audiosync_init_pages(void)
{
	int page;

	for (page = 0; page < 2; page++)
	{
		sh2_fill16((uint16_t *)&MARS_FRAMEBUFFER + 0x100,
			(canvas_pitch * canvas_height) >> 1, 0);
		Hw32xUpdateLineTable(4, 0, 0);
		audiosync_draw_static();
		Hw32xScreenFlip(1);
	}
}

void at_audiosync_test()
{
	perf_set_scene(PERF_SCENE_AUDIO_SYNC);
	int done = 0, cycle = 1, x = 160, y = 160;
	int marker_y[2] = {-1, -1};
	int draw_page;
	int refreshRate = video_refresh_rate();
	int sync40 = scale_ntsc_frames(40);
	int sync60 = scale_ntsc_frames(60);
	int sync80 = scale_ntsc_frames(80);
	int sync100 = scale_ntsc_frames(100);
	int syncEnd = scale_ntsc_frames(120);
	s16 status = -1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u32 lastUpdateTick;

	canvas_pitch = 336;
	canvas_yaw = 224;
	window_canvas_x = 8;
	window_canvas_y = 0;
	draw_setScissor(0, 0, 320, 224);
	marsVDP256Start();

	MDPSG_init();
	HwMdPSGSetFrequency(0, 1000);

	Hw32xSetBGColor(0, 0, 0, 0);
	setColor(1, 31, 31, 31);
	setColor(2, 0, 0, 0);
	setColor(3, 0, 0, 0);
	setColor(4, 0, 0, 0);
	setColor(5, 0, 0, 0);

	audiosync_init_pages();

	lastUpdateTick = Hw32xGetTicks();
	Hw32xScreenFlip(0);

	while (!done)
	{
		wait_for_video_tick(&lastUpdateTick);
		Hw32xFlipWait();

		if (status == syncEnd)
		{
			HwMdPSGSetChandVol(0, 0);
			HwMdPSGSetFrequency(0, 1000);
			Hw32xSetBGColor(0, 31, 31, 31);
		}

		if (status == syncEnd + 1)
		{
			MDPSG_stop();

			Hw32xSetBGColor(0, 0, 0, 0);
			setColor(2, 0, 0, 0);
			setColor(3, 0, 0, 0);
			setColor(4, 0, 0, 0);
			setColor(5, 0, 0, 0);

			status = -1;
		}

		draw_page = ((*(volatile u16 *)((uintptr_t)&currentFB |
			0x20000000u)) ^ 1) & 1;
		if (marker_y[draw_page] >= 0)
			fillRect8(x, marker_y[draw_page], 8, 8, 0);
		marker_y[draw_page] = -1;

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) && (pressedButton & SEGA_CTRL_C)) ||
			(pressedButton & SEGA_CTRL_Z))
		{
			MDPSG_stop();
			DrawHelp(HELP_AUDIOSYNC);

			marsVDP256Start();
			MDPSG_init();
			HwMdPSGSetFrequency(0, 1000);

			Hw32xSetBGColor(0, status == syncEnd ? 31 : 0,
				status == syncEnd ? 31 : 0, status == syncEnd ? 31 : 0);
			setColor(1, 31, 31, 31);
			setColor(2, status >= sync40 ? 31 : 0, status >= sync40 ? 31 : 0, status >= sync40 ? 31 : 0);
			setColor(3, status >= sync60 ? 31 : 0, status >= sync60 ? 31 : 0, status >= sync60 ? 31 : 0);
			setColor(4, status >= sync80 ? 31 : 0, status >= sync80 ? 31 : 0, status >= sync80 ? 31 : 0);
			setColor(5, status >= sync100 ? 31 : 0, status >= sync100 ? 31 : 0, status >= sync100 ? 31 : 0);

			if (status == syncEnd)
				HwMdPSGSetChandVol(0, 0);

			audiosync_init_pages();
			marker_y[0] = marker_y[1] = -1;
			lastUpdateTick = Hw32xGetTicks();
			Hw32xScreenFlip(0);

			button = MARS_SYS_COMM8;
			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;
			oldButton = button;
			pressedButton = 0;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			cycle = !cycle;
			if (!cycle)
				status = syncEnd;
			else
				y = 160;
		}

		if (cycle == 1 && status == -1)
		{
			status = 0;
		}

		if (status > -1)
		{
			status++;
			if (status <= syncEnd)
			{
				int nominalStatus = (status * 60 + refreshRate / 2) / refreshRate;
				y = nominalStatus <= 60 ? 160 - nominalStatus : 40 + nominalStatus;
				draw_sprite(x, y, 8, 8, block_2x2_tile,
					DRAWSPR_OVERWRITE | DRAWSPR_PRECISE | DRAWSPR_NODIRTY, 1);
				marker_y[draw_page] = y;
			}
		}

		if (status == sync40)
			setColor(2, 31, 31, 31);
		if (status == sync60)
			setColor(3, 31, 31, 31);
		if (status == sync80)
			setColor(4, 31, 31, 31);
		if (status == sync100)
			setColor(5, 31, 31, 31);

		if (pressedButton & SEGA_CTRL_START)
		{
			MDPSG_stop();
			screenFadeOut(1);
			done = 1;
		}

		Hw32xUpdateLineTable(4, 0, 0);

		Hw32xScreenFlip(0);
	}
	return;
}

static u8 enableControllerSort = 1;
static u8 debugControllers = 0;

static void drawControllerButton(const char *name, int x, int y,
	u16 state, u16 mask)
{
	drawTextwHighlight(name, x, y,
		state & mask ? fontColorRed : fontColorWhite,
		state & mask ? fontColorRedHighlight : fontColorWhiteHighlight);
}

static void drawController(u16 state, u16 type, int x, int y, int id)
{
	char label[5] = { '-', 'C', '1', '-', 0 };
	int buttonRow;

	if (type != MD_CONTROLLER_TYPE_PAD3 && type != MD_CONTROLLER_TYPE_PAD6)
		return;

	label[2] = '1' + id;
	drawTextwHighlight(label, x + 24, y - 8,
		fontColorGreen, fontColorGreenHighlight);
	drawControllerButton("U", x + 8, y, state, SEGA_CTRL_UP);
	drawControllerButton("L", x, y + 8, state, SEGA_CTRL_LEFT);
	drawControllerButton("R", x + 16, y + 8, state, SEGA_CTRL_RIGHT);
	drawControllerButton("D", x + 8, y + 16, state, SEGA_CTRL_DOWN);
	drawControllerButton("S", x + 32, y + 8, state, SEGA_CTRL_START);

	if (type == MD_CONTROLLER_TYPE_PAD6)
	{
		drawControllerButton("X", x + 48, y, state, SEGA_CTRL_X);
		drawControllerButton("Y", x + 56, y, state, SEGA_CTRL_Y);
		drawControllerButton("Z", x + 64, y, state, SEGA_CTRL_Z);
		drawControllerButton("M", x + 72, y - 8, state, SEGA_CTRL_MODE);
		buttonRow = y + 16;
	}
	else
		buttonRow = y + 8;

	drawControllerButton("A", x + 48, buttonRow, state, SEGA_CTRL_A);
	drawControllerButton("B", x + 56, buttonRow, state, SEGA_CTRL_B);
	drawControllerButton("C", x + 64, buttonRow, state, SEGA_CTRL_C);
}

void controller_options_menu(void)
{
	int done = 0;
	int selection = 0;
	u16 oldButton = 0xFFFF;

	initMainBG();
	Hw32xScreenFlip(0);

	while (!done)
	{
		u16 button;
		u16 pressedButton;

		Hw32xFlipWait();
		drawMainBG();
		drawTextwHighlight("Options", 128, 64,
			fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Auto-sort controllers:", 40, 120,
			selection == 0 ? fontColorRed : fontColorWhite,
			selection == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(enableControllerSort ? "ON " : "OFF", 224, 120,
			selection == 0 ? fontColorRed : fontColorWhite,
			selection == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Debug controllers:", 40, 128,
			selection == 1 ? fontColorRed : fontColorWhite,
			selection == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(debugControllers ? "ON " : "OFF", 224, 128,
			selection == 1 ? fontColorRed : fontColorWhite,
			selection == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Back", 40, 152,
			selection == 2 ? fontColorRed : fontColorWhite,
			selection == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;
		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_UP)
			selection = selection == 0 ? 2 : selection - 1;
		if (pressedButton & SEGA_CTRL_DOWN)
			selection = selection == 2 ? 0 : selection + 1;

		if ((pressedButton & (SEGA_CTRL_LEFT | SEGA_CTRL_RIGHT | SEGA_CTRL_A)) &&
			selection < 2)
		{
			if (selection == 0)
				enableControllerSort = !enableControllerSort;
			else
				debugControllers = !debugControllers;
		}

		if ((pressedButton & SEGA_CTRL_A && selection == 2) ||
			(pressedButton & (SEGA_CTRL_B | SEGA_CTRL_START)))
			done = 1;

		if (!done)
			Hw32xScreenFlip(0);
	}

	screenFadeOut(1);
}

static void drawControllerDebugInfo(const u16 *types, const u16 *portTypes,
	const u16 *portSupports)
{
	char value[5];
	int i;

	intToHex(portTypes[0], value, 4);
	drawTextwHighlight(value, 32, 16, fontColorGreen, fontColorGreenHighlight);
	intToHex(portSupports[0], value, 4);
	drawTextwHighlight(value, 32, 24, fontColorGreen, fontColorGreenHighlight);
	intToHex(portTypes[1], value, 4);
	drawTextwHighlight(value, 144, 16, fontColorGreen, fontColorGreenHighlight);
	intToHex(portSupports[1], value, 4);
	drawTextwHighlight(value, 144, 24, fontColorGreen, fontColorGreenHighlight);

	for (i = 0; i < MD_CONTROLLER_COUNT; i++)
	{
		intToHex(types[i], value, 4);
		drawTextwHighlight(value, i * 40, 32,
			fontColorGreen, fontColorGreenHighlight);
	}
}

void ht_controller_test(void)
{
	static const u16 drawposx[MD_CONTROLLER_COUNT] =
		{ 4, 15, 26, 4, 15, 26, 8, 22 };
	static const u16 drawposy[MD_CONTROLLER_COUNT] =
		{ 7, 7, 7, 12, 12, 12, 17, 17 };
	u16 oldTypes[MD_CONTROLLER_COUNT];
	u16 oldPortTypes[2];
	u16 oldPortSupports[2];
	u16 portTypes[2];
	u16 portSupports[2];
	u16 types[MD_CONTROLLER_COUNT];
	u16 states[MD_CONTROLLER_COUNT];
	int done = 0;
	int firstFrame = 1;
	int debugRefresh = 0;
	int i;

	for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		oldTypes[i] = MD_CONTROLLER_TYPE_UNKNOWN;
	oldPortTypes[0] = MD_CONTROLLER_PORT_UNKNOWN;
	oldPortTypes[1] = MD_CONTROLLER_PORT_UNKNOWN;
	oldPortSupports[0] = MD_CONTROLLER_SUPPORT_OFF;
	oldPortSupports[1] = MD_CONTROLLER_SUPPORT_OFF;

	initMainBG();
	HwMdControllerEnable();
	Hw32xScreenFlip(0);

	while (!done)
	{
		int foundControllers = 0;
		int topologyChanged = 0;
		int resetPorts = 0;

		Hw32xFlipWait();

		if (debugRefresh <= 0)
		{
			HwMdControllerReadPortInfo(portTypes, portSupports);
			debugRefresh = 15;
		}
		debugRefresh--;
		HwMdControllerReadSnapshot(states, types, 0, 0);

		for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		{
			if (types[i] != oldTypes[i])
				topologyChanged = 1;
		}
		for (i = 0; i < 2; i++)
		{
			if (portTypes[i] != oldPortTypes[i] ||
				portSupports[i] != oldPortSupports[i])
				topologyChanged = 1;
			oldPortTypes[i] = portTypes[i];
			oldPortSupports[i] = portSupports[i];
		}

		if (!debugControllers && topologyChanged && !firstFrame)
			initMainBG();

		if (debugControllers)
		{
			clearScreen_Fill8bit();
			invalidateMenuText();
		}
		drawMainBG();

		for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		{
			int position;

			if (types[i] != MD_CONTROLLER_TYPE_PAD3 &&
				types[i] != MD_CONTROLLER_TYPE_PAD6)
				continue;

			if (enableControllerSort)
				position = foundControllers++;
			else
				position = i;

			drawController(states[i], types[i],
				drawposx[position] * 8, drawposy[position] * 8, i);

			if ((states[i] & (SEGA_CTRL_START | SEGA_CTRL_LEFT)) ==
				(SEGA_CTRL_START | SEGA_CTRL_LEFT))
				done = 1;
			if ((states[i] & (SEGA_CTRL_START | SEGA_CTRL_RIGHT)) ==
				(SEGA_CTRL_START | SEGA_CTRL_RIGHT))
				resetPorts = 1;
		}

		if (debugControllers)
			drawControllerDebugInfo(types, portTypes, portSupports);
		else
			drawTextwHighlight("CONTROLLER TEST", 96, 32,
				fontColorGreen, fontColorGreenHighlight);

		drawTextwHighlight("Use START+RIGHT to reset ports", 40, 176,
			fontColorWhite, fontColorWhiteHighlight);
		drawTextwHighlight("Use START+LEFT to exit", 72, 192,
			fontColorGreen, fontColorGreenHighlight);

		for (i = 0; i < MD_CONTROLLER_COUNT; i++)
			oldTypes[i] = types[i];
		firstFrame = 0;

		if (resetPorts && !done)
		{
			HwMdControllerReset();
			for (i = 0; i < MD_CONTROLLER_COUNT; i++)
				oldTypes[i] = MD_CONTROLLER_TYPE_UNKNOWN;
		}

		if (!done)
			Hw32xScreenFlip(0);
	}

	HwMdControllerDisable();
	screenFadeOut(1);
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

		if ((button & SEGA_CTRL_TYPE) != SEGA_CTRL_THREE && (pressedButton & SEGA_CTRL_C))
		{
			ascii = !ascii;
			HwMdClearScreen();
			redraw = 1;
		}

		if ((((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE) && (pressedButton & SEGA_CTRL_C)) ||
			(pressedButton & SEGA_CTRL_Z))
		{
			HwMdClearScreen();
			DrawHelp(HELP_MEMVIEW);

			for (int i = 0; i < 255; i++)
				setColor(i, 0, 0, 0);

			MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
			Hw32xScreenFlip(0);
			redraw = 1;
			oldButton = MARS_SYS_COMM8;
			pressedButton = 0;
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

static void Wait32XSdramTestFrames(u16 frames)
{
	while (frames--)
		Hw32xScreenFlip(1);
}

static u32 Check32XSdramPattern(char *message, u16 pattern, int pos, u16 frames)
{
	u32 memoryFail;

	HwMdPuts(message, 0x0000, 12, pos);
	Mars_ParkSecondaryForSdramTest();
	memoryFail = Hw32xTestSdramPattern(pattern);
	Mars_ResumeSecondaryAfterSdramTest();
	if (memoryFail == MEMORY_OK)
		HwMdPuts("ALL OK", 0x4000, 16, pos+1);
	else
		HwMdPuts("FAILED", 0x2000, 16, pos+1);

	if (frames)
		Wait32XSdramTestFrames(frames);

	return memoryFail;
}

void ht_test_32x_sdram()
{
	u16 done = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u32 memoryFail, patternFail;
	int restartSound;

	initMainBG();

	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	drawMainBG();
	HwMdPuts("32X SDRAM", 0x4000, 15, 4);
	HwMdPuts("Test 0x6010000-0x603BFFF", 0x0000, 7, 6);
	Hw32xScreenFlip(1);

	restartSound = sound_isInitialized();
	if (restartSound)
		Mars_StopSoundMixer();

	Mars_ParkSecondaryForSdramTest();
	memoryFail = Hw32xTestSdramAddressLines();
	Mars_ResumeSecondaryAfterSdramTest();

	patternFail = Check32XSdramPattern("Setting to 0x00", 0x0000, 10, 60);
	if (memoryFail == MEMORY_OK && patternFail != MEMORY_OK)
		memoryFail = patternFail;
	patternFail = Check32XSdramPattern("Setting to 0xFF", 0xFFFF, 12, 60);
	if (memoryFail == MEMORY_OK && patternFail != MEMORY_OK)
		memoryFail = patternFail;
	patternFail = Check32XSdramPattern("Setting to 0x55", 0x5555, 14, 30);
	if (memoryFail == MEMORY_OK && patternFail != MEMORY_OK)
		memoryFail = patternFail;
	patternFail = Check32XSdramPattern("Setting to 0xAA", 0xAAAA, 16, 0);
	if (memoryFail == MEMORY_OK && patternFail != MEMORY_OK)
		memoryFail = patternFail;

	if (restartSound)
		Mars_StartSoundMixer();

	if (memoryFail != MEMORY_OK)
	{
		ShowMessageAndData("Address", memoryFail & ~0x20000000u,
			0x2000, 8, 9, 19);
	}
	HwMdPuts("PRESS START OR B TO EXIT TEST", 0x4000, 5, 24);
	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
		oldButton = button;

		drawMainBG();

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_B)
			done = 1;

		Hw32xScreenFlip(0);
	}
	return;
}
