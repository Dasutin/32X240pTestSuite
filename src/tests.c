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
#include "tests.h"
#include "shared_objects.h"
#include "help.h"

// Global Variables
extern int fontColorWhite;
extern int fontColorRed;
extern int fontColorGreen;
extern int fontColorGray;
extern int fontColorBlack;

void vt_drop_shadow_test()
{
	int done = 0;
	int frameDelay = 1;
	int x = 10;
	int y = 10;
	int bee_mirror = 1; //Starts right
	int frameCount = 0;
	int evenFrames = 0;
	extern const vu8 BEE_PATTERN[];
	extern const vu8 BEE_SHADOW_TILE[];
	extern const vu8 MARKER_SHADOW_TILE[];
	int changeSprite = 0;
	int background = 1;
	extern const vu16 TEST_PALETTE[];
	extern const u8 MOTOKO_PATTERN[];
	extern const u8 H_STRIPES_SHADOW_TILE[];
	extern const u8 CHECKERBOARD_SHADOW_TILE[];
	vu16 *cram16 = &MARS_CRAM;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;

	marsVDP256Start();

	for (int i = 0; i < 12+13; i++){
		cram16[i] = TEST_PALETTE[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done) {

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

    	pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_A)
		{
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			background++;
	
			if(background > 3){
		 		background = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			if(changeSprite == 0){
				changeSprite = 1;
			}	
			else {
				changeSprite = 0;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_SHADOW);

			for (int i = 0; i < 12+13; i++){
				cram16[i] = TEST_PALETTE[i] & 0x7FFF;
			}
		}

		if (button & SEGA_CTRL_UP)
		{
			y--;
			if(y < 1)
				y = 1;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			y++;
			if(y > 192)
				y = 192;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			bee_mirror = 0;
			x--;
			if(x < 1)
				x = 1;
			
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			bee_mirror = 1;
			x++;
			if(x > 288)
				x = 288;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (background) {
				case 1:
					drawBG(MOTOKO_PATTERN);
				break;
				
				case 2:
					drawBG(CHECKERBOARD_SHADOW_TILE);
				break;

				case 3:
					drawBG(H_STRIPES_SHADOW_TILE);
				break;
		}

		if (changeSprite == 0){
		if (frameCount % 2 == evenFrames ) {
		drawSprite(BEE_SHADOW_TILE,x,y,32,32,bee_mirror,0);
		}
		drawSprite(BEE_PATTERN,x-20,y-20,32,32,bee_mirror,0);
		}
		else {
		if (frameCount % 2 == evenFrames ) {
		drawSprite(MARKER_SHADOW_TILE,x,y,32,32,0,0);
		}
		}
		
		frameCount++;

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

    	currentFB ^= 1;
    	MARS_VDP_FBCTL = currentFB;
		
		Hw32xDelay(frameDelay);
	}
	return;
}

void vt_striped_sprite_test()
{
	int done = 0;
	int x = 10;
	int y = 10;
	int background = 1;
	extern const vu8 MARKER_STRIPED_TILE[];
	extern const u8 H_STRIPES_SHADOW_TILE[];
	extern const u8 CHECKERBOARD_SHADOW_TILE[];
	extern const vu16 TEST_PALETTE[];
	extern const u8 MOTOKO_PATTERN[];
	vu16 *cram16 = &MARS_CRAM;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;

	marsVDP256Start();

	Hw32xScreenClear();
	Hw32xSetBGColor(0,0,0,0);

	for (int i = 0; i < 12+13; i++){
		cram16[i] = TEST_PALETTE[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	MARS_SYS_COMM6 = MASTER_STATUS_OK;

	while (!done) {
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
	
			if(background > 3){
		 		background = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			background--;
	
			if(background < 1){
		 		background = 3;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_STRIPED);

			for (int i = 0; i < 12+13; i++){
				cram16[i] = TEST_PALETTE[i] & 0x7FFF;
			}
		}

		if (button & SEGA_CTRL_UP)
		{
			
			y--;
			if(y < 1)
				y = 1;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			
			y++;
			if(y > 192)
				y = 192;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			x--;
			if(x < 1)
				x = 1;
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			x++;
			if(x > 288)
				x = 288;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (background) {
				case 1:
					memcpy(frameBuffer16 + 0x100, MOTOKO_PATTERN, 320*224);
				break;
				
				case 2:
					memcpy(frameBuffer16 + 0x100, CHECKERBOARD_SHADOW_TILE, 320*224);
				break;

				case 3:
					memcpy(frameBuffer16 + 0x100, H_STRIPES_SHADOW_TILE, 320*224);
				break;
		}

		drawS(MARKER_STRIPED_TILE,x,y,32,32);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;
	}
	return;
}

void vt_horizontal_stripes()
{
	u16 done = 0;
	int frameDelay = 1;
	int test = 1;
	int manualtest = 1;
	int palette = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	MARS_SYS_COMM6 = 0;

	vu8 horizontalbars_tile_data_8[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
	};

	vu16 horizontalbars_pal[2] = {
		0x7FFF,0x0000
	};

	vu16 horizontalbars_alt_pal[2] = {
		0x0000,0x7FFF
	};

	marsVDP256Start();

	loadPalette(&horizontalbars_pal[0], &horizontalbars_pal[2], 0);

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

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(horizontalbars_tile_data_8,a,i,8,8,0,0);
					}
				}	
			break;
				
			case 2:
				switch (palette) {
					case 1:
						loadPalette(&horizontalbars_alt_pal[0], &horizontalbars_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&horizontalbars_pal[0], &horizontalbars_pal[2], 0);
					break;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (test == 1)
			{
				test = 2;
			}		
			else {
				test = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_RIGHT)
		{
			manualtest++;
	
			if(manualtest > 2){
		 		manualtest = 1;
			}

			switch (manualtest) {
			case 1:
				test = 1;
				loadPalette(&horizontalbars_pal[0], &horizontalbars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&horizontalbars_alt_pal[0], &horizontalbars_alt_pal[2], 0);
			break;
			}
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			manualtest++;
	
			if(manualtest > 2){
		 		manualtest = 1;
			}

			switch (manualtest) {
			case 1:
				test = 1;
				loadPalette(&horizontalbars_pal[0], &horizontalbars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&horizontalbars_alt_pal[0], &horizontalbars_alt_pal[2], 0);
			break;
		}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_STRIPES);
			marsVDP256Start();
			loadPalette(&horizontalbars_pal[0], &horizontalbars_pal[2], 0);
		}

		palette++;

		if(palette > 2){
			palette = 1;
		}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void vt_vertical_stripes()
{
	u16 done = 0;
	int frameDelay = 1;
	int test = 1;
	int manualtest = 1;
	int palette = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	MARS_SYS_COMM6 = 0;

	vu8 verticalbars_tile_data_8[] = {
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
	};

	vu16 verticalbars_pal[2] = {
		0x7FFF,0x0000
	};

	vu16 verticalbars_alt_pal[2] = {
		0x0000,0x7FFF
	};

	marsVDP256Start();

	loadPalette(&verticalbars_pal[0], &verticalbars_pal[2], 0);

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

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(verticalbars_tile_data_8,a,i,8,8,0,0);
					}
				}	
			break;
				
			case 2:
				switch (palette) {
					case 1:
						loadPalette(&verticalbars_alt_pal[0], &verticalbars_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&verticalbars_pal[0], &verticalbars_pal[2], 0);
					break;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (test == 1)
			{
				test = 2;
			}		
			else {
				test = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_RIGHT)
		{

			manualtest++;
	
			if(manualtest > 2){
		 		manualtest = 1;
			}

			switch (manualtest) {
			case 1:
				test = 1;
				loadPalette(&verticalbars_pal[0], &verticalbars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&verticalbars_alt_pal[0], &verticalbars_alt_pal[2], 0);
			break;
			}
		}

		if (pressedButton & SEGA_CTRL_LEFT)
		{
			manualtest++;
	
			if(manualtest > 2){
		 		manualtest = 1;
			}

			switch (manualtest) {
			case 1:
				test = 1;
				loadPalette(&verticalbars_pal[0], &verticalbars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&verticalbars_alt_pal[0], &verticalbars_alt_pal[2], 0);
			break;
		}
		}

		palette++;

		if(palette > 2){
			palette = 1;
		}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
	return;
}

void vt_checkerboard()
{
	u16 done = 0;
	int test = 1;
	int palette = 1;
	int i;
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;

	vu8 checkerboard_tile_data_8[] = {
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00
	};

	marsVDP256Start();

	vu16 checkerboard_pal[2] = {
		0x7FFF,0x0000
	};

	vu16 checkerboard_alt_pal[2] = {
		0x0000,0x7FFF
	};

	loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);

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

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		for (int i=0; i<=224; i=i+8){
			for (int a=0; a<=320; a=a+8){
				drawSprite(checkerboard_tile_data_8,a,i,8,8,0,0);
			}
		}

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(checkerboard_tile_data_8,a,i,8,8,0,0);
					}
				}	
			break;
				
			case 2:

				switch (palette) {
					case 1:
						loadPalette(&checkerboard_alt_pal[0], &checkerboard_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);
					break;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (test == 1)
			{
				test = 2;
			}		
			else {
				test = 1;
			}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CHECK);
			marsVDP256Start();
			loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);
		}

		palette++;

		if(palette > 2){
			palette = 1;
		}

		MARS_SYS_COMM6 = 1;

		drawLineTable(4);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;
	}
	return;
}

