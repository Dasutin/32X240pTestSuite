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
#include "32x.h"
#include "hw_32x.h"
#include "32x_images.h"
#include "shared_objects.h"
#include "main.h"
#include "patterns.h"
#include "tests.h"
#include "help.h"

extern int fontColorWhite;
extern int fontColorRed;
extern int fontColorGreen;
extern int fontColorGray;
extern int fontColorBlack;

uint32_t  canvas_pitch = 320; // canvas_width + scrollwidth
uint32_t  canvas_yaw = 224; // canvas_height + scrollheight

int main()
{
	int frameDelay = 1;
	int curse = 1;
	//char NTSC;
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	
	marsVDP256Start();

	//Hw32xUpdateLineTable(0, 0, 0);

	SetSH2SR(1);

	while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

    //NTSC = (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) != 0;

	Hw32xScreenFlip(0);

	while (1)
	{
		Hw32xFlipWait();
		
		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Test Patterns",  50, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Video tests", 50, 88, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Audio tests", 50, 96, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Hardware tools", 50, 104, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Help", 50, 146, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Credits", 50, 155, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		
		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		{
			mars_drawTextwShadow("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Genesis 32X", 208, 208, fontColorWhite, fontColorGray);
		}
		else 
		{
			mars_drawTextwShadow("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Mega Drive 32X", 184, 208, fontColorWhite, fontColorGray);
		}
		

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
		 	if(curse > 6)
		 		curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
		 	if(curse < 1)
		 		curse = 6;
		}

		if(pressedButton & SEGA_CTRL_A)
		{
			switch (curse) {
				case 1:
					screenFadeOut(1);
					menu_tp();
				break;
				
				case 2:
					screenFadeOut(1);
					menu_vt();
				break;

				case 3:
					screenFadeOut(1);
					menu_at();
				break;

				case 4:
					screenFadeOut(1);
					menu_ht();
				break;

				case 5:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 6:
					screenFadeOut(1);
					credits();
					DrawMainBGwGillian();
				break;
			}
		}

		//HwMdScreenPrintf(0x4000, 0, 5, "clip: %02d", canvas_pitch);
		Hw32xDelay(frameDelay);

		Hw32xScreenFlip(0);
	}
    return 0;
}

