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

volatile unsigned mars_pwdt_ovf_count = 0;
volatile unsigned mars_swdt_ovf_count = 0;

unsigned mars_frtc2msec_frac = 0;

const int NTSC_CLOCK_SPEED = 23011360; // HZ
const int PAL_CLOCK_SPEED = 22801467; // HZ

//Hw32xInitSoundDMA();

snddma_secondary_init();

int main()
{
	int frameDelay = 0;
	int curse = 1;
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	char NTSC;

	marsVDP256Start();

	SetSH2SR(1);

	while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

    NTSC = (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) != 0;

	SH2_WDT_WTCSR_TCNT = 0x5A00; /* WDT TCNT = 0 */
    SH2_WDT_WTCSR_TCNT = 0xA53E; /* WDT TCSR = clr OVF, IT mode, timer on, clksel = Fs/4096 */

	/* init hires timer system */
    SH2_WDT_VCR = (65 << 8) | (SH2_WDT_VCR & 0x00FF); // set exception vector for WDT
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xFF0F) | 0x0020; // set WDT INT to priority 2

	MARS_SYS_COMM4 = 0;
	MARS_SYS_COMM6 = 0;

	mars_frtc2msec_frac = 4096.0f * 1000.0f / (NTSC ? NTSC_CLOCK_SPEED : PAL_CLOCK_SPEED) * 65536.0f;

	Hw32xScreenFlip(0);

	while (1)
	{
		Hw32xFlipWait();
		
		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Test Paterns",  -15, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Video tests", -15, 88, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Audio tests", -15, 96, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Hardware tools", -15, 104, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Help", -15, 146, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Credits", -15, 155, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		
		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		mars_drawTextwShadow("NTSC VDP 320x224p", 90, 193, fontColorWhite, fontColorGray);
		else
		mars_drawTextwShadow("PAL VDP 320x224p", 98, 193, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Sega 32X", 170, 208, fontColorWhite, fontColorGray);

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

		mars_drawTextwShadow("Pluge", -24, 48, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Color Bars", -24, 56, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("EBU Color Bars", -24, 64, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("SMPTE Color Bars", -24, 72, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Referenced Color Bars", -24, 80, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Color Bleed Check", -24, 88, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Grid", -24, 96, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Monoscope", -24, 104, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Gray Ramp", -24, 112, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("White & RGB Screens", -24, 120, curse == 10 ? fontColorRed : fontColorWhite, curse == 10 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("100 IRE", -24, 128, curse == 11 ? fontColorRed : fontColorWhite, curse == 11 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Sharpness", -24, 136, curse == 12 ? fontColorRed : fontColorWhite, curse == 12 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Convergence", -24, 144, curse == 13 ? fontColorRed : fontColorWhite, curse == 13 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", -24, 162, curse == 14 ? fontColorRed : fontColorWhite, curse == 14 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", -24, 178, curse == 15 ? fontColorRed : fontColorWhite, curse == 15 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		mars_drawTextwShadow("NTSC VDP 320x224p", 90, 193, fontColorWhite, fontColorGray);
		else
		mars_drawTextwShadow("PAL VDP 320x224p", 98, 193, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Sega 32X", 170, 208, fontColorWhite, fontColorGray);

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
			if(curse > 15)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
			if(curse < 1)
				curse = 15;
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
					tp_grid();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 8:
					screenFadeOut(1);
					tp_monoscope();
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
					tp_convergence();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 14:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 15:
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
	u16 button, pressedButton, oldButton  = 0xFFFF;

	Hw32xScreenFlip(0);
	
	while(!done) 
	{
		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Drop Shadow Test", -24, 88, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);		
		mars_drawTextwShadow("Striped Sprite Test", -24, 96, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Timing & Reflex Test", -24, 104, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Horizontal Stripes", -24, 112, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Vertical Stripes", -24, 120, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Checkerboard", -24, 128, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Backlit Zone Test", -24, 136, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", -24, 150, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", -24, 166, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		mars_drawTextwShadow("NTSC VDP 320x224p", 90, 193, fontColorWhite, fontColorGray);
		else
		mars_drawTextwShadow("PAL VDP 320x224p", 98, 193, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Sega 32X", 170, 208, fontColorWhite, fontColorGray);

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
			if(curse > 9)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
		 	curse--;
		 	if(curse < 1)
		 		curse = 9;
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
					marsVDP256Start();
					DrawMainBGwGillian();
				break;
					
				case 2:
					screenFadeOut(1);
					vt_striped_sprite_test();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					vt_reflex_test();
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 4:
					screenFadeOut(1);
					vt_horizontal_stripes();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 5:
					screenFadeOut(1);
					vt_vertical_stripes();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 6:
					screenFadeOut(1);
					vt_checkerboard();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 7:
					screenFadeOut(1);
					vt_backlitzone_test();
					marsVDP256Start();
					DrawMainBGwGillian();	
				break;

				case 8:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 9:
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
	u16 button, pressedButton, oldButton  = 0xFFFF;

	Hw32xScreenFlip(0);
	
	while (!done) 
	{
		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Sound Test", -15, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Help", -15, 146, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", -15, 161, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		mars_drawTextwShadow("NTSC VDP 320x224p", 90, 193, fontColorWhite, fontColorGray);
		else
		mars_drawTextwShadow("PAL VDP 320x224p", 98, 193, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Sega 32X", 170, 208, fontColorWhite, fontColorGray);

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
		 	if(curse > 3)
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
		 		curse = 3;
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
					DrawHelp(HELP_GENERAL);
					DrawMainBGwGillian();
				break;

				case 3:
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
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xScreenFlip(0);

	while (!done)
	{

		Hw32xFlipWait();

		DrawMainBGwGillian();
		loadTextPalette();

		mars_drawTextwShadow("Controller Test", -15, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Memory Viewer", -15, 88, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Test 32X SDRAM", -15, 96, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("32X BIOS Info", -15, 104, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Help", -15, 146, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Back to Main Menu", -15, 161, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorBlack : fontColorGray);

		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		mars_drawTextwShadow("NTSC VDP 320x224p", 90, 193, fontColorWhite, fontColorGray);
		else
		mars_drawTextwShadow("PAL VDP 320x224p", 98, 193, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Sega 32X", 170, 208, fontColorWhite, fontColorGray);

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
					ht_memory_viewer(0);
					HwMdClearScreen();
					marsVDP256Start();
					DrawMainBGwGillian();
				break;

				case 3:
					screenFadeOut(1);
					ht_test_32x_sdram();
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
	int frameDelay = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 done = 0;

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		DrawMainBG();
		loadTextPalette();

		mars_drawTextwShadow("Credits", 60, 35, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Ver. 0.6", 156, 57, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("1/27/2022", 156, 65, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Code and Port by:", -30, 81, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Dasutin", -22, 90, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Patterns:", -30, 98, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Artemio Urbina", -22, 106, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Menu Pixel Art:", -30, 114, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Asher", -22, 122, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Donna:", -30, 130, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Jose Salot", -22, 138, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("32X Toolchain:", -30, 146, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Marsdev (Chilly Willy)", -22, 154, fontColorWhite, fontColorGray);
		mars_drawTextwShadow("Info on using this test suite:", -30, 162, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("http://junkerhq.net/240p", -22, 170, fontColorWhite, fontColorGray);

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