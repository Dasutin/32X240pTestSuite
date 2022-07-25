/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin
 * Copyright (C)2011-2022 Artemio Urbina
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

#include "types.h"
#include "string.h"
#include "32x.h"
#include "hw_32x.h"
#include "32x_images.h"
#include "draw.h"
#include "dtiles.h"
#include "patterns.h"
#include "shared_objects.h"
#include "help.h"
#include "colorchart_pal.h"
#include "colorbars_palette.h"
#include "colorbars.h"
#include "colorbars_map.h"

#define BLOCK_COLOR_1 32

static volatile const char *new_palette;

void tp_pluge()
{
	int done = 0;
	int draw = 1;
	int IsRGB = 0, text = 0;
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	extern const u16 PLUGE_NTSC_PAL[];
	extern const u16 PLUGE_RGB_PAL[];
	extern const uint8_t PLUGE_TILE[] __attribute__((aligned(16)));

	loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if(draw)
		{
			if(!IsRGB)
				loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);
			else
				loadPalette(&PLUGE_RGB_PAL[0], &PLUGE_RGB_PAL[255],0);

			draw = 0;
		}

		if(text)
		{
			text--;
			if(!text)
			{
				HwMdClearScreen();
			}
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			IsRGB = !IsRGB;

			if(!IsRGB)
			{
				loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);
				HwMdPuts("NTSC 7.5 IRE  ", 0x2000, 24, 2);
			}
			else
			{
				loadPalette(&PLUGE_RGB_PAL[0], &PLUGE_RGB_PAL[255],0);
				HwMdPuts("RGB FULL RANGE", 0x2000, 24, 2);
			}
			text = 30;
		}

    	if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			DrawHelp(HELP_PLUGE);
			draw = 1;
		}

		drawBG(PLUGE_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_colorchart()
{
	int done = 0;
	int fpcamera_x, fpcamera_y;
	u16 button, pressedButton, oldButton = 0xFFFF;
	
	SetSH2SR(1);

	while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

	Hw32xSetPalette(colorbars_Palette);

	init_tilemap(&tm, &colorbars_Map, (uint8_t **)colorbars_Reslist);

	fpcamera_x = fpcamera_y = 0;

	Hw32xScreenFlip(0);

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_COLORS);
			Hw32xSetPalette(colorbars_Palette);
		}

		Hw32xFlipWait();

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_colorbars()
{
	int done = 0;
	int draw = 1;
	int Is75 = 0, text = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 EBU_COLORBARS_100_PAL[];
	extern const u16 EBU_COLORBARS_75_PAL[];	
	extern const uint8_t EBU_COLORBARS_TILE[] __attribute__((aligned(16)));

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if(draw)
		{
			if(!Is75)
				loadPalette(&EBU_COLORBARS_100_PAL[0], &EBU_COLORBARS_100_PAL[255],0);
			else
				loadPalette(&EBU_COLORBARS_75_PAL[0], &EBU_COLORBARS_75_PAL[255],0);

			draw = 0;
		}

		if(text)
		{
			text--;
			if(!text)
			{
				HwMdClearScreen();
			}
		}

    	if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			Is75 = !Is75;

			if(!Is75)
			{
				loadPalette(&EBU_COLORBARS_100_PAL[0], &EBU_COLORBARS_100_PAL[255],0);
				HwMdPuts("100%", 0x0000, 32, 1);
			}
			else
			{
				loadPalette(&EBU_COLORBARS_75_PAL[0], &EBU_COLORBARS_75_PAL[255],0);
				HwMdPuts(" 75%", 0x0000, 32, 1);
			}
			text = 30;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			DrawHelp(HELP_COLORS);
			draw = 1;
		}

		drawBG(EBU_COLORBARS_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_smpte_color_bars()
{
	int done = 0;
	int draw = 1;
	int Is75 = 0, text = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 SMPTE75IRE_PAL[];
	extern const u16 SMPTE100IRE_PAL[];
	extern const uint8_t SMPTE100IRE_TILE[] __attribute__((aligned(16)));

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if(draw)
		{
			if(!Is75)
				loadPalette(&SMPTE100IRE_PAL[0], &SMPTE100IRE_PAL[255],0);
			else
				loadPalette(&SMPTE75IRE_PAL[0], &SMPTE75IRE_PAL[255],0);

			draw = 0;
		}

		if(text)
		{
			text--;
			if(!text)
			{
				HwMdClearScreen();
			}
		}

    	if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			Is75 = !Is75;

			if(!Is75)
			{
				loadPalette(&SMPTE100IRE_PAL[0], &SMPTE100IRE_PAL[255],0);
				HwMdPuts("100%", 0x0000, 32, 1);
			}
			else
			{
				loadPalette(&SMPTE75IRE_PAL[0], &SMPTE75IRE_PAL[255],0);
				HwMdPuts(" 75%", 0x0000, 32, 1);
			}
			text = 30;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			DrawHelp(HELP_SMPTE);
			draw = 1;
		}

		drawBG(SMPTE100IRE_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_ref_color_bars()
{
	int done = 0;
	int frameDelay = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORREF_PAL[];
	extern const uint8_t COLORREF_TILE[] __attribute__((aligned(16)));

	loadPalette(&COLORREF_PAL[0], &COLORREF_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_601CB);
			loadPalette(&COLORREF_PAL[0], &COLORREF_PAL[255],0);
		}

		drawBG(COLORREF_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_color_bleed_check()
{
	int done = 0;
	int frameDelay = 0;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORBLEED_PAL[];
	extern const uint8_t COLORBLEED_TILE[] __attribute__((aligned(16)));
	extern const uint8_t COLORBLEED_CHECKERBOARD_TILE[] __attribute__((aligned(16)));

	loadPalette(&COLORBLEED_PAL[0], &COLORBLEED_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if(pattern > 2){
		 		pattern = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_BLEED);
			loadPalette(&COLORBLEED_PAL[0], &COLORBLEED_PAL[255],0);
		}

		switch (pattern) {
				case 1:
					drawBG(COLORBLEED_TILE);
				break;
				
				case 2:
					drawBG(COLORBLEED_CHECKERBOARD_TILE);
				break;
			}

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_grid()
{
	int done = 0;
	int frameDelay = 4;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 GRID_PAL[];
	extern const u16 GRID_GRAY_PAL[];
	extern const uint8_t GRID_TILE[] __attribute__((aligned(16)));

	loadPalette(&GRID_PAL[0], &GRID_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	switch (pattern) {
			case 1:
				loadPalette(&GRID_PAL[0], &GRID_PAL[255],0);
			break;
				
			case 2:
				loadPalette(&GRID_GRAY_PAL[0], &GRID_GRAY_PAL[255],0);
			break;
		}

    	if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if(pattern > 2){
		 		pattern = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRID);
			loadPalette(&GRID_PAL[0], &GRID_PAL[255],0);
		}

		drawBG(GRID_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_monoscope()
{
	int done = 0;
	int frameDelay = 5;
	int pattern = 1;
	int gray = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const uint8_t MONOSCOPE_TILE[] __attribute__((aligned(16)));
	volatile unsigned short *cram16 = &MARS_CRAM;

	cram16[0] = COLOR(30, 30, 30);
	cram16[1] = COLOR(0, 0, 0);
	cram16[2] = COLOR(31, 0, 0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if(!gray) {
			cram16[1] = COLOR(0, 0, 0);
		}
		else {
			cram16[1] = COLOR(13, 13, 13);
		}
		

    	if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
			if(pattern > 7)
				pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			gray = !gray;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_MONOSCOPE);
				cram16[0] = COLOR(30, 30, 30);
				cram16[1] = COLOR(0, 0, 0);
				cram16[2] = COLOR(31, 0, 0);
		}

		drawBG(MONOSCOPE_TILE);

    	switch (pattern) {
			case 1:
				cram16[0] = COLOR(30, 30, 30);
			break;
				
			case 2:
				cram16[0] = COLOR(26, 26, 26);
			break;

			case 3:
				cram16[0] = COLOR(21, 21, 21);
			break;
				
			case 4:
				cram16[0] = COLOR(17, 17, 17);
			break;

			case 5:
				cram16[0] = COLOR(13, 13, 13);
			break;
				
			case 6:
				cram16[0] = COLOR(8, 8, 8);
			break;

			case 7:
				cram16[0] = COLOR(4, 4, 4);
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_gray_ramp()
{
	int done = 0;
	int frameDelay = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 GRAYRAMP_PAL[];
	extern const uint8_t GRAYRAMP_TILE[] __attribute__((aligned(16)));

	loadPalette(&GRAYRAMP_PAL[0], &GRAYRAMP_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRAY);
			loadPalette(&GRAYRAMP_PAL[0], &GRAYRAMP_PAL[255],0);
		}

		drawBG(GRAYRAMP_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_white_rgb()
{
	int done = 0;
	int color = 1;
	int draw = 1;
	int sel = 1;
	int l = 320*224;
	u16 r = 31, g = 31, b = 31;
	int custom = 0;
	char str[20], num[4];
	u16 button, pressedButton, oldButton = 0xFFFF;
	volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;
	volatile unsigned short *cram16 = &MARS_CRAM;

	cram16[0] = COLOR(r, g, b);
	cram16[1] = COLOR(0, 0, 0);
	cram16[2] = COLOR(31, 0, 0);
	cram16[3] = COLOR(0, 31, 0);
	cram16[4] = COLOR(0, 0, 31);

	Hw32xScreenFlip(0);

	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			DrawHelp(HELP_WHITE);
			tp_white_rgb();
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			custom = !custom;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			color++;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			color--;
			draw = 1;
		}

		cram16[0] = COLOR(r, g, b);

		switch (color)
		{
			case 1:
				for (int i=0; i<=l; i++){
					frameBuffer16[i] = 0x0000;
				} 
			break;

			case 2:
				for (int i=0; i<=l; i++){
					frameBuffer16[i] = 0x0101;
				}
			break;

			case 3:
				for (int i=0; i<=l; i++){
					frameBuffer16[i] = 0x0202;
				}
			break;
					
			case 4:
				for (int i=0; i<=l; i++){
					frameBuffer16[i] = 0x0303;
				}
			break;

			case 5:
				for (int i=0; i<=l; i++){
					frameBuffer16[i] = 0x0404;
				}
			break;
		}

		if(custom && color == 1)
		{
			strcpy(str, "R:");
			intToStr(r, num, 2);
			strcat(str, num);

			HwMdPuts(str, sel == 1 ? 0x4000 : 0x2000, 22, 1);

			strcpy(str, "G:");
			intToStr(g, num, 2);
			strcat(str, num);

			HwMdPuts(str, sel == 2 ? 0x4000 : 0x2000, 27, 1);

			strcpy(str, "B:");
			intToStr(b, num, 2);
			strcat(str, num);

			HwMdPuts(str, sel == 3 ? 0x4000 : 0x2000, 32, 1);
		}

		if(custom)
		{
			if(pressedButton & SEGA_CTRL_LEFT)
			{
				sel--;
			}

			if(pressedButton & SEGA_CTRL_RIGHT)
			{
				sel++;
			}

			if (pressedButton & SEGA_CTRL_UP)
			{
				switch (sel)
				{
				case 1:
					r++;
					if (r > 31)
						r = 0;
					break;
				case 2:
					g++;
					if (g > 31)
						g = 0;
					break;
				case 3:
					b++;
					if (b > 31)
						b = 0;
					break;
				}
			}

			if (pressedButton & SEGA_CTRL_DOWN)
			{
				switch (sel)
				{
				case 1:
					r--;
					if (r == 65535)
						r = 31;
					break;
				case 2:
					g--;
					if (g == 65535)
						g = 31;
					break;
				case 3:
					b--;
					if (b== 65535)
						b = 31;
					break;
				}
			}
			if(sel < 1)
				sel = 3;
			if(sel > 3)
				sel = 1;
		}

		drawLineTable(4);

		if(color > 5)
			color = 1;

		if(color < 1)
			color = 5;

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_100_ire()
{
	int done = 0;
	int text = 0;
	u16 irevals[] = {13,25,41,53,66,82,94};
	int draw = 1;
	u16 ire = 6;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const uint8_t IRE_TILE[] __attribute__((aligned(16)));
	volatile unsigned short *cram16 = &MARS_CRAM;

	cram16[0] = COLOR(0, 0, 0);

	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if(draw)
		{
			cram16[1] = COLOR(30, 30, 30);
			drawBG(IRE_TILE);
			draw = 0;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_A)
		{
			if(ire != 0)
				ire--;
			HwMdScreenPrintf(0x0000, 32, 25, "IRE:%2d", irevals[ire]);
			text = 60;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if(ire != 6)
				ire++;
			HwMdScreenPrintf(0x0000, 32, 25, "IRE:%2d", irevals[ire]);
			text = 60;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdPuts("       ", 0x0000, 32, 25);
			DrawHelp(HELP_IRE);
			MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
			cram16[0] = COLOR(0, 0, 0);
		}

		drawBG(IRE_TILE);

    	switch (ire) {
			case 0:
				cram16[1] = COLOR(4, 4, 4);     // 13
			break;
				
			case 1:
				cram16[1] = COLOR(8, 8, 8);     // 25
			break;

			case 2:
				cram16[1] = COLOR(13, 13, 13);  // 41
			break;
				
			case 3:
				cram16[1] = COLOR(17, 17, 17);  // 53
			break;

			case 4:
				cram16[1] = COLOR(21, 21, 21);  // 66
			break;
				
			case 5:
				cram16[1] = COLOR(26, 26, 26);  // 82
			break;

			case 6:
				cram16[1] = COLOR(30, 30, 30);  // 94
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if(text)
		{
			text--;
			if(!text)
			{
				HwMdClearScreen();
			}
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_sharpness()
{
	int done = 0;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 SHARPNESS_PAL[];
	extern const u16 SHARPNESS_BRICK_PAL[];
	extern const uint8_t SHARPNESS_TILE[] __attribute__((aligned(16)));
	extern const uint8_t SHARPNESS_BRICK_TILE[] __attribute__((aligned(16)));

	loadPalette(&SHARPNESS_PAL[0], &SHARPNESS_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if(pattern > 2){
		 		pattern = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_SHARPNESS);

			switch (pattern) {
			case 1:
				loadPalette(&SHARPNESS_PAL[0], &SHARPNESS_PAL[255],0);
				drawBG(SHARPNESS_TILE);
			break;
				
			case 2:
				loadPalette(&SHARPNESS_BRICK_PAL[0], &SHARPNESS_BRICK_PAL[255],0);
				drawBG(SHARPNESS_BRICK_TILE);
			break;
			}
		}

		switch (pattern) {
			case 1:
				loadPalette(&SHARPNESS_PAL[0], &SHARPNESS_PAL[255],0);
				drawBG(SHARPNESS_TILE);
			break;
				
			case 2:
				loadPalette(&SHARPNESS_BRICK_PAL[0], &SHARPNESS_BRICK_PAL[255],0);
				drawBG(SHARPNESS_BRICK_TILE);
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_overscan()
{
	// u32 _tile_l[8], _tile_r[8], _tile_t[8], _tile_b[8];
	// u32 _tile_lb[8], _tile_lt[8], _tile_rt[8], _tile_rb[8];
	// u16 vram = TILE_USERINDEX;
	int type = 0, ntype = 0;
	int left = 0, right = 0, top = 320, bottom = 224, done = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	int redraw = 1;
	int sel = 0, maxTileVert = 0, maxTileHor = 0, loadvram = 1;
	const u32 back[8] = { 0x44444444, 0x44444444, 0x44444444, 0x44444444,
		0x44444444, 0x44444444, 0x44444444, 0x44444444
	};
	const u32 white[8] = { 0x77777777, 0x77777777, 0x77777777, 0x77777777,
		0x77777777, 0x77777777, 0x77777777, 0x77777777
	};
	//vu16 *cram16 = &MARS_CRAM;
	
	//vu8 gray = COLOR(15, 15, 15);

	/* tile_l = _tile_l;
	tile_r = _tile_r;
	tile_t = _tile_t;
	tile_b = _tile_b;
	tile_lb = _tile_lb;
	tile_lt = _tile_lt;
	tile_rt = _tile_rt;
	tile_rb = _tile_rb; */

	Hw32xSetBGColor(0,31,31,31);

	Hw32xSetFGColor(BLOCK_COLOR_1,15,15,15);
	vu8 blockColor[8] = {BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1,BLOCK_COLOR_1};
	
	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while(!done)
	{
		Hw32xFlipWait();

		clearScreen_Fill8bit();
		
		/* if(loadvram)
		{
			VDP_Start();
			if(type == RES_256)
				VDP_setScreenWidth256();
			else
				VDP_setScreenWidth320();
					
			VDP_loadTileData(back, vram, 1, USE_DMA);
			VDP_loadTileData(white, vram + 9, 1, USE_DMA);

			VDP_setPalette(PAL3, palette_grey);
			VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL3, 0, 0, 0) + vram, 0, 0, (type == RES_256 ? 256 : 320) / 8, getVerticalRes() / 8);
			VDP_End();
			loadvram = 0;
			redraw = 1;
		} */

			char data[10];
			int l, r, t, b;

			l = left;
			r = right;
			t = top;
			b = bottom;

			maxTileVert = 224;
			maxTileHor = 320;

			 // Clean center
			//VDP_fillTileMapRect(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram, l + 1, t + 1, maxTileHor - (r + l) - 2, maxTileVert - (t + b) - 2);

			//FillTiles(vram, left, right, top, bottom);

			 // Left
			//drawFillRect(l, t + 1, 1, (maxTileVert - 2) - b - t, (vu8*)&blockColor);

			//drawFillRect(l, t + 1, 1, (maxTileVert - 2) - b - t, (vu8*)&blockColor);

			drawFillRect(l, r, t, b, (vu8*)&blockColor);
			
			// Right
			//drawFillRect((maxTileHor - 1) - r, t + 1, 1, (maxTileVert - 2) - b - t, (vu8*)&blockColor);
			// Top
			//drawFillRect(l + 1, t, (maxTileHor - 2) - r - l, 1, (vu8*)&blockColor);
			// Bottom
			//drawFillRect(l + 1, (maxTileVert - 1) - b, (maxTileHor - 2) - r - l, 1, (vu8*)&blockColor);

			// Corners

			// left top
			//VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 5, l, t);
			// left bottom
			//VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 6, l, (maxTileVert - 1) - b);
			// right top
			//VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 7, (maxTileHor - 1) - r, t);
			// right bottom
			//VDP_setTileMapXY(BPLAN, TILE_ATTR(PAL0, 0, 0, 0) + vram + 8, (maxTileHor - 1) - r, (maxTileVert - 1) - b);

			// Whites
			//if(l)
			//	drawFillRect(0, t, l, maxTileVert - b - t, (vu8*)&blockColor);
			// Right
			//if(r)
			//	drawFillRect(maxTileHor - r, t, r, maxTileVert - b - t, (vu8*)&blockColor);
			// Top
			//if(t)
			//	drawFillRect(0, 0, maxTileHor, t, (vu8*)&blockColor);
			// Bottom
			//if(b)
			//	drawFillRect(0, maxTileVert - b, maxTileHor, b, (vu8*)&blockColor);

			// text
			intToStr(top, data, 1);
			HwMdPuts("Top:", sel == 0 ? 0x2000 : 0x0000, 12, 12);
			HwMdPuts("   pixels", sel == 0 ? 0x2000 : 0x0000, 20, 12);
			HwMdPuts(data, sel == 0 ? 0x2000 : 0x0000, 20, 12);

			intToStr(bottom, data, 1);
			HwMdPuts("Bottom:", sel == 1 ? 0x2000 : 0x0000, 12, 13);
			HwMdPuts("   pixels", sel == 1 ? 0x2000 : 0x0000, 20, 13);
			HwMdPuts(data, sel == 1 ? 0x2000 : 0x0000, 20, 13);

			intToStr(left, data, 1);
			HwMdPuts("Left:", sel == 2 ? 0x2000 : 0x0000, 12, 14);
			HwMdPuts("   pixels", sel == 2 ? 0x2000 : 0x0000, 20, 14);
			HwMdPuts(data, sel == 2 ? 0x2000 : 0x0000, 20, 14);

			intToStr(right, data, 1);
			HwMdPuts("Right:", sel == 3 ? 0x2000 : 0x0000, 12, 15);
			HwMdPuts("   pixels", sel == 3 ? 0x2000 : 0x0000, 20, 15);
			HwMdPuts(data, sel == 3 ? 0x2000 : 0x0000, 20, 15);

		//drawFillRect(20, 80, 40, 40, (vu8*)&blockColor);

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}
		pressedButton = button & ~oldButton;
    	oldButton = button;

		if(pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_OVERSCAN);
		}

		if(pressedButton & SEGA_CTRL_START)
			done = 1;

		if(pressedButton & SEGA_CTRL_UP)
		{
			sel--;
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_DOWN)
		{
			sel++;
			redraw = 1;
		}

		if(sel < 0)
			sel = 3;
		if(sel > 3)
			sel = 0;

		if(pressedButton & SEGA_CTRL_LEFT)
		{
			int *data = NULL;

			switch (sel)
			{
			case 0:
				data = &top;
				break;
			case 1:
				data = &bottom;
				break;
			case 2:
				data = &left;
				break;
			case 3:
				data = &right;
				break;
			}

			if(data)
			{
				(*data)--;
				if(*data < 0)
					*data = 0;
			}
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_RIGHT)
		{
			int *data = NULL;

			switch (sel)
			{
			case 0:
				data = &top;
				break;
			case 1:
				data = &bottom;
				break;
			case 2:
				data = &left;
				break;
			case 3:
				data = &right;
				break;
			}

			if(data)
			{
				(*data)++;
				if(*data > 320)
					*data = 320;
			}
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_A)
		{
			left = right = bottom = top = 0;
			redraw = 1;
		}

		drawLineTable(4);
			
		Hw32xScreenFlip(0);
	}
}

void tp_convergence()
{
	int done = 0;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 CONVERGENCE_GRID_PAL[];
	extern const u16 CONVERGENCE_COLOR_PAL[];
	extern const u16 CONVERGENCE_COLOR_BORDER_PAL[];
	extern const uint8_t CONVERGENCE_GRID_TILE[] __attribute__((aligned(16)));
	extern const uint8_t CONVERGENCE_STARS_TILE[] __attribute__((aligned(16)));
	extern const uint8_t CONVERGENCE_DOTS_TILE[] __attribute__((aligned(16)));
	extern const uint8_t CONVERGENCE_COLOR_TILE[] __attribute__((aligned(16)));
	extern const uint8_t CONVERGENCE_COLOR_BORDER_TILE[] __attribute__((aligned(16)));
	volatile unsigned short *cram16 = &MARS_CRAM;

	for (int i = 0; i < 255; i++){
		cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
	}

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
			if(pattern > 3){
		 		pattern = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if(pattern < 3){
				pattern = 4;
			}

			pattern++;
			if(pattern > 5){
		 		pattern = 4;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CONVERGENCE);

			for (int i = 0; i < 255; i++){
				cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
			}
		}

    	switch (pattern) {
			case 1:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_GRID_TILE);
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_DOTS_TILE);
			break;

			case 3:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_STARS_TILE);
			break;
		
			case 4:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_COLOR_PAL[i] & 0x7FFF;
				}
				drawBG(CONVERGENCE_COLOR_TILE);
			break;
				
			case 5:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_COLOR_BORDER_PAL[i] & 0x7FFF;
				}
				drawBG(CONVERGENCE_COLOR_BORDER_TILE);
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}
