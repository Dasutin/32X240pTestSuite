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
#include "draw.h"
#include "patterns.h"
#include "shared_objects.h"
#include "help.h"
#include "colorchart_pal.h"
#include "colorbars_palette.h"
#include "colorbars.h"
#include "colorbars_map.h"
#include "grid_palette.h"
#include "grid.h"
#include "grid_map.h"
#include "ebu_palette.h"
#include "ebu.h"
#include "ebu_map.h"
#include "colorbleedstripe_palette.h"
#include "colorbleedstripe.h"
#include "colorbleedstripe_map.h"
#include "colorbleedcheck_palette.h"
#include "colorbleedcheck.h"
#include "colorbleedcheck_map.h"
#include "grayramp_palette.h"
#include "grayramp.h"
#include "grayramp_map.h"
#include "convergencedots_palette.h"
#include "convergencedots.h"
#include "convergencedots_map.h"
#include "convergencecross_palette.h"
#include "convergencecross.h"
#include "convergencecross_map.h"
#include "convergencegrid_palette.h"
#include "convergencegrid.h"
#include "convergencegrid_map.h"
#include "convergencewrgb_palette.h"
#include "convergencewrgb.h"
#include "convergencewrgb_map.h"
#include "convergencewrgbborder_palette.h"
#include "convergencewrgbborder.h"
#include "convergencewrgbborder_map.h"
#include "monoscope_palette.h"
#include "monoscope.h"
#include "monoscope_map.h"
#include "sharpness_palette.h"
#include "sharpness.h"
#include "sharpness_map.h"
#include "sharpnessbrick_palette.h"
#include "sharpnessbrick.h"
#include "sharpnessbrick_map.h"
#include "smpte_palette.h"
#include "smpte.h"
#include "smpte_map.h"
#include "ire_palette.h"
#include "ire.h"
#include "ire_map.h"
#include "pluge_palette.h"
#include "pluge.h"
#include "pluge_map.h"
#include "colorref_palette.h"
#include "colorref.h"
#include "colorref_map.h"

