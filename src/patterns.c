/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin
 * Copyright (C)2011-2021 Artemio Urbina
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
#include "patterns.h"
#include "shared_objects.h"
#include "help.h"

void tp_pluge()
{
	u16 done = 0;
	int frameDelay = 5;
	int pattern = 1;
	unsigned short button, pressedButton, oldButton = 0xFFFF;
	extern const u16 PLUGE_NTSC_PALETTE_DATA[];
	extern const u16 PLUGE_RGB_PALETTE_DATA[];
	extern const u8 PLUGE_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = PLUGE_NTSC_PALETTE_DATA[i] & 0x7FFF;
	}

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	switch (pattern) {
			case 1:
				for (int i = 0; i < 255; i++){
					cram16[i] = PLUGE_NTSC_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = PLUGE_RGB_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			pattern++;
	
			if(pattern > 2){
		 		pattern = 1;
			}
		}

    	if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_PLUGE);

			for (int i = 0; i < 255; i++){
				cram16[i] = PLUGE_NTSC_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(PLUGE_PATTERN);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		drawLineTable(4);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_colorchart()
{
	u16 done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORCHART_PALETTE_DATA[];
	extern const u8 COLORCHART_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = COLORCHART_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_COLORS);

			for (int i = 0; i < 255; i++){
				cram16[i] = COLORCHART_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(COLORCHART_PATTERN);

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_colorbars()
{
	u16 done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 EBU_COLORBARS_PALETTE_DATA[];
	extern const u8 EBU_COLORBARS_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = EBU_COLORBARS_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_COLORS);

			for (int i = 0; i < 255; i++){
				cram16[i] = EBU_COLORBARS_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(EBU_COLORBARS_PATTERN);

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_smpte_color_bars()
{
	u16 done = 0;
	int frameDelay = 1;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 SMPTE75IRE_PALETTE_DATA[];
	extern const u16 SMPTE100IRE_PALETTE_DATA[];
	extern const u8 SMPTE100IRE_PATTERN[];
	extern const u8 TEST_SMTPE_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done) 
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	switch (pattern) {
			case 1:
				for (int i = 0; i < 255; i++){
					cram16[i] = SMPTE75IRE_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = SMPTE100IRE_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
		}

    	if (pressedButton & SEGA_CTRL_B)
		{
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
			DrawHelp(HELP_SMPTE);

			for (int i = 0; i < 255; i++){
				cram16[i] = SMPTE75IRE_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(SMPTE100IRE_PATTERN);

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_ref_color_bars()
{
	u16 done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORREF_PALETTE_DATA[];
	extern const u8 COLORREF_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = COLORREF_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_601CB);

			for (int i = 0; i < 255; i++){
				cram16[i] = COLORREF_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(COLORREF_PATTERN);

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_color_bleed_check()
{
	u16 done = 0;
	int frameDelay = 0;
	int background = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 COLORBLEED_PALETTE_DATA[];
	extern const u8 COLORBLEED_PATTERN;
	extern const u8 COLORBLEED_CHECKERBOARD_TILE;
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = COLORBLEED_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
			background++;
	
			if(background > 2){
		 		background = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_BLEED);

			for (int i = 0; i < 255; i++){
				cram16[i] = COLORBLEED_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (background) {
				case 1:
					drawBG(&COLORBLEED_PATTERN);
				break;
				
				case 2:
					drawBG(&COLORBLEED_CHECKERBOARD_TILE);
				break;
			}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

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
	extern const u16 GRID_PALETTE_DATA[];
	extern const u16 GRID_GRAY_PALETTE_DATA[];
	extern const u8 GRID_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 3; i++){
		cram16[i] = GRID_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	switch (pattern) {
			case 1:
				for (int i = 0; i < 255; i++){
					cram16[i] = GRID_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = GRID_GRAY_PALETTE_DATA[i] & 0x7FFF;
				}
			break;
		}

    	if (pressedButton & SEGA_CTRL_B)
		{
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

			for (int i = 0; i < 3; i++){
				cram16[i] = GRID_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(GRID_PATTERN);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		drawLineTable(4);

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
	extern const u8 MONOSCOPE_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	cram16[0] = COLOR(30, 30, 30);
	cram16[1] = COLOR(0, 0, 0);
	cram16[2] = COLOR(31, 0, 0);

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
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

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(MONOSCOPE_PATTERN);

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

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		drawLineTable(4);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_gray_ramp()
{
	u16 done = 0;
	int frameDelay = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 GRAYRAMP_PALETTE_DATA[];
	extern const u8 GRAYRAMP_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = GRAYRAMP_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GRAY);

			for (int i = 0; i < 255; i++){
				cram16[i] = GRAYRAMP_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(GRAYRAMP_PATTERN);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		drawLineTable(4);

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_white_rgb()
{
	u16 done = 0;
	int frameDelay = 5;
	int colorloop = 1;
	int i;
	int l = 320*224 + 0x100;
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;
	
	cleanup();
	marsVDP32KStart();

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;
	
	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			colorloop++;
			if(colorloop > 5)
				colorloop = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_GENERAL);
			marsVDP32KStart();
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (colorloop){
			case 1:
				for (i=0x100; i<=l; i++){
					frameBuffer16[i] = 0x7FFF;
				} 
			break;

			case 2:
				for (i=0x100; i<=l; i++){
					frameBuffer16[i] = 0x0000;
				}
			break;

			case 3:
				for (i=0x100; i<=l; i++){
					frameBuffer16[i] = 0x001F;
				}
			break;
				
			case 4:
				for (i=0x100; i<=l; i++){
					frameBuffer16[i] = 0x03E0;
				}
			break;

			case 5:
				for (i=0x100; i<=l; i++){
					frameBuffer16[i] = 0x7C00;
				}
			break;
		}

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void tp_100_ire()
{
	u16 done = 0;
	int frameDelay = 5;
	int pattern = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u8 IRE_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	cram16[0] = COLOR(0, 0, 0);

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done)
	{
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
			DrawHelp(HELP_IRE);
			cram16[0] = COLOR(0, 0, 0);
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawBG(IRE_PATTERN);

    	switch (pattern) {
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
				cram16[1] = COLOR(4, 4, 4);
			break;
		}

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		drawLineTable(4);

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
	extern const u16 SHARPNESS_PALETTE_DATA[];
	extern const u16 SHARPNESS_BRICK_PALETTE_DATA[];
	extern const u8 SHARPNESS_PATTERN[];
	extern const u8 SHARPNESS_BRICK_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 4; i++)	{
		cram16[i] = SHARPNESS_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done) 
	{
		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
		oldButton = button;

		if (pressedButton & SEGA_CTRL_B)
		{
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
			DrawHelp(HELP_SHARPNESS);

			switch (pattern) {
			case 1:
				for (int i = 0; i < 4; i++)	{
					cram16[i] = SHARPNESS_PALETTE_DATA[i] & 0x7FFF;
				}
				drawBG(SHARPNESS_PATTERN);
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = SHARPNESS_BRICK_PALETTE_DATA[i] & 0x7FFF;
				}
				drawBG(SHARPNESS_BRICK_PATTERN);
			break;
			}
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (pattern) {
			case 1:
				for (int i = 0; i < 4; i++)	{
					cram16[i] = SHARPNESS_PALETTE_DATA[i] & 0x7FFF;
				}
				drawBG(SHARPNESS_PATTERN);
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = SHARPNESS_BRICK_PALETTE_DATA[i] & 0x7FFF;
				}
				drawBG(SHARPNESS_BRICK_PATTERN);
			break;
		}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

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
	extern const u16 CONVERGENCE_GRID_PALETTE_DATA[];
	extern const u16 CONVERGENCE_COLOR_PALETTE[];
	extern const u16 CONVERGENCE_COLOR_BORDER_PALETTE[];
	extern const u8 CONVERGENCE_GRID_TILE[];
	extern const u8 CONVERGENCE_STARS_TILE[];
	extern const u8 CONVERGENCE_DOTS_TILE[];
	extern const u8 CONVERGENCE_COLOR_TILE[];
	extern const u8 CONVERGENCE_COLOR_BORDER_TILE[];
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 255; i++){
		cram16[i] = CONVERGENCE_GRID_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS; 

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done) 
	{
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
				cram16[i] = CONVERGENCE_GRID_PALETTE_DATA[i] & 0x7FFF;
			}
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

    	switch (pattern) {
			case 1:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PALETTE_DATA[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_GRID_TILE);
			break;
				
			case 2:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PALETTE_DATA[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_DOTS_TILE);
			break;

			case 3:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_GRID_PALETTE_DATA[i] & 0x7FFF;
				}
				cram16[1] = COLOR(31, 31, 31);
				drawBG(CONVERGENCE_STARS_TILE);
			break;
		
			case 4:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_COLOR_PALETTE[i] & 0x7FFF;
				}
				drawBG(CONVERGENCE_COLOR_TILE);
			break;
				
			case 5:
				for (int i = 0; i < 255; i++){
					cram16[i] = CONVERGENCE_COLOR_BORDER_PALETTE[i] & 0x7FFF;
				}
				drawBG(CONVERGENCE_COLOR_BORDER_TILE);
			break;
		}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}
