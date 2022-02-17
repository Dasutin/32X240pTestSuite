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
#include "patterns.h"
#include "shared_objects.h"
#include "help.h"

void tp_pluge()
{
	u16 done = 0;
	int frameDelay = 1;
	int pattern = 1;
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	extern const u16 PLUGE_NTSC_PAL[];
	extern const u16 PLUGE_RGB_PAL[];
	extern const u8 PLUGE_TILE[] __attribute__((aligned(16)));

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

    	 switch (pattern) {
			case 1:
				loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);
			break;
			case 2:
				loadPalette(&PLUGE_RGB_PAL[0], &PLUGE_RGB_PAL[255],0);
			break;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if(pattern > 2){
		 		pattern = 1;
			}
		}

    	if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_PLUGE);
			loadPalette(&PLUGE_NTSC_PAL[0], &PLUGE_NTSC_PAL[255],0);
		}

		drawBG(PLUGE_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_colorchart()
{
	u16 done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORCHART_PAL[];
	extern const u8 COLORCHART_TILE[] __attribute__((aligned(16)));

	loadPalette(&COLORCHART_PAL[0], &COLORCHART_PAL[255],0);

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
			DrawHelp(HELP_COLORS);
			loadPalette(&COLORCHART_PAL[0], &COLORCHART_PAL[255],0);
		}

		drawBG(COLORCHART_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_colorbars()
{
	u16 done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 EBU_COLORBARS_PAL[];
	extern const u8 EBU_COLORBARS_TILE[] __attribute__((aligned(16)));

	loadPalette(&EBU_COLORBARS_PAL[0], &EBU_COLORBARS_PAL[255],0);

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
			DrawHelp(HELP_COLORS);
			loadPalette(&EBU_COLORBARS_PAL[0], &EBU_COLORBARS_PAL[255],0);
		}

		drawBG(EBU_COLORBARS_TILE);

		drawLineTable(4);

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_smpte_color_bars()
{
	u16 done = 0;
	u16 frameDelay = 1, draw = 1;
	u16 Is75 = 0, text = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 SMPTE75IRE_PAL[];
	extern const u16 SMPTE100IRE_PAL[];
	extern const u8 SMPTE100IRE_TILE[] __attribute__((aligned(16)));

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

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_ref_color_bars()
{
	u16 done = 0;
	int frameDelay = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORREF_PAL[];
	extern const u8 COLORREF_TILE[] __attribute__((aligned(16)));

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
	u16 done = 0;
	int frameDelay = 0;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORBLEED_PAL[];
	extern const u8 COLORBLEED_TILE[] __attribute__((aligned(16)));
	extern const u8 COLORBLEED_CHECKERBOARD_TILE[] __attribute__((aligned(16)));

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
	u16 done = 0;
	int frameDelay = 4;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 GRID_PAL[];
	extern const u16 GRID_GRAY_PAL[];
	extern const u8 GRID_TILE[] __attribute__((aligned(16)));

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
	u16 done = 0;
	int frameDelay = 5;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u8 MONOSCOPE_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;

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

    	if (pressedButton & SEGA_CTRL_A)
		{
			pattern--;
			if(pattern < 1)
				pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			pattern++;
			if(pattern > 7)
				pattern = 7;
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
	u16 done = 0;
	int frameDelay = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 GRAYRAMP_PAL[];
	extern const u8 GRAYRAMP_TILE[] __attribute__((aligned(16)));

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
	u8 custom = 0;
	char str[20], num[4];
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	vu16 *cram16 = &MARS_CRAM;

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
			DrawHelp(HELP_GENERAL);
			draw = 1;
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
	char str[10];
	u16 done = 0;
	int frameDelay = 1;
	int pattern = 1;
	u16 text = 0;
	//u16 irevals[] = { 13, 25, 41, 53, 66, 82, 94 };
	u16 irevals[] = { 94, 82, 66, 53, 41, 25, 13 };
	u16 draw = 1;
	u16 ire = 6;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u8 IRE_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;

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

			intToStr(irevals[ire], str, 2);
			HwMdPuts(str, 0x0000, 32, 26);
			HwMdPuts("IRE", 0x0000, 35, 26);

			text = 60;
			//pattern--;
			//if(pattern < 1)
			//	pattern = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if(ire != 6)
				ire++;

			intToStr(irevals[ire], str, 2);
			HwMdPuts(str, 0x0000, 32, 26);
			HwMdPuts("IRE", 0x0000, 35, 26);

			text = 60;
			//pattern++;
			//if(pattern > 7)
			//	pattern = 7;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			DrawHelp(HELP_IRE);
			cram16[0] = COLOR(0, 0, 0);
		}

		drawBG(IRE_TILE);

    	switch (ire) {
			case 1:
				cram16[1] = COLOR(30, 30, 30);
			break;
				
			case 2:
				cram16[1] = COLOR(26, 26, 26);
			break;

			case 3:
				cram16[1] = COLOR(21, 21, 21);
			break;
				
			case 4:
				cram16[1] = COLOR(17, 17, 17);
			break;

			case 5:
				cram16[1] = COLOR(13, 13, 13);
			break;
				
			case 6:
				cram16[1] = COLOR(8, 8, 8);
			break;

			case 7:
				cram16[1] = COLOR(4, 4, 4);    // 13
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

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_sharpness()
{
	u16 done = 0;
	int frameDelay = 5;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 SHARPNESS_PAL[];
	extern const u16 SHARPNESS_BRICK_PAL[];
	extern const u8 SHARPNESS_TILE[] __attribute__((aligned(16)));
	extern const u8 SHARPNESS_BRICK_TILE[] __attribute__((aligned(16)));

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

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_convergence()
{
	u16 done = 0;
	int frameDelay = 5;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 CONVERGENCE_GRID_PAL[];
	extern const u16 CONVERGENCE_COLOR_PAL[];
	extern const u16 CONVERGENCE_COLOR_BORDER_PAL[];
	extern const u8 CONVERGENCE_GRID_TILE[] __attribute__((aligned(16)));
	extern const u8 CONVERGENCE_STARS_TILE[] __attribute__((aligned(16)));
	extern const u8 CONVERGENCE_DOTS_TILE[] __attribute__((aligned(16)));
	extern const u8 CONVERGENCE_COLOR_TILE[] __attribute__((aligned(16)));
	extern const u8 CONVERGENCE_COLOR_BORDER_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;

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

		Hw32xDelay(frameDelay);
	}
	return;
}
