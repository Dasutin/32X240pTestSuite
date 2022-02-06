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

#include <limits.h>
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

extern u32 schecksum;

typedef struct bios_data {
    u32 crc;
    char *name;
} BIOSID;

static const BIOSID bioslist[] = {
{ 0xDD9C46B8, "32X Master SH2 1.0" },	// 32X_M_BIOS.bin
{ 0xBFDA1FE5, "32X Slave SH2 1.0" },	// 32X_S_BIOS.bin
{ 0, NULL } } ;

int Detect32XMBIOS(u32 address)
{
    u8 *bios;
	
    bios = (u8 *)0+address;
    if (memcmp1(bios + 0x47E, "MARS", 4))
    	return 0;
    return 1;
}

int Detect32XSBIOS(u32 address)
{
    u8 *bios;
	
    bios = (u8 *)0+address;
    //if (memcmp1(bios + 0x1D4, "M", 1))
    //	return 0;
    return 1;
}

char *GetBIOSNamebyCRC(u32 checksum)
{
	int i = 0;
	
	while(bioslist[i].crc != 0)
	{		
		if(checksum == bioslist[i].crc)
			return bioslist[i].name;
		i++;
	}
	return;
}

void doMBIOSID(u32 checksum, u32 address)
{
	char 		*name = NULL;
	
	name = GetBIOSNamebyCRC(checksum);
	if(name)
	{
		HwMdPuts(name, 0x2000, 11, 20);
		return;
	}

	if(Detect32XMBIOS(address))
	{
		u32	   mchecksum = 0;

		mchecksum = CalculateCRC(address, 0x0000800);

		ShowMessageAndData("32X M BIOS CRC32:", mchecksum, 0x4000, 8, 6, 18);
	}
	
	// No match! check if we find the SEGA string and report
	if(Detect32XMBIOS(address))
	{
		HwMdPuts("Unknown BIOS, please report CRC", 0x4000, 4, 19);
	}
	else
	{
		HwMdPuts("BIOS not recognized", 0x4000, 8, 19);
	}
	return;
}

void doSBIOSID(u32 checksum, u32 address)
{
	char 		*name = NULL;
	
	name = GetBIOSNamebyCRC(checksum);
	if(name)
	{
		HwMdPuts(name, 0x2000, 11, 21);
		return;
	}

	if(Detect32XSBIOS(address))
	{
		//u32	   schecksum = 0;

		//schecksum = CalculateCRC(address, 0x0000400);

		//ShowMessageAndData("32X S BIOS CRC32:", schecksum, 0x4000, 8, 6, 20);
	}
	
	// No match! check if we find the SEGA string and report
	if(Detect32XSBIOS(address))
	{
		HwMdPuts("Unknown BIOS, please report CRC", 0x4000, 4, 21);
	}
	else
	{
		HwMdPuts("BIOS not recognized", 0x4000, 8, 21);
	}
	return;
}

void ShowMessageAndData(char *message, u32 address, u8 color, int len, int xpos, int ypos)
{
	int			msglen = 0;
	char		buffer[40];
	
	intToHex(address, buffer, len);
	
	msglen = strlen(message);
	//mars_drawTextwShadow(message, xpos, ypos, fontColorGreen, fontColorGray);
	HwMdPuts(message, color, xpos, ypos);
	//mars_drawTextwShadow(" 0x", xpos+msglen, ypos, fontColorGreen, fontColorGray);
	HwMdPuts(" 0x", color, xpos+msglen, ypos);
	//mars_drawTextwShadow(buffer, xpos+msglen+3, ypos, fontColorGreen, fontColorGray);
	HwMdPuts(buffer, color, xpos+msglen+3, ypos);
}


