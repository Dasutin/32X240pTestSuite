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
#include "32x_images.h"
#include "draw.h"
#include "shared_objects.h"
#include "main.h"
#include "patterns.h"
#include "tests.h"
#include "help.h"

// canvas_width + scrollwidth
uint32_t canvas_pitch = 320;
// canvas_height + scrollheight
uint32_t canvas_yaw = 224;

int main(void)
{
	int curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	marsVDP256Start();

	SetSH2SR(1);

	initMainBGwGil();

	while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

	Hw32xScreenFlip(0);

	while (1)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 72;
		drawTextwHighlight("Test Patterns", 50, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Video tests", 50, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Audio tests", 50, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Hardware tools", 50, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 34;
		drawTextwHighlight("Help", 50, pos += 8, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Credits", 50, pos += 8, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 6)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
				curse = 6;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					menu_tp();
					loadMainBGwGilPalette();
					break;

				case 2:
					screenFadeOut(1);
					menu_vt();
					loadMainBGwGilPalette();
					break;

				case 3:
					screenFadeOut(1);
					menu_at();
					loadMainBGwGilPalette();
					break;

				case 4:
					screenFadeOut(1);
					menu_ht();
					loadMainBGwGilPalette();
					break;

				case 5:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					redrawBGwGil();
					break;

				case 6:
					screenFadeOut(1);
					credits();
					redrawBGwGil();
					break;
			}
		}

		Hw32xScreenFlip(0);
	}
	return 0;
}

void menu_tp()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBGwGil();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 88;
		drawTextwHighlight("Color & Black Levels", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Geometry", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		pos += 8;
		drawTextwHighlight("Help", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Credits", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 4)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
				curse = 4;
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

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					menu_color_black_levels();
					loadMainBGwGilPalette();
					break;

				case 2:
					screenFadeOut(1);
					menu_geo();
					loadMainBGwGilPalette();
					break;

				case 3:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					credits();
					redrawBGwGil();
					break;
			}
		}

		Hw32xScreenFlip(0);
	}
	return;
}

void menu_color_black_levels()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBGwGil();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 64;
		drawTextwHighlight("Pluge", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Color Bars", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("EBU Color Bars", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("SMPTE Color Bars", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Referenced Color Bars", 40, pos += 8, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Color Bleed Check", 40, pos += 8, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Gray Ramp", 40, pos += 8, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("White & RGB Screens", 40, pos += 8, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("100 IRE", 40, pos += 8, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Sharpness", 40, pos += 8, curse == 10 ? fontColorRed : fontColorWhite, curse == 10 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		drawTextwHighlight("Back to Patterns Menu", 40, pos += 8, curse == 11 ? fontColorRed : fontColorWhite, curse == 11 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;
		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 11)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
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

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					initMainBG();
					redrawBGwGil();
					break;

				case 2:
					screenFadeOut(1);
					canvas_rebuild_id++;
					tp_colorchart();
					canvas_rebuild_id++;
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 3:
					screenFadeOut(1);
					tp_colorbars();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					tp_smpte_color_bars();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 5:
					screenFadeOut(1);
					tp_ref_color_bars();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 6:
					screenFadeOut(1);
					tp_color_bleed_check();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 7:
					screenFadeOut(1);
					tp_gray_ramp();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 8:
					screenFadeOut(1);
					tp_white_rgb();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 9:
					screenFadeOut(1);
					tp_100_ire();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 10:
					screenFadeOut(1);
					tp_sharpness();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 11:
					screenFadeOut(1);
					done = 1;
					break;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		Hw32xScreenFlip(0);
	}
	return;
}

void menu_geo()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBGwGil();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 88;
		drawTextwHighlight("Monoscope", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Grid", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Overscan", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Convergence", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		drawTextwHighlight("Back to Patterns Menu", 40, pos += 8, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 5)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
				curse = 5;
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

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					tp_monoscope();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 2:
					screenFadeOut(1);
					tp_grid();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 3:
					screenFadeOut(1);
					tp_overscan();
					HwMdClearScreen();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					tp_convergence();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 5:
					screenFadeOut(1);
					done = 1;
					break;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		Hw32xScreenFlip(0);
	}
	return;
}