void tp_pluge()
{
	u16 done = 0, draw = 1, IsNTSC = 1, text = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(plugentsc_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!IsNTSC)
			{
				init_tilemap(&tm, &pluge_map_Map, (uint8_t **)pluge_Reslist);
				Hw32xSetPalette(plugergb_Palette);
			} else {
				init_tilemap(&tm, &pluge_map_Map, (uint8_t **)pluge_Reslist);
				Hw32xSetPalette(plugentsc_Palette);
			}

			canvas_rebuild_id++;
			draw = 0;
		}

		if (text)
		{
			text--;
			if (!IsNTSC)
			{
				drawTextwHighlight("RGB FULL RANGE", 192, 16, fontColorRed, fontColorRedHighlight);
			} else {
				drawTextwHighlight("NTSC 7.5 IRE  ", 192, 16, fontColorRed, fontColorRedHighlight);
			}
		} else if (!text) {
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			IsNTSC = !IsNTSC;

			if (!IsNTSC)
			{
				Hw32xSetPalette(plugergb_Palette);
				drawTextwHighlight("RGB FULL RANGE", 192, 16, fontColorRed, fontColorRedHighlight);
			} else {
				Hw32xSetPalette(plugentsc_Palette);
				drawTextwHighlight("NTSC 7.5 IRE  ", 192, 16, fontColorRed, fontColorRedHighlight);
			}
			canvas_rebuild_id++;
			text = 300;
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
				DrawHelp(HELP_PLUGE);
				init_tilemap(&tm, &pluge_map_Map, (uint8_t **)pluge_Reslist);
				Hw32xSetPalette(plugentsc_Palette);
				draw = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_PLUGE);
			init_tilemap(&tm, &pluge_map_Map, (uint8_t **)pluge_Reslist);
			Hw32xSetPalette(plugentsc_Palette);
			draw = 1;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_colorchart()
{
	u16 done = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(colorbars_Palette);
	init_tilemap(&tm, &colorbars_Map, (uint8_t **)colorbars_Reslist);

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
				init_tilemap(&tm, &colorbars_Map, (uint8_t **)colorbars_Reslist);
				canvas_rebuild_id++;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_COLORS);
			Hw32xSetPalette(colorbars_Palette);
			init_tilemap(&tm, &colorbars_Map, (uint8_t **)colorbars_Reslist);
			canvas_rebuild_id++;
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
	u16 done = 0, draw = 1, Is75 = 0, text = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(ebu_Palette);
	init_tilemap(&tm, &ebu_map_Map, (uint8_t **)ebu_Reslist);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!Is75)
				Hw32xSetPalette(ebu_Palette);
			else
				Hw32xSetPalette(ebu75_Palette);

			canvas_rebuild_id++;
			draw = 0;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (text)
		{
			text--;
			if (!Is75)
			{
				drawTextwHighlight("100%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			} else {
				drawTextwHighlight(" 75%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			}
		} else if (!text) {
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			Is75 = !Is75;

			if (!Is75)
			{
				Hw32xSetPalette(ebu_Palette);
				drawTextwHighlight("100%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			} else {
				Hw32xSetPalette(ebu75_Palette);
				drawTextwHighlight(" 75%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			}
			canvas_rebuild_id++;
			text = 300;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_601CB);
				Hw32xSetPalette(ebu_Palette);
				init_tilemap(&tm, &ebu_map_Map, (uint8_t **)ebu_Reslist);
				canvas_rebuild_id++;
				draw = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			DrawHelp(HELP_601CB);
			Hw32xSetPalette(ebu_Palette);
			init_tilemap(&tm, &ebu_map_Map, (uint8_t **)ebu_Reslist);
			canvas_rebuild_id++;
			draw = 1;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_smpte_color_bars()
{
	u16 done = 0, draw = 1, Is75 = 0, text = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(smpte100_Palette);
	init_tilemap(&tm, &smpte_map_Map, (uint8_t **)smpte_Reslist);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (draw)
		{
			if (!Is75)
			{
				init_tilemap(&tm, &smpte_map_Map, (uint8_t **)smpte_Reslist);
				Hw32xSetPalette(smpte100_Palette);
			} else {
				init_tilemap(&tm, &smpte_map_Map, (uint8_t **)smpte_Reslist);
				Hw32xSetPalette(smpte75_Palette);
			}
			canvas_rebuild_id++;
			draw = 0;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (text)
		{
			text--;
			if (!Is75)
			{
				drawTextwHighlight("100%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			} else {
				drawTextwHighlight(" 75%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			}
		} else if (!text) {
			canvas_rebuild_id++;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			Is75 = !Is75;

			if (!Is75)
			{
				Hw32xSetPalette(smpte100_Palette);
				drawTextwHighlight("100%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			} else {
				Hw32xSetPalette(smpte75_Palette);
				drawTextwHighlight(" 75%", 256, 8, fontColorWhite, fontColorWhiteHighlight);
			}
			canvas_rebuild_id++;
			text = 300;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				screenFadeOut(1);
				DrawHelp(HELP_SMPTE);
				init_tilemap(&tm, &smpte_map_Map, (uint8_t **)smpte_Reslist);
				Hw32xSetPalette(smpte100_Palette);
				canvas_rebuild_id++;
				draw = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			screenFadeOut(1);
			DrawHelp(HELP_SMPTE);
			init_tilemap(&tm, &smpte_map_Map, (uint8_t **)smpte_Reslist);
			Hw32xSetPalette(smpte100_Palette);
			canvas_rebuild_id++;
			draw = 1;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_ref_color_bars()
{
	u16 done = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(colorref_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		init_tilemap(&tm, &colorref_map_Map, (uint8_t **)colorref_Reslist);
		canvas_rebuild_id++;

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
				Hw32xSetPalette(colorref_Palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_601CB);
			Hw32xSetPalette(colorref_Palette);
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);

	}
	return;
}

void tp_color_bleed_check()
{
	u16 done = 0, pattern = 1, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(colorbleedstripe_Palette);

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
		{
			screenFadeOut(1);
			done = 1;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_BLEED);
				Hw32xSetPalette(colorbleedstripe_Palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_BLEED);
			Hw32xSetPalette(colorbleedstripe_Palette);
		}

		switch (pattern)
		{
			case 1:
				init_tilemap(&tm, &colorbleedstripe_map_Map, (uint8_t **)colorbleedstripe_Reslist);
				canvas_rebuild_id++;
				break;

			case 2:
				init_tilemap(&tm, &colorbleedcheck_map_Map, (uint8_t **)colorbleedcheck_Reslist);
				canvas_rebuild_id++;
				break;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);

	}
	return;
}

void tp_grid()
{
	u16 done = 0, pattern = 1, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(grid_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		init_tilemap(&tm, &grid_map_Map, (uint8_t **)grid_Reslist);
		canvas_rebuild_id++;

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		switch (pattern)
		{
			case 1:
				setColor(0, 0, 0, 0);
				break;

			case 2:
				setColor(0, 6, 6, 6);
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
				init_tilemap(&tm, &grid_map_Map, (uint8_t **)grid_Reslist);
				Hw32xSetPalette(grid_Palette);
				canvas_rebuild_id++;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRID);
			init_tilemap(&tm, &grid_map_Map, (uint8_t **)grid_Reslist);
			Hw32xSetPalette(grid_Palette);
			canvas_rebuild_id++;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);

	}
	return;
}

void tp_monoscope()
{
	u16 done = 0, pattern = 1, gray = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(monoscope_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();

		init_tilemap(&tm, &monoscope_map_Map, (uint8_t **)monoscope_Reslist);
		canvas_rebuild_id++;

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			button = MARS_SYS_COMM10;

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (!gray)
			setColor(0, 0, 0, 0);
		else
			setColor(0, 13, 13, 13);

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
				Hw32xSetPalette(monoscope_Palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_MONOSCOPE);
			Hw32xSetPalette(monoscope_Palette);
		}

		switch (pattern)
		{
			case 1:
				setColor(2, 30, 30, 30);
				break;

			case 2:
				setColor(2, 26, 26, 26);
				break;

			case 3:
				setColor(2, 21, 21, 21);
				break;

			case 4:
				setColor(2, 17, 17, 17);
				break;

			case 5:
				setColor(2, 13, 13, 13);
				break;

			case 6:
				setColor(2, 8, 8, 8);
				break;

			case 7:
				setColor(2, 4, 4, 4);
				break;
		}

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

void tp_gray_ramp()
{
	u16 done = 0, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(grayramp_Palette);
	init_tilemap(&tm, &grayramp_map_Map, (uint8_t **)grayramp_Reslist);

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
				Hw32xSetPalette(grayramp_Palette);
				init_tilemap(&tm, &grayramp_map_Map, (uint8_t **)grayramp_Reslist);
				canvas_rebuild_id++;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRAY);
			Hw32xSetPalette(grayramp_Palette);
			init_tilemap(&tm, &grayramp_map_Map, (uint8_t **)grayramp_Reslist);
			canvas_rebuild_id++;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_white_rgb()
{
	u16 done = 0, color = 1, sel = 1, custom = 0, r = 31, g = 31, b = 31;
	int l = 320 * 224;
	char str[20], num[4];
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;

	setColor(0, r, g, b);
	setColor(1, 0, 0, 0);
	setColor(2, 31, 0, 0);
	setColor(3, 0, 31, 0);
	setColor(4, 0, 0, 31);

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

		setColor(0, r, g, b);

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
	u16 done = 0, text = 0, draw = 1, fpcamera_x = 0, fpcamera_y = 0;
	u16 irevals[] = {13,25,41,53,66,82,94}, ire = 6;
	char str[2];
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(ire_Palette);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		loadTextPalette();

		button = MARS_SYS_COMM8;

		if (draw)
		{
			setColor(0, 30, 30, 30);
			init_tilemap(&tm, &ire_map_Map, (uint8_t **)ire_Reslist);
			canvas_rebuild_id++;
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
			canvas_rebuild_id++;
			text = 300;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			if (ire != 6)
				ire++;
			canvas_rebuild_id++;
			text = 300;
		}

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_THREE)
		{
			if (pressedButton & SEGA_CTRL_C)
			{
				DrawHelp(HELP_IRE);
				init_tilemap(&tm, &ire_map_Map, (uint8_t **)ire_Reslist);
				canvas_rebuild_id++;
				Hw32xSetPalette(ire_Palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_IRE);
			init_tilemap(&tm, &ire_map_Map, (uint8_t **)ire_Reslist);
			canvas_rebuild_id++;
			Hw32xSetPalette(ire_Palette);
		}

		switch (ire)
		{
			case 0:
				// 13
				setColor(0, 4, 4, 4);
				break;

			case 1:
				// 25
				setColor(0, 8, 8, 8);
				break;

			case 2:
				// 41
				setColor(0, 13, 13, 13);
				break;

			case 3:
				// 53
				setColor(0, 17, 17, 17);
				break;

			case 4:
				// 66
				setColor(0, 21, 21, 21);
				break;

			case 5:
				// 82
				setColor(0, 26, 26, 26);
				break;

			case 6:
				// 94
				setColor(0, 30, 30, 30);
				break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
			done = 1;
		}

		if (text)
		{
			text--;
			drawTextwHighlight("IRE:", 256, 200, fontColorWhite, fontColorWhiteHighlight);
			intToStr(irevals[ire], str, 2);
			drawTextwHighlight(str, 288, 200, fontColorWhite, fontColorWhiteHighlight);
		} else if (!text) {
			canvas_rebuild_id++;
		}

		draw_tilemap(&tm, fpcamera_x, fpcamera_y, 0);
		draw_setScissor(0, 0, 320, 224);

		Hw32xScreenFlip(0);
	}
	return;
}

void tp_sharpness()
{
	u16 done = 0, pattern = 1, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(sharpness_Palette);

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
						Hw32xSetPalette(sharpness_Palette);
						init_tilemap(&tm, &sharpness_map_Map, (uint8_t **)sharpness_Reslist);
						canvas_rebuild_id++;
						break;

					case 2:
						Hw32xSetPalette(sharpnessbrick_Palette);
						init_tilemap(&tm, &sharpnessbrick_map_Map, (uint8_t **)sharpnessbrick_Reslist);
						canvas_rebuild_id++;
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
					Hw32xSetPalette(sharpness_Palette);
					init_tilemap(&tm, &sharpness_map_Map, (uint8_t **)sharpness_Reslist);
					canvas_rebuild_id++;
					break;

				case 2:
					Hw32xSetPalette(sharpnessbrick_Palette);
					init_tilemap(&tm, &sharpnessbrick_map_Map, (uint8_t **)sharpnessbrick_Reslist);
					canvas_rebuild_id++;
					break;
			}
		}

		switch (pattern)
		{
			case 1:
				Hw32xSetPalette(sharpness_Palette);
				init_tilemap(&tm, &sharpness_map_Map, (uint8_t **)sharpness_Reslist);
				canvas_rebuild_id++;
				break;

			case 2:
				Hw32xSetPalette(sharpnessbrick_Palette);
				init_tilemap(&tm, &sharpnessbrick_map_Map, (uint8_t **)sharpnessbrick_Reslist);
				canvas_rebuild_id++;
				break;
		}

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

void tp_overscan()
{
	u16 done = 0, sel = 0;
	int left = 0, right = 320, top = 0, bottom = 223;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	//u16 displayBottom, displayRight;

	setColor(0, 31, 31, 31);
	setColor(1, 15, 15, 15);

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
		u16 l, r, t, b;

		l = left;
		r = right;
		t = top;
		b = bottom;

		for (int i = t; i <= b; i++)
			drawLine(l, i, r, 1);

		// Text
		intToStr(top, datat, 1);
		drawTextwHighlight("Top:", 96, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("   pixels", 160, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(datat, 160, 96, sel == 0 ? fontColorRed : fontColorWhite, sel == 0 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(abs(bottom-223), datab, 1);
		drawTextwHighlight("Bottom:", 96, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("   pixels", 160, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(datab, 160, 104, sel == 1 ? fontColorRed : fontColorWhite, sel == 1 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(left, datal, 1);
		drawTextwHighlight("Left:", 96, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("   pixels", 160, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(datal, 160, 112, sel == 2 ? fontColorRed : fontColorWhite, sel == 2 ? fontColorRedHighlight : fontColorWhiteHighlight);

		intToStr(abs(right-320), datar, 1);
		drawTextwHighlight("Right:", 96, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight("   pixels", 160, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);
		drawTextwHighlight(datar, 160, 120, sel == 3 ? fontColorRed : fontColorWhite, sel == 3 ? fontColorRedHighlight : fontColorWhiteHighlight);

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
				setColor(0, 31, 31, 31);
				setColor(1, 15, 15, 15);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_OVERSCAN);
			setColor(0, 31, 31, 31);
			setColor(1, 15, 15, 15);
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
	return;
}

void tp_convergence()
{
	u16 done = 0, pattern = 1, fpcamera_x = 0, fpcamera_y = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;

	Hw32xSetPalette(convergencegrid_Palette);

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
				Hw32xSetPalette(convergencegrid_Palette);
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CONVERGENCE);
			Hw32xSetPalette(convergencegrid_Palette);
		}

		switch (pattern)
		{
			case 1:
				Hw32xSetPalette(convergencegrid_Palette);
				init_tilemap(&tm, &convergencegrid_map_Map, (uint8_t **)convergencegrid_Reslist);
				canvas_rebuild_id++;
				break;

			case 2:
				Hw32xSetPalette(convergencegrid_Palette);
				init_tilemap(&tm, &convergencedots_map_Map, (uint8_t **)convergencedots_Reslist);
				canvas_rebuild_id++;
				break;

			case 3:
				Hw32xSetPalette(convergencegrid_Palette);
				init_tilemap(&tm, &convergencecross_map_Map, (uint8_t **)convergencecross_Reslist);
				canvas_rebuild_id++;
				break;

			case 4:
				Hw32xSetPalette(convergencewrgb_Palette);
				init_tilemap(&tm, &convergencewrgb_map_Map, (uint8_t **)convergencewrgb_Reslist);
				canvas_rebuild_id++;
				break;

			case 5:
				Hw32xSetPalette(convergencewrgbborder_Palette);
				init_tilemap(&tm, &convergencewrgbborder_map_Map, (uint8_t **)convergencewrgbborder_Reslist);
				canvas_rebuild_id++;
				break;
		}

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