void vt_drop_shadow_test()
{
	int done = 0;
	int frameDelay = 1;
	int x = 10;
	int y = 10;
	int bee_mirror = 1; // Start right
	int frameCount = 0;
	int evenFrames = 0;
	extern const u8 BUZZ_TILE[] __attribute__((aligned(16)));
	extern const u8 BUZZ_SHADOW_TILE[] __attribute__((aligned(16)));
	extern const u8 MARKER_SHADOW_TILE[] __attribute__((aligned(16)));
	int changeSprite = 0;
	int background = 1;
	extern const vu16 TEST_PAL[];
	extern const vu16 DONNA_PAL[];
	extern const u8 DONNA_TILE[] __attribute__((aligned(16)));
	extern const u8 H_STRIPES_SHADOW_TILE[] __attribute__((aligned(16)));
	extern const u8 CHECKERBOARD_SHADOW_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	for (int i = 0; i < 227; i++){
		cram16[i] = DONNA_PAL[i] & 0x7FFF;
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
				cram16[i] = DONNA_PAL[i] & 0x7FFF;
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
			screenFadeOut(1);
			done = 1;
		}

		switch (background) {
				case 1:
					drawBG(DONNA_TILE);
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
		drawSprite(BUZZ_SHADOW_TILE,x,y,32,32,bee_mirror,0);
		}
		drawSprite(BUZZ_TILE,x-20,y-20,32,32,bee_mirror,0);
		}
		else {
		if (frameCount % 2 == evenFrames ) {
		drawSprite(MARKER_SHADOW_TILE,x,y,32,32,0,0);
		}
		}
		
		frameCount++;

		drawLineTable(4);

		Hw32xScreenFlip(0);
		
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
	extern const u8 MARKER_STRIPED_TILE[] __attribute__((aligned(16)));
	extern const u8 H_STRIPES_SHADOW_TILE[] __attribute__((aligned(16)));
	extern const u8 CHECKERBOARD_SHADOW_TILE[] __attribute__((aligned(16)));
	extern const vu16 TEST_PAL[];
	extern const vu16 DONNA_PAL[];
	extern const u8 DONNA_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	for (int i = 0; i < 227; i++){
		cram16[i] = DONNA_PAL[i] & 0x7FFF;
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
				cram16[i] = DONNA_PAL[i] & 0x7FFF;
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
			screenFadeOut(1);
			done = 1;
		}

		switch (background) 
		{
			case 1:
				drawBG(DONNA_TILE);
			break;
				
			case 2:
				drawBG(CHECKERBOARD_SHADOW_TILE);
			break;

			case 3:
				drawBG(H_STRIPES_SHADOW_TILE);
			break;
		}

		drawSprite(MARKER_STRIPED_TILE,x,y,32,32,0,0);

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_horizontal_stripes()
{
	u16 done = 0;
	int test = 1;
	int manualtest = 1;
	int palette = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;

	u8 h_bars_tile_8[] __attribute__((aligned(16))) = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
	};

	vu16 h_bars_pal[2] = {
		0x7FFF,0x0000
	};

	vu16 h_bars_alt_pal[2] = {
		0x0000,0x7FFF
	};

	marsVDP256Start();

	loadPalette(&h_bars_pal[0], &h_bars_pal[2], 0);

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

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(h_bars_tile_8,a,i,8,8,0,0);
					}
				}	
			break;
				
			case 2:
				switch (palette) {
					case 1:
						loadPalette(&h_bars_alt_pal[0], &h_bars_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&h_bars_pal[0], &h_bars_pal[2], 0);
					break;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
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
				loadPalette(&h_bars_pal[0], &h_bars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&h_bars_alt_pal[0], &h_bars_alt_pal[2], 0);
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
				loadPalette(&h_bars_pal[0], &h_bars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&h_bars_alt_pal[0], &h_bars_alt_pal[2], 0);
			break;
		}
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_STRIPES);
			loadPalette(&h_bars_pal[0], &h_bars_pal[2], 0);
		}

		palette++;

		if(palette > 2){
			palette = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_vertical_stripes()
{
	u16 done = 0;
	int test = 1;
	int manualtest = 1;
	int palette = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;

	u8 v_bars_tile_8[] __attribute__((aligned(16))) = {
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,
		0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01
	};

	vu16 v_bars_pal[2] = {
		0x7FFF,0x0000
	};

	vu16 v_bars_alt_pal[2] = {
		0x0000,0x7FFF
	};

	marsVDP256Start();

	loadPalette(&v_bars_pal[0], &v_bars_pal[2], 0);

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

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(v_bars_tile_8,a,i,8,8,0,0);
					}
				}	
			break;
				
			case 2:
				switch (palette) {
					case 1:
						loadPalette(&v_bars_alt_pal[0], &v_bars_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&v_bars_pal[0], &v_bars_pal[2], 0);
					break;
				}
			break;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
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
				loadPalette(&v_bars_pal[0], &v_bars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&v_bars_alt_pal[0], &v_bars_alt_pal[2], 0);
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
				loadPalette(&v_bars_pal[0], &v_bars_pal[2], 0);
			break;
				
			case 2:
				test = 1;
				loadPalette(&v_bars_alt_pal[0], &v_bars_alt_pal[2], 0);
			break;
		}
		}

		palette++;

		if(palette > 2){
			palette = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);

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

	u8 checkerboard_tile_8[] __attribute__((aligned(16))) = {
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

	const u16 checkerboard_pal[2] = {
		0x7FFF,0x0000
	};

	const u16 checkerboard_alt_pal[2] = {
		0x0000,0x7FFF
	};

	loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);

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

		for (int i=0; i<=224; i=i+8){
			for (int a=0; a<=320; a=a+8){
				drawSprite(checkerboard_tile_8,a,i,8,8,0,0);
			}
		}

		switch (test) {
			case 1:
				for (int i=0; i<=224; i=i+8){
					for (int a=0; a<=320; a=a+8){
						drawSprite(checkerboard_tile_8,a,i,8,8,0,0);
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
			screenFadeOut(1);
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

		drawLineTable(4);

		Hw32xScreenFlip(0);
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

	u8 block_0x0_tile[] __attribute__((aligned(16))) = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	u8 block_1x1_tile[] __attribute__((aligned(16))) = {
		0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	u8 block_2x2_tile[] __attribute__((aligned(16))) = {
		0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
		0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	u8 block_4x4_tile[] __attribute__((aligned(16))) = {
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	u8 block_6x6_tile[] __attribute__((aligned(16))) = {
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	u8 block_8x8_tile[] __attribute__((aligned(16))) = {
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
			screenFadeOut(1);
			done = 1;
		}

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

		drawLineTable(4);

		Hw32xScreenFlip(0);

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
	extern const vu16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	sound_t JUMP;
	sound_t BEEP;

	Hw32xAudioInit();

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_PAL[i] & 0x7FFF;
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

	Hw32xScreenFlip(0);
	
	while (!done)
	{
		Hw32xFlipWait();

		memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);

		mars_drawTextwShadow("Sound Test", 54, 58, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Left Channel", -15, 80, curse == 1 ? fontColorRed : fontColorWhite, curse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Center Channel", 40, 100, curse == 2 ? fontColorRed : fontColorWhite, curse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Right Channel", 110, 80, curse == 3 ? fontColorRed : fontColorWhite, curse == 3 ? fontColorBlack : fontColorGray);

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
			screenFadeOut(1);
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

void ht_controller_test()
{
	int done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 button2, pressedButton2, oldButton2 = 0xFFFF;
	extern const vu16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_PAL[i] & 0x7FFF;
	}

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

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
			screenFadeOut(1);
		 	done = 1;
		}

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

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
    return;
}

#define	MAX_LOCATIONS	9

void ht_memory_viewer(u32 address)
{
	int done = 0;
	int frameDelay = 0;
	int redraw = 1, docrc = 0, locpos = 1, i = 0;
	u32	crc = 0, locations[MAX_LOCATIONS] = { 0, 0x0004000, 0x0004100, 0x0004200, 0x0004400, 0x2000000, 0x4000000, 0x4020000, 0x6000000 };
	u16 button, pressedButton, oldButton = 0xFFFF;

	// Clear the 32X CRAM
	for(i = 0; i < 255; i++)
	{
		Hw32xSetBGColor(i,0,0,0);
	}

	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	for(i = 0; i < MAX_LOCATIONS; i++)
	{
		if(locations[i] == address)
		{
			locpos = i;
			break;
		}
	}

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		if(redraw)
		{
			int 	i = 0, j = 0;
			u8     *mem = NULL;
			char 	buffer[10];

			mem = (u8*)address;

			if(docrc)
				crc = CalculateCRC(address, 0x1C0);

			intToHex(address, buffer, 8);
			HwMdPuts(buffer, 0x4000, 32, 0);
			intToHex(address+448, buffer, 8);
			HwMdPuts(buffer, 0x4000, 32, 27);

			if(docrc)
			{
				intToHex(crc, buffer, 8);
				HwMdPuts(buffer, 0x2000, 32, 14);
			}

			for(i = 0; i < 28; i++)
			{
				for(j = 0; j < 16; j++)
				{
					intToHex(mem[i*16+j], buffer, 2);
					HwMdPuts(buffer, 0x0000, j*2, i);
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
		{
		 	done = 1;
		}

		if(pressedButton & SEGA_CTRL_A)
		{
			docrc = !docrc;
			HwMdClearScreen();
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_B)
		{
			locpos ++;
			if(locpos == MAX_LOCATIONS)
				locpos = 0;
			address = locations[locpos];
			redraw = 1;
		}

		if (pressedButton & SEGA_CTRL_Z)
		{
			HwMdClearScreen();
			DrawHelp(HELP_MEMVIEW);
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_LEFT)
		{
			if(address >= 0x1C0)
				address -= 0x1C0;
			else
				address = 0;

			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_RIGHT)
		{
			address += 0x1C0;	
			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_UP)
		{
			if(address >= 0x10000)
				address -= 0x10000;
			else
				address = 0;

			redraw = 1;
		}

		if(pressedButton & SEGA_CTRL_DOWN)
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
	u8      *bios = NULL;
	char	buffer[50];
	int		i = 0, j = 0, data[] = { 16, 16, 17, 15, 16, 48, 14, -2, 16, -4, -4, -4, -4, -2, -2, -4, -4, -12, -40, 16, 0};
	

	bios = (u8*)(address+0x047E);

	while(data[i] != 0)
	{
		if(data[i] > 0)
		{
			memcpy(buffer, bios, sizeof(u8)*data[i]);
			buffer[data[i]] = '\0';
			HwMdPuts(buffer, 0x0000, 12, 8+j);
			j++;
		}
		bios += data[i] > 0 ? data[i] : -1*data[i];
		i++;
	}
}

void PrintSBIOSInfo(u32 saddress)
{
	u8      *sbios = NULL;
	char	sbuffer[50];
	int		i = 0, j = 0, sdata[] = { 16, 16, 17, 15, 16, 48, 14, -2, 16, -4, -4, -4, -4, -2, -2, -4, -4, -12, -40, 16, 0};
	

	sbios = (u8*)(saddress+0x047E);

	while(sdata[i] != 0)
	{
		if(sdata[i] > 0)
		{
			memcpy(sbuffer, sbios, sizeof(u8)*sdata[i]);
			sbuffer[sdata[i]] = '\0';
			HwMdPuts(sbuffer, 0x0000, 12, 8+j);
			j++;
		}
		sbios += sdata[i] > 0 ? sdata[i] : -1*sdata[i];
		i++;
	}
}

void ht_check_32x_bios_crc(u32 address)
{
	int done = 0;
	int frameDelay = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	extern const vu16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	u32	checksum = 0;
	int redraw = 1;

	paused = UNPAUSED;

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);

		ShowMessageAndData("Sega 32X BIOS Data at", address, 0x0000, 8, 4, 4);
		PrintBIOSInfo(address);

		checksum = CalculateCRC(address, 0x0000800);

		////mars_drawTextwShadow(checksum, 0, 193, fontColorGreen, fontColorGray);

		doMBIOSID(checksum, address);

		////ShowMessageAndData("32X S BIOS CRC32:", schecksum, 0x4000, 8, 6, 22);
		//ShowMessageAndData("", schecksum, 0x4000, 8, 0, 193);

		////HwMdPuts(schecksum, 0x4000, 11, 21);

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
    	oldButton = button;

		////while (MARS_SYS_COMM6 == SLAVE_LOCK);
		////MARS_SYS_COMM6 = 4;

		////MARS_SYS_COMM6 = 1;

		if (pressedButton & SEGA_CTRL_START)
		{
		 	done = 1;
		}

		if(pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
	paused = PAUSED;
	paused = PAUSED;
    return;
}

void Set32XRegion(u8 value, u32 startaddress, u32 size)
{
	u8		*ram = NULL;
	u32		address = 0;
	
	ram = (u8*)startaddress;
	for(address = 0; address < size; address++)
		ram[address] = value;
}

int Read32XRegion(u8 value, u32 startaddress, u32 size)
{
	u8		*ram = NULL;
	u32		address = 0;
	
	ram = (u8*)startaddress;
	for(address = 0; address < size; address++)
	{
		if(ram[address] != value)
			return address;
	}
	
	return MEMORY_OK;
}

int Check32XRegion(u8 value, u32 startaddress, u32 size)
{
	Set32XRegion(value, startaddress, size);
	
	return(Read32XRegion(value, startaddress, size));
}

int Check32XRAM(void *start, u32 size)
{
   vu32 *sdram = start;
   while(size--)
   {
      u16 result, value;
      value = *sdram;
      *sdram ^= UCHAR_MAX;
      result = value ^ *sdram;
      *sdram++ = value;
      if (result != UCHAR_MAX)
      {
      	HwMdPuts("FAILED", 0x4000, 12, 10);
        return 0;
      }
   }
   HwMdPuts("ALL OK", 0x2000, 12, 10);
   return 1;
}

int Check32XRAMWithValue(char * message, u32 start, u32 end, u8 value, int pos)
{
	int memoryFail = 0;
	
	HwMdPuts(message, 0x0000, 12, pos);
	
	memoryFail = Check32XRegion(value, start, end - start);
	
	if(memoryFail != MEMORY_OK)
	{
		ShowMessageAndData("FAILED", memoryFail, 6, 0x4000, 12, pos+1);
		return 0;
	}
	
	HwMdPuts("ALL OK", 0x2000, 16, pos+1);
	return 1;
}

void ht_test_32x_sdram()
{
	int done = 0;
	int frameDelay = 5;
	int redraw = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	extern const vu16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
    	oldButton = button;

		memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);

		//ShowMessageAndData("32X SDRAM", 0x6000000, 15, 0x0000, 7, 7);
		//if(redraw)
		//{
		//Check32XRAM(0x06010000, 0x0001FFFF);
		//Check32XRAMWithValue("Setting to 0x00", 0x06002000, 0x0603FFDF, 0x00, 10);
		//Check32XRAMWithValue("Setting to 0xFF", 0x06002000, 0x0603FFDF, 0xFF, 12);
		//Check32XRAMWithValue("Setting to 0x55", 0x06002000, 0x0603FFDF, 0x55, 14);
		//Check32XRAMWithValue("Setting to 0xAA", 0x06002000, 0x0603FFDF, 0xAA, 16);
		//redraw = 0;
		//}
		
		//while (MARS_SYS_COMM6 == SLAVE_LOCK);
		//MARS_SYS_COMM6 = 4;

		if(redraw)
		{
		Check32XRAM(0x6000000, 0xFF);
		//Check32XRAMWithValue("Setting to 0x00", 0x06002000, 0x0603FFDF, 0x00, 10);
		//Check32XRAMWithValue("Setting to 0xFF", 0x06002000, 0x0603FFDF, 0xFF, 12);
		//Check32XRAMWithValue("Setting to 0x55", 0x06002000, 0x0603FFDF, 0x55, 14);
		//Check32XRAMWithValue("Setting to 0xAA", 0x06002000, 0x0603FFDF, 0xAA, 16);
		redraw = 0;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
		 	done = 1;
		}

		if(pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		Hw32xScreenFlip(0);

		Hw32xDelay(frameDelay);
	}
    return;
}