void menu_tp()
{
	int done = 0;
	int frameDelay = 1;
	int curse = 1;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Pluge", 40, 48, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Color Bars", 40, 56, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("EBU Color Bars", 40, 64, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("SMPTE Color Bars", 40, 72, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Referenced Color Bars", 40, 80, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Color Bleed Check", 40, 88, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Monoscope", 40, 96, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Grid", 40, 104, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Gray Ramp", 40, 112, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("White & RGB Screens", 40, 120, curse == 10 ? fontColorRed : fontColorWhite, curse == 10 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("100 IRE", 40, 128, curse == 11 ? fontColorRed : fontColorWhite, curse == 11 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Sharpness", 40, 136, curse == 12 ? fontColorRed : fontColorWhite, curse == 12 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Overscan", 40, 144, curse == 13 ? fontColorRed : fontColorWhite, curse == 13 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Convergence", 40, 152, curse == 14 ? fontColorRed : fontColorWhite, curse == 14 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", 40, 170, curse == 15 ? fontColorRed : fontColorWhite, curse == 15 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", 40, 178, curse == 16 ? fontColorRed : fontColorWhite, curse == 16 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		{
			mars_drawTextwShadow("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Genesis 32X", 208, 208, fontColorWhite, fontColorGray);
		}
		else 
		{
			mars_drawTextwShadow("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Mega Drive 32X", 184, 208, fontColorWhite, fontColorGray);
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if(curse > 16)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
			if(curse < 1)
				curse = 16;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			DrawHelp(HELP_GENERAL);
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			switch (curse) 
			{
				case 1:
					screenFadeOut(1);
					tp_pluge();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;
					
				case 2:
					screenFadeOut(1);
					tp_colorchart();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					tp_colorbars();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 4:
					screenFadeOut(1);
					tp_smpte_color_bars();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 5:
					screenFadeOut(1);
					tp_ref_color_bars();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 6:
					screenFadeOut(1);
					tp_color_bleed_check();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 7:
					screenFadeOut(1);
					tp_monoscope();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 8:
					screenFadeOut(1);
					tp_grid();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 9:
					screenFadeOut(1);
					tp_gray_ramp();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 10:
					screenFadeOut(1);
					tp_white_rgb();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 11:
					screenFadeOut(1);
					tp_100_ire();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 12:
					screenFadeOut(1);
					tp_sharpness();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 13:
					screenFadeOut(1);
					tp_overscan();
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 14:
					screenFadeOut(1);
					tp_convergence();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 15:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 16:
					screenFadeOut(1);
					done = 1;
				break;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
    return;
}

void menu_vt()
{
	int done = 0;
	int frameDelay = 0;
	int curse = 1;
	unsigned short button, pressedButton, oldButton  = 0xFFFF;

	Hw32xScreenFlip(0);
	
	while(!done) 
	{
		Hw32xFlipWait();
		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Drop Shadow Test", 40, 64, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);		
		mars_drawTextwShadow("Striped Sprite Test", 40, 72, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Lag Test", 40, 80, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Timing & Reflex Test", 40, 88, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Scroll Test", 40, 96, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Grid Scroll Test", 40, 104, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Horiz/Vert Stripes", 40, 112, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Checkerboard", 40, 120, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Backlit Zone Test", 40, 128, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", 40, 154, curse == 10 ? fontColorRed : fontColorWhite, curse == 10 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", 40, 162, curse == 11 ? fontColorRed : fontColorWhite, curse == 11 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		{
			mars_drawTextwShadow("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Genesis 32X", 208, 208, fontColorWhite, fontColorGray);
		}
		else 
		{
			mars_drawTextwShadow("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Mega Drive 32X", 184, 208, fontColorWhite, fontColorGray);
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if(curse > 11)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
		 	if(curse < 1)
		 		curse = 11;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			DrawHelp(HELP_GENERAL);
		}

		if(pressedButton & SEGA_CTRL_A)
		{
			switch (curse) 
			{
				case 1:
					screenFadeOut(1);
					vt_drop_shadow_test();
					canvas_pitch = 320;
					canvas_yaw = 224;
					marsVDP256Start();
					DrawMainBGwGillian();
				break;
					
				case 2:
					screenFadeOut(1);
					vt_striped_sprite_test();
					canvas_pitch = 320;
					canvas_yaw = 224;
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					vt_lag_test();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 4:
					screenFadeOut(1);
					vt_reflex_test();
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 5:
					screenFadeOut(1);
					vt_scroll_test();
					HwMdClearScreen();
					canvas_pitch = 320;
					canvas_yaw = 224;
					//Hw32xUpdateLineTable(0, 0, 0);
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 6:
					screenFadeOut(1);
					vt_gridscroll_test();
					HwMdClearScreen();
					canvas_pitch = 320;
					canvas_yaw = 224;
					//Hw32xUpdateLineTable(0, 0, 0);
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 7:
					screenFadeOut(1);
					vt_horizontal_stripes();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 8:
					screenFadeOut(1);
					vt_checkerboard();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 9:
					screenFadeOut(1);
					vt_backlitzone_test();
					marsVDP256Start();
					DrawMainBGwGillian();	
				break;

				case 10:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 11:
					screenFadeOut(1);
					done = 1;
				break;
			}

			button = MARS_SYS_COMM8;

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			{
				button = MARS_SYS_COMM10;
			}

			pressedButton = button & ~oldButton;
    		oldButton = button;
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
    return;
}

void menu_at()
{
	int done = 0;
	int frameDelay = 0;
	int curse = 1;
	unsigned short button, pressedButton, oldButton  = 0xFFFF;

	Hw32xScreenFlip(0);
	
	while (!done) 
	{
		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Sound Test", 40, 88, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Audio Sync Test", 40, 96, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", 40, 122, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", 40, 130, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		{
			mars_drawTextwShadow("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Genesis 32X", 208, 208, fontColorWhite, fontColorGray);
		}
		else 
		{
			mars_drawTextwShadow("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Mega Drive 32X", 184, 208, fontColorWhite, fontColorGray);
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
		 	if(curse > 4)
		 		curse = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
		 	if(curse < 1)
		 		curse = 4;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GENERAL);
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			switch (curse) 
			{
				case 1:
					screenFadeOut(1);
					at_sound_test();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 2:
					screenFadeOut(1);
					at_audiosync_test();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 4:
					screenFadeOut(1);
					done = 1;
				break;
			}
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
    return;
}

void menu_ht()
{
	int done = 0;
	int frameDelay = 0;
	int curse = 1;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	Hw32xScreenFlip(0);

	while (!done)
	{

		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Controller Test", 40, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("SDRAM Check", 40, 88, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Memory Viewer", 40, 96, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("BIOS Info", 40, 104, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Help", 40, 128, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", 40, 136, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		{
			mars_drawTextwShadow("NTSC VDP 320x224p", 152, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Genesis 32X", 208, 208, fontColorWhite, fontColorGray);
		}
		else 
		{
			mars_drawTextwShadow("PAL VDP 320x224p", 160, 192, fontColorWhite, fontColorGray);
			mars_drawTextwShadow("Mega Drive 32X", 184, 208, fontColorWhite, fontColorGray);
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;	

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if(curse > 6)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if(curse < 1)
			curse = 6;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			DrawHelp(HELP_GENERAL);
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			switch (curse) 
			{
				case 1:
					screenFadeOut(1);
					ht_controller_test();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;
					
				case 2:
					ht_test_32x_sdram();
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					ht_memory_viewer(0);
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 4:
					screenFadeOut(1);
					ht_check_32x_bios_crc(0);
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;
					
				case 5:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 6:
					screenFadeOut(1);
					done = 1;
				break;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}

void options()
{
}

void credits()
{
	int done = 0;
	int frameDelay = 0;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		DrawMainBG();
		loadTextPalette();

		mars_drawTextwShadow("Credits", 125, 35, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Ver. 0.8", 221, 50, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("7/16/2022", 221, 58, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Code and Port by:", 35, 66, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Dasutin (Dustin Dembrosky)", 43, 74, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Patterns:", 35, 82, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Artemio Urbina", 43, 90, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Menu Pixel Art:", 35, 98, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Asher", 43, 106, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Donna:", 35, 114, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Jose Salot", 43, 122, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("32X Toolchain:", 35, 130, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Chilly Willy (Joseph Fenton)", 43, 138, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Tile Mapper:", 35, 146, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Vic (Victor Luchitz)", 43, 154, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Info on using this test suite:", 35, 162, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("http://junkerhq.net/240p", 43, 170, fontColorWhite, fontColorGray);

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

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
		 	done = 1;
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	return;
}