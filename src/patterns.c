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
#include <string.h>
#include "types.h"
#include "string.h"
#include "32x.h"
#include "hw_32x.h"
#include "32x_images.h"
#include "draw.h"
#include "patterns.h"
#include "shared_objects.h"
#include "help.h"
#include "colorchart_pal.h"
#include "colorbars_palette.h"
#include "colorbars.h"
#include "colorbars_map.h"

#define BLOCK_COLOR_1 32

extern int fontColorWhite, fontColorRed, fontColorGreen, fontColorGray, fontColorBlack;
extern int fontColorWhiteHighlight, fontColorRedHighlight, fontColorGreenHighlight;

void tp_pluge()
{
	int done = 0, draw = 1, IsNTSC = 0, text = 0;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!IsNTSC)
				loadPalette(&PLUGE_RGB_PAL[0], &PLUGE_RGB_PAL[255],0);
			else
				loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);

			draw = 0;
		}

		if (text)
		{
			text--;
			if (!text)
				HwMdClearScreen();
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			IsNTSC = !IsNTSC;

			if (!IsNTSC)
			{
				loadPalette(&PLUGE_RGB_PAL[0], &PLUGE_RGB_PAL[255],0);
				HwMdPuts("RGB FULL RANGE", 0x2000, 24, 2);
			} else {
				loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);
				HwMdPuts("NTSC 7.5 IRE  ", 0x2000, 24, 2);
			}
			text = 30;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				HwMdClearScreen();
				DrawHelp(HELP_PLUGE);
				draw = 1;
			}
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_COLORS);
				Hw32xSetPalette(colorbars_Palette);
			}
		}

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
	int done = 0, draw = 1, Is75 = 0, text = 0;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!Is75)
				loadPalette(&EBU_COLORBARS_100_PAL[0], &EBU_COLORBARS_100_PAL[255],0);
			else
				loadPalette(&EBU_COLORBARS_75_PAL[0], &EBU_COLORBARS_75_PAL[255],0);

			draw = 0;
		}

		if (text)
		{
			text--;
			if (!text)
				HwMdClearScreen();
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

			if (!Is75)
			{
				loadPalette(&EBU_COLORBARS_100_PAL[0], &EBU_COLORBARS_100_PAL[255],0);
				HwMdPuts("100%", 0x0000, 32, 1);
			} else {
				loadPalette(&EBU_COLORBARS_75_PAL[0], &EBU_COLORBARS_75_PAL[255],0);
				HwMdPuts(" 75%", 0x0000, 32, 1);
			}
			text = 30;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				HwMdClearScreen();
				screenFadeOut(1);
				DrawHelp(HELP_601CB);
				draw = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			DrawHelp(HELP_601CB);
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
	int done = 0, draw = 1, Is75 = 0, text = 0;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!Is75)
				loadPalette(&SMPTE100IRE_PAL[0], &SMPTE100IRE_PAL[255],0);
			else
				loadPalette(&SMPTE75IRE_PAL[0], &SMPTE75IRE_PAL[255],0);

			draw = 0;
		}

		if (text)
		{
			text--;
			if (!text)
				HwMdClearScreen();
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

			if (!Is75)
			{
				loadPalette(&SMPTE100IRE_PAL[0], &SMPTE100IRE_PAL[255],0);
				HwMdPuts("100%", 0x0000, 32, 1);
			} else {
				loadPalette(&SMPTE75IRE_PAL[0], &SMPTE75IRE_PAL[255],0);
				HwMdPuts(" 75%", 0x0000, 32, 1);
			}
			text = 30;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				HwMdClearScreen();
				screenFadeOut(1);
				DrawHelp(HELP_SMPTE);
				draw = 1;
			}
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_601CB);
				loadPalette(&COLORREF_PAL[0], &COLORREF_PAL[255],0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_601CB);
			loadPalette(&COLORREF_PAL[0], &COLORREF_PAL[255],0);
		}

		drawBG(COLORREF_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(1);
	}
	return;
}

void tp_color_bleed_check()
{
	int done = 0, pattern = 1;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if (pattern > 2)
				pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_BLEED);
				loadPalette(&COLORBLEED_PAL[0], &COLORBLEED_PAL[255],0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_BLEED);
			loadPalette(&COLORBLEED_PAL[0], &COLORBLEED_PAL[255],0);
		}

		switch (pattern)
		{
			case 1:
				drawBG(COLORBLEED_TILE);
			break;
			case 2:
				drawBG(COLORBLEED_CHECKERBOARD_TILE);
			break;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(1);
	}
	return;
}