void vt_backlitzone_test()
{
	int done = 0;
	int frameDelay = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu8 background_color;
	int x = 160;
	int y = 112;
	int blockColor_1 = 1;
	int backgroundColor_2 = 2;
	int block = 2;
	vu16 *cram16 = &MARS_CRAM;
	MARS_SYS_COMM6 = 0;

	const vu8 block_0x0_tile[] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	const vu8 block_1x1_tile[] = {
		0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	const vu8 block_2x2_tile[] = {
		0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	const vu8 block_4x4_tile[] = {
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	const vu8 block_6x6_tile[] = {
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	const vu8 block_8x8_tile[] = {
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
	};

	Hw32xSetFGColor(blockColor_1,31,31,31);
	Hw32xSetFGColor(backgroundColor_2,0,0,0);
	
	vu8 blockColor[8] = {blockColor_1,blockColor_1,blockColor_1,blockColor_1,blockColor_1,blockColor_1,blockColor_1,blockColor_1};
	vu8 backgroundColor[8] = {backgroundColor_2,backgroundColor_2,backgroundColor_2,backgroundColor_2,backgroundColor_2,backgroundColor_2,backgroundColor_2,backgroundColor_2};

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
			block++;
			if(block > 5)
			block = 1;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			drawSprite(block_0x0_tile,x,y,8,8,0,0);
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_LED);
			Hw32xSetFGColor(blockColor_1,31,31,31);
			Hw32xSetFGColor(backgroundColor_2,0,0,0);
		}

		if (button & SEGA_CTRL_UP)
		{
			y--;
			if(y < 1)
				y = 1;
		}

		if (button & SEGA_CTRL_DOWN)
		{
			y++;
			if(y > 222)
				y = 222;
		}

		if (button & SEGA_CTRL_LEFT)
		{
			x--;
			if(x < 1)
				x = 1;
		}

		if (button & SEGA_CTRL_RIGHT)
		{
			x++;
			if(x > 320)
				x = 320;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		drawFillRect(0,0,320,224,(vu8*)&backgroundColor);

		switch (block)
		{
			case 1:
				drawSprite(block_1x1_tile,x,y,8,8,0,0);
			break;
			case 2:
				drawSprite(block_2x2_tile,x,y,8,8,0,0);
			break;
			case 3:
				drawSprite(block_4x4_tile,x,y,8,8,0,0);
			break;
			case 4:
				drawSprite(block_6x6_tile,x,y,8,8,0,0);
			break;
			case 5:
				drawSprite(block_8x8_tile,x,y,8,8,0,0);
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

void at_sound_test()
{
	int done = 0;
	int frameDelay = 5;
	int curse = 2;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const vu16 BACKGROUND_PALETTE_DATA[];
	extern const u8 BACKGROUND_TILE[];
	vu16 *cram16 = &MARS_CRAM;
	u16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	sound_t JUMP;
	sound_t BEEP;
	MARS_SYS_COMM6 = 0;

	Hw32xAudioInit();

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_PALETTE_DATA[i] & 0x7FFF;
	}

	cram16[205] = COLOR(31, 31, 31); // 204 is White
	cram16[206] = COLOR(31, 0, 0);   // 205 is Red
	cram16[207] = COLOR(0, 31, 0);   // 205 is Green
	cram16[208] = COLOR(5, 5, 5);    // 206 is Gray
	cram16[209] = COLOR(0, 0, 0);    // 206 is Black

	int fontColorWhite = 204;
	int fontColorRed = 205;
	int fontColorGreen = 206;
	int fontColorGray = 207;
	int fontColorBlack = 208;
	
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

    	if (pressedButton & SEGA_CTRL_RIGHT)
		{
			curse++;
		 	if(curse > 3)
		 		curse = 1;
		 }

		if (pressedButton & SEGA_CTRL_LEFT)
		{
		 	curse--;
		 	if(curse < 1)
		 		curse = 3;
		}

		if (pressedButton & SEGA_CTRL_B)
		{
		 	done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			switch (curse) 
			{
				case 1:
					Hw32xAudioLoad(&JUMP, "jump");
					Hw32xAudioPlay(&JUMP, 1, 1);  // Left Channel Only
					Hw32xAudioFree(&JUMP);
				break;
					
				case 2:
					Hw32xAudioLoad(&JUMP, "jump");
					Hw32xAudioPlay(&JUMP, 1, 3);  // Center
					Hw32xAudioFree(&JUMP);
				break;

				case 3:
					Hw32xAudioLoad(&JUMP, "jump");
					Hw32xAudioPlay(&JUMP, 1, 2);  // Right Channel Only
					Hw32xAudioFree(&JUMP);
				break;
			}
		}
		
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);

		mars_drawTextwShadow("Sound Test", 54, 58, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Left Channel", -15, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Center Channel", 40, 100, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Right Channel", 110, 80, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB; 

		Hw32xDelay(frameDelay);
	}
    return;
}

void ht_controller_test()
{
	int done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 button2, pressedButton2, oldButton2 = 0xFFFF;
	extern const vu16 BACKGROUND_PALETTE_DATA[];
	extern const u8 BACKGROUND_TILE[];
	vu16 *cram16 = &MARS_CRAM;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	MARS_SYS_COMM6 = 0;

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_PALETTE_DATA[i] & 0x7FFF;
	}

	currentFB = MARS_VDP_FBCTL & MARS_VDP_FS; 
	
	MARS_SYS_COMM6 = MASTER_STATUS_OK;
	
	while (!done) 
	{
		button = MARS_SYS_COMM8;

		button2 = MARS_SYS_COMM10;

		//if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE){
			//button = MARS_SYS_COMM10; // If controller 1 isn't detected, try using controller 2
		//}

		pressedButton = button & ~oldButton;
    	oldButton = button;

		pressedButton2 = button2 & ~oldButton2;
    	oldButton2 = button2;

    	if (pressedButton & SEGA_CTRL_START && pressedButton & SEGA_CTRL_LEFT)
		{
		 	done = 1;
		}
		
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) != currentFB) {}

		while (MARS_SYS_COMM6 == SLAVE_LOCK);
		MARS_SYS_COMM6 = 4;

		memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);

		cram16[205] = COLOR(31, 31, 31); // 204 is White
		cram16[206] = COLOR(31, 0, 0);   // 205 is Red
		cram16[207] = COLOR(0, 31, 0);   // 205 is Green
		cram16[208] = COLOR(5, 5, 5);    // 206 is Gray
		cram16[209] = COLOR(0, 0, 0);    // 206 is Black

		int fontColorWhite = 204;
		int fontColorRed = 205;
		int fontColorGreen = 206;
		int fontColorGray = 207;
		int fontColorBlack = 208;

		mars_drawTextwShadow("Controller Test", 30, 58, fontColorGreen, fontColorGray);

		// Controller 1
		mars_drawTextwShadow("Up", 15, 80, pressedButton & SEGA_CTRL_UP ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_UP ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Left", -15, 90, pressedButton & SEGA_CTRL_LEFT ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_LEFT ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Right", 30, 90, pressedButton & SEGA_CTRL_RIGHT ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_RIGHT ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Down", 10, 100, pressedButton & SEGA_CTRL_DOWN ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_DOWN ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Start", 90, 90, pressedButton & SEGA_CTRL_START ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_START ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("M", 210, 72, pressedButton & SEGA_CTRL_MODE ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_MODE ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("X", 154, 80, pressedButton & SEGA_CTRL_X ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_X ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Y", 174, 80, pressedButton & SEGA_CTRL_Y ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_Y ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Z", 194, 80, pressedButton & SEGA_CTRL_Z ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_Z ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("A", 154, 100, pressedButton & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_A ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("B", 174, 100, pressedButton & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_B ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("C", 194, 100, pressedButton & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton & SEGA_CTRL_C ? fontColorBlack : fontColorGray);

		// Controller 2
		mars_drawTextwShadow("Up", 15, 130, pressedButton2 & SEGA_CTRL_UP ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_UP ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Left", -15, 140, pressedButton2 & SEGA_CTRL_LEFT ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_LEFT ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Right", 30, 140, pressedButton2 & SEGA_CTRL_RIGHT ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_RIGHT ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Down", 10, 150, pressedButton2 & SEGA_CTRL_DOWN ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_DOWN ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Start", 90, 140, pressedButton2 & SEGA_CTRL_START ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_START ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("M", 210, 122, pressedButton2 & SEGA_CTRL_MODE ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_MODE ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("X", 154, 130, pressedButton2 & SEGA_CTRL_X ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_X ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Y", 174, 130, pressedButton2 & SEGA_CTRL_Y ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_Y ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Z", 194, 130, pressedButton2 & SEGA_CTRL_Z ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_Z ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("A", 154, 150, pressedButton2 & SEGA_CTRL_A ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_A ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("B", 174, 150, pressedButton2 & SEGA_CTRL_B ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_B ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("C", 194, 150, pressedButton2 & SEGA_CTRL_C ? fontColorRed : fontColorWhite, pressedButton2 & SEGA_CTRL_C ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Use START+LEFT to exit", 0, 193, fontColorGreen, fontColorGray);

		MARS_SYS_COMM6 = 1;

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		Hw32xDelay(frameDelay);
	}
    return;
}