void menu_vt()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton  = 0xFFFF;

	marsVDP256Start();
	initMainBGwGil();
	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		drawBGwGil();
		loadTextPalette();

		pos = 50;
		drawTextwHighlight("Drop Shadow Test", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Striped Sprite Test", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Lag Test", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Timing & Reflex Test", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Scroll Test", 40, pos += 8, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Grid Scroll Test", 40, pos += 8, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Horiz/Vert Stripes", 40, pos += 8, curse == 7 ? fontColorRed : fontColorWhite, curse == 7 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Checkerboard", 40, pos += 8, curse == 8 ? fontColorRed : fontColorWhite, curse == 8 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Backlit Zone Test", 40, pos += 8, curse == 9 ? fontColorRed : fontColorWhite, curse == 9 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Disappearing Logo", 40, pos += 8, curse == 10 ? fontColorRed : fontColorWhite, curse == 10 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		drawTextwHighlight("Help", 40, pos += 8, curse == 11 ? fontColorRed : fontColorWhite, curse == 11 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Back to Main Menu", 40, pos += 8, curse == 12 ? fontColorRed : fontColorWhite, curse == 12 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 12)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
				curse = 12;
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

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					canvas_rebuild_id++;
					vt_drop_shadow_test();
					canvas_pitch = 320;
					canvas_yaw = 224;
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 2:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_striped_sprite_test();
					canvas_pitch = 320;
					canvas_yaw = 224;
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 3:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_lag_test();
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					vt_reflex_test();
					HwMdClearScreen();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 5:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_scroll_test();
					HwMdClearScreen();
					canvas_pitch = 320;
					canvas_yaw = 224;
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 6:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_gridscroll_test();
					HwMdClearScreen();
					canvas_pitch = 320;
					canvas_yaw = 224;
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 7:
					screenFadeOut(1);
					vt_stripes();
					marsVDP256Start();
					initMainBG();
					redrawBGwGil();
					break;

				case 8:
					screenFadeOut(1);
					vt_checkerboard();
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 9:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_backlitzone_test();
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 10:
					screenFadeOut(1);
					canvas_rebuild_id++;
					vt_DisappearingLogo();
					initMainBG();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 11:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					redrawBGwGil();
					break;

				case 12:
					screenFadeOut(1);
					done = 1;
					break;
			}

			button = MARS_SYS_COMM8;

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;

			pressedButton = button & ~oldButton;
			oldButton = button;
		}

		Hw32xScreenFlip(0);
	}
	return;
}

void menu_at()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBGwGil();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 80;
		drawTextwHighlight("Sound Test", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Audio Sync Test", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		pos += 8;
		drawTextwHighlight("Help", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Back to Main Menu", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 4)
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
			if (curse < 1)
				curse = 4;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
			DrawHelp(HELP_GENERAL);

		if (pressedButton & SEGA_CTRL_A)
		{
			switch (curse)
			{
				case 1:
					screenFadeOut(1);
					at_sound_test();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 2:
					screenFadeOut(1);
					at_audiosync_test();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 3:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					done = 1;
					break;
			}
			button = MARS_SYS_COMM8;

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
				button = MARS_SYS_COMM10;

			pressedButton = button & ~oldButton;
			oldButton = button;
		}

		Hw32xScreenFlip(0);
	}
	return;
}

void menu_ht()
{
	int done = 0, curse = 1, pos;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBGwGil();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawBGwGil();
		loadTextPalette();

		pos = 72;
		drawTextwHighlight("Controller Test", 40, pos += 8, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("SDRAM Check", 40, pos += 8, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Memory Viewer", 40, pos += 8, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("BIOS Info", 40, pos += 8, curse == 4 ? fontColorRed : fontColorWhite, curse == 4 ? fontColorRedHighlight : fontColorWhiteHighlight);
		pos += 8;
		pos += 8;
		drawTextwHighlight("Help", 40, pos += 8, curse == 5 ? fontColorRed : fontColorWhite, curse == 5 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("Back to Main Menu", 40, pos += 8, curse == 6 ? fontColorRed : fontColorWhite, curse == 6 ? fontColorRedHighlight : fontColorWhiteHighlight);

		drawResolution();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			curse++;
			if (curse > 6)
				curse = 1;
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			curse--;
			if (curse < 1)
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

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_GENERAL);
			}
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
					redrawBGwGil();
					break;

				case 2:
					ht_test_32x_sdram();
					HwMdClearScreen();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 3:
					screenFadeOut(1);
					ht_memory_viewer(0);
					HwMdClearScreen();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 4:
					screenFadeOut(1);
					ht_check_32x_bios_crc(0);
					HwMdClearScreen();
					marsVDP256Start();
					redrawBGwGil();
					break;

				case 5:
					screenFadeOut(1);
					DrawHelp(HELP_GENERAL);
					redrawBGwGil();
					break;

				case 6:
					screenFadeOut(1);
					done = 1;
					break;
			}
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		Hw32xScreenFlip(0);
	}
	return;
}

void credits()
{
	int done = 0;
	unsigned short button, pressedButton, oldButton = 0xFFFF;

	initMainBG();

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		drawMainBG();
		loadTextPalette();

		drawTextwHighlight("Ver. 1.10", 80, 35, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("07/10/2023", 166, 35, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Code and Port by:", 35, 50, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Dustin Dembrosky (@Dasutin)", 43, 58, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Patterns:", 35, 66, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Artemio Urbina (@artemio)", 43, 74, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Monoscope Pattern:", 35, 82, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Keith Raney (@khmr33)", 43, 90, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Menu Pixel Art:", 35, 98, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Asher", 43, 106, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Donna Art:", 35, 114, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Jose Salot (@pepe_salot)", 43, 122, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("32X Toolchain:", 35, 130, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Joseph Fenton (Chilly Willy)", 43, 138, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Tile Mapper:", 35, 146, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("Victor Luchitz (@vluchitz)", 43, 154, fontColorWhite, fontColorWhiteHighlight);

		drawTextwHighlight("Info on using this test suite:", 35, 162, fontColorGreen, fontColorGreenHighlight);
		drawTextwHighlight("http://junkerhq.net/240p", 43, 170, fontColorWhite, fontColorWhiteHighlight);

		drawQRCode(248, 88, 32, 32);

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

		if (pressedButton & SEGA_CTRL_B)
		{
			screenFadeOut(1);
			done = 1;
		}

		Hw32xScreenFlip(0);
	}
	return;
}