void tp_grid()
{
	int done = 0, pattern = 1;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		switch (pattern)
		{
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

			if (pattern > 2)
				pattern = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_GRID);
				loadPalette(&GRID_PAL[0], &GRID_PAL[255],0);
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

		Hw32xDelay(5);
	}
	return;
}

void tp_monoscope()
{
	int done = 0, pattern = 1, gray = 0;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!gray)
			cram16[1] = COLOR(0, 0, 0);
		else
			cram16[1] = COLOR(13, 13, 13);

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
			if (pattern > 7)
				pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
			gray = !gray;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_MONOSCOPE);
				cram16[0] = COLOR(30, 30, 30);
				cram16[1] = COLOR(0, 0, 0);
				cram16[2] = COLOR(31, 0, 0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_MONOSCOPE);
			cram16[0] = COLOR(30, 30, 30);
			cram16[1] = COLOR(0, 0, 0);
			cram16[2] = COLOR(31, 0, 0);
		}

		drawBG(MONOSCOPE_TILE);

		switch (pattern)
		{
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

		Hw32xDelay(5);
	}
	return;
}

void tp_gray_ramp()
{
	int done = 0;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_GRAY);
				loadPalette(&GRAYRAMP_PAL[0], &GRAYRAMP_PAL[255],0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRAY);
			loadPalette(&GRAYRAMP_PAL[0], &GRAYRAMP_PAL[255],0);
		}

		drawBG(GRAYRAMP_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(1);
	}
	return;
}

void tp_white_rgb()
{
	int done = 0, color = 1, sel = 1, l = 320*224, custom = 0;
	u16 r = 31, g = 31, b = 31;
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
			button = MARS_SYS_COMM10;

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
			custom = !custom;

		if (pressedButton & SEGA_CTRL_A)
			color++;

		if (pressedButton & SEGA_CTRL_B)
			color--;

		cram16[0] = COLOR(r, g, b);

		switch (color)
		{
			case 1:
				for (int i = 0; i <= l; i++)
					frameBuffer16[i] = 0x0000;
			break;
			case 2:
				for (int i = 0; i <= l; i++)
					frameBuffer16[i] = 0x0101;
			break;
			case 3:
				for (int i = 0; i <= l; i++)
					frameBuffer16[i] = 0x0202;
			break;
			case 4:
				for (int i = 0; i <= l; i++)
					frameBuffer16[i] = 0x0303;
			break;
			case 5:
				for (int i = 0; i <= l; i++)
					frameBuffer16[i] = 0x0404;
			break;
		}

		if (custom && color == 1)
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

		if (custom)
		{
			if (pressedButton & SEGA_CTRL_LEFT)
				sel--;

			if (pressedButton & SEGA_CTRL_RIGHT)
				sel++;

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
			if (sel < 1)
				sel = 3;
			if (sel > 3)
				sel = 1;
		}

		drawLineTable(4);

		if (color > 5)
			color = 1;

		if (color < 1)
			color = 5;

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_100_ire()
{
	int done = 0, text = 0, draw = 1;
	u16 irevals[] = {13,25,41,53,66,82,94};
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

		if (draw)
		{
			cram16[1] = COLOR(30, 30, 30);
			drawBG(IRE_TILE);
			draw = 0;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			if (ire != 0)
				ire--;
			HwMdScreenPrintf(0x0000, 32, 25, "IRE:%2d", irevals[ire]);
			text = 60;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if (ire != 6)
				ire++;
			HwMdScreenPrintf(0x0000, 32, 25, "IRE:%2d", irevals[ire]);
			text = 60;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				HwMdPuts("       ", 0x0000, 32, 25);
				DrawHelp(HELP_IRE);
				MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
				cram16[0] = COLOR(0, 0, 0);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdPuts("       ", 0x0000, 32, 25);
			DrawHelp(HELP_IRE);
			MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;
			cram16[0] = COLOR(0, 0, 0);
		}

		drawBG(IRE_TILE);

		switch (ire)
		{
			case 0:
				cram16[1] = COLOR(4, 4, 4);		// 13
			break;
			case 1:
				cram16[1] = COLOR(8, 8, 8);		// 25
			break;
			case 2:
				cram16[1] = COLOR(13, 13, 13);	// 41
			break;
			case 3:
				cram16[1] = COLOR(17, 17, 17);	// 53
			break;
			case 4:
				cram16[1] = COLOR(21, 21, 21);	// 66
			break;
			case 5:
				cram16[1] = COLOR(26, 26, 26);	// 82
			break;
			case 6:
				cram16[1] = COLOR(30, 30, 30);	// 94
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			HwMdClearScreen();
			screenFadeOut(1);
			done = 1;
		}

		if (text)
		{
			text--;
			if (!text)
				HwMdClearScreen();
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_sharpness()
{
	int done = 0, pattern = 1;
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
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if (pattern > 2)
				pattern = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_SHARPNESS);

				switch (pattern)
				{
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
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_SHARPNESS);

			switch (pattern)
			{
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

		switch (pattern)
		{
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
	int left = 0, right = 320, top = 0, bottom = 223, done = 0;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	int sel = 0;
	int displayBottom, displayRight;

	Hw32xSetBGColor(0,31,31,31);
	Hw32xSetFGColor(1,15,15,15);

	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();
		clearScreen_Fill16bit();

		char datat[10];
		char datal[10];
		char datab[10];
		char datar[10];
		int l, r, t, b;

		l = left;
		r = right;
		t = top;
		b = bottom;

		for (int i= t; i <= b; i++)
			drawLine(l,i,r,1);

		// Text
		intToStr(top, datat, 1);
		mars_drawTextwShadow("Top:", 96, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow("   pixels", 160, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow(datat, 160, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(abs(bottom-223), datab, 1);
		mars_drawTextwShadow("Bottom:", 96, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow("   pixels", 160, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow(datab, 160, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(left, datal, 1);
		mars_drawTextwShadow("Left:", 96, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow("   pixels", 160, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow(datal, 160, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(abs(right-320), datar, 1);
		mars_drawTextwShadow("Right:", 96, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow("   pixels", 160, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		mars_drawTextwShadow(datar, 160, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_OVERSCAN);
				Hw32xSetBGColor(0,31,31,31);
				Hw32xSetFGColor(1,15,15,15);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_OVERSCAN);
			Hw32xSetBGColor(0,31,31,31);
			Hw32xSetFGColor(1,15,15,15);
		}

		if (pressedButton & SEGA_CTRL_START)
			done = 1;

		if (pressedButton & SEGA_CTRL_UP)
			sel--;

		if (pressedButton & SEGA_CTRL_DOWN)
			sel++;

		if (sel < 0)
			sel = 3;

		if (sel > 3)
			sel = 0;

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			int *datat = NULL;
			int *datal = NULL;
			int *datab = NULL;
			int *datar = NULL;

			switch (sel)
			{
				case 0:
					datat = &top;
					(*datat)--;
					if (*datat < 0)
						*datat = 0;
				break;
				case 1:
					datab = &bottom;
					(*datab)++;
					if (*datab > 223)
						*datab = 223;
				break;
				case 2:
					datal = &left;
					(*datal)--; right++;
					if (*datal < 0)
						*datal = 0;
				break;
				case 3:
					datar = &right;
					(*datar)++;
					if (*datar > 320)
						*datar = 320;
				break;
			}
		}

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			int *datat = NULL;
			int *datal = NULL;
			int *datab = NULL;
			int *datar = NULL;

			switch (sel)
			{
				case 0:
					datat = &top;
					(*datat)++;
					if (*datat < 0)
						*datat = 0;
				break;
				case 1:
					datab = &bottom;
					(*datab)--;
					if (*datab < 124)
						*datab = 124;
				break;
				case 2:
					datal = &left;
					(*datal)++; right--;
					if (*datal < 0)
						*datal = 0;
				break;
				case 3:
					datar = &right;
					(*datar)--;
					if (*datar < 221)
						*datar = 221;
				break;
			}
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			left = top = 0;
			right = 320;
			bottom = 223;
		}

		Hw32xScreenFlip(0);
	}
}

void tp_convergence()
{
	int done = 0, pattern = 1;
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

	for (int i = 0; i < 255; i++)
		cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
			if (pattern > 3)
				pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if (pattern < 3)
				pattern = 4;

			pattern++;
			if (pattern > 5)
				pattern = 4;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_CONVERGENCE);

				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CONVERGENCE);

			for (int i = 0; i < 255; i++)
				cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
		}

		switch (pattern)
		{
			case 1:
				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_GRID_TILE);
			break;
			case 2:
				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_DOTS_TILE);
			break;
			case 3:
				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_GRID_PAL[i] & 0x7FFF;
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_STARS_TILE);
			break;
			case 4:
				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_COLOR_PAL[i] & 0x7FFF;
				drawBG(CONVERGENCE_COLOR_TILE);
			break;
			case 5:
				for (int i = 0; i < 255; i++)
					cram16[i] = CONVERGENCE_COLOR_BORDER_PAL[i] & 0x7FFF;
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
