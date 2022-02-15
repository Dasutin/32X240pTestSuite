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

//TODO Move CRC and BIOS support functions
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
		HwMdPuts(name, 0x4000, 11, 20);
		return;
	}

	if(Detect32XMBIOS(address))
	{
		u32	   mchecksum = 0;

		mchecksum = CalculateCRC(address, 0x0000800);

		ShowMessageAndData("32X M BIOS CRC32:", mchecksum, 0x2000, 8, 6, 18);
	}
	
	// No match! check if we find the SEGA string and report
	if(Detect32XMBIOS(address))
	{
		HwMdPuts("Unknown BIOS, please report CRC", 0x2000, 4, 19);
	}
	else
	{
		HwMdPuts("BIOS not recognized", 0x2000, 8, 19);
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
	HwMdPuts(message, color, xpos, ypos);
	HwMdPuts(" 0x", color, xpos+msglen, ypos);
	HwMdPuts(buffer, color, xpos+msglen+3, ypos);
}

void MDPSG_init()
{
	HwMdPSGSetTone(0,0);
	HwMdPSGSetTone(1,0);
	HwMdPSGSetTone(2,0);
	HwMdPSGSetTone(3,0);

	HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(1, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(2, PSG_ENVELOPE_MIN);
	HwMdPSGSetEnvelope(3, PSG_ENVELOPE_MIN);
}

void MDPSG_stop()
{
	//HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
	HwMdPSGSetChandVol(0,15);
	HwMdPSGSetChandVol(1,15);
	HwMdPSGSetChandVol(2,15);
	HwMdPSGSetChandVol(3,15);
	//HwMdPSGSetEnvelope(1, PSG_ENVELOPE_MIN);
	//HwMdPSGSetEnvelope(2, PSG_ENVELOPE_MIN);
	//HwMdPSGSetEnvelope(3, PSG_ENVELOPE_MIN);
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
	//extern const u16 TEST_PAL[];
	extern const u16 DONNA_PAL[];
	extern const u8 DONNA_TILE[] __attribute__((aligned(16)));
	extern const u8 H_STRIPES_SHADOW_TILE[] __attribute__((aligned(16)));
	extern const u8 CHECKERBOARD_SHADOW_TILE[] __attribute__((aligned(16)));
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	loadPalette(&DONNA_PAL[0], &DONNA_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done)
	{
		Hw32xFlipWait();
		//clearScreen_Fill8bit();

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
			loadPalette(&DONNA_PAL[0], &DONNA_PAL[255],0);
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
	//extern const u16 TEST_PAL[];
	extern const u16 DONNA_PAL[];
	extern const u8 DONNA_TILE[] __attribute__((aligned(16)));
	u16 button = 0, pressedButton = 0, oldButton = 0xFFFF;

	loadPalette(&DONNA_PAL[0], &DONNA_PAL[255],0);

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
			loadPalette(&DONNA_PAL[0], &DONNA_PAL[255],0);
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

void vt_reflex_test()
{
	char str[10];
	u16 pal = 0x0000, change = 1, loadvram = 1;
	s16 speed = 1, vary = 0;
	u16 size, ind = 0, usersound = 0;
	u16 x = 0, y = 0, x2 = 0, y2 = 0, done = 0, variation = 1, draw = 1, done1 = 1;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 pos = 0, view = 0, audio = 1, drawoffset = 0;
	u16 first_pal[16], oldColor = 0;
	s16 clicks[10];
	vu16 *cram16 = &MARS_CRAM;
	int blockColor_1 = 1;
	int backgroundColor_2 = 2;
	//extern const u8 MARKER_TILE[] __attribute__((aligned(16)));
	extern const u8 MARKER_TILE1[] __attribute__((aligned(16)));
	extern const u8 MARKER_TILE2[] __attribute__((aligned(16)));
	extern const u16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));

	u8 MARKER_TILE3[] __attribute__((aligned(16))) = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 
	0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 
	0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 
	0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 
	0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 
	0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	x = 144;
	y = 60;

	x2 = 108;
	y2 = 96;
	
	MDPSG_init();
	HwMdPSGSetFrequency(0, 1000);

	// Set screen priority for the 32X 
	MARS_VDP_DISPMODE = MARS_VDP_PRIO_32X | MARS_224_LINES | MARS_VDP_MODE_256;

	Hw32xScreenFlip(0);

	cram16[1] = COLOR(31, 31, 31);
	cram16[2] = COLOR(31, 31, 31);

	while (!done)
	{
		Hw32xFlipWait();
		clearScreen_Fill8bit();

		if (loadvram)
		{	
			HwMdPuts("Press the \"A\" button when the sprite", 0x4000, 2, 21);
			HwMdPuts("is aligned. A negative value means", 0x4000, 2, 22);
			HwMdPuts("you pressed \"A\" before they intersect.", 0x4000, 2, 23);
			HwMdPuts("\"B\" button toggles horz/vert", 0x4000, 2, 24);
			HwMdPuts("\"C\" button toggles audio", 0x4000, 2, 25);
			HwMdPuts("DOWN toggles random/rhythmic", 0x4000, 2, 26);

			loadvram = 0;
			draw = 1;

			if (pos)
			{
				int i = 0;
				u16 ppos = 0;

				for(i = 0; i < pos; i++)
				{
					pal = 0x0000;

					intToStr(clicks[i], str, 1);

					if (clicks[i] == 0)
						pal = 0x4000;
					if (clicks[i] < 0)
						pal = 0x2000;

					ppos = i + 1;
					intToStr(ppos, str, 1);
					HwMdPuts("Offset", 0x0000, 2, i);
					HwMdPuts(str, 0x0000, 9, i);
					HwMdPuts(":", 0x0000, i == 9 ? 11 : 10, i);
					intToStr(clicks[i], str, 1);
					HwMdPuts(str, pal, i == 9 ? 13 : 12, i);
					ppos = strlen(str);
					if (clicks[i] == 1)
						HwMdPuts(" frame    ", pal, pos == 9 ? 13 : 12 + ppos, i);
					else
						HwMdPuts(" frames   ", pal, pos == 9 ? 13 : 12 + ppos, i);
				}
			}
		}
		
		if (y == 96)	//  Screen Flash    
		{
			cram16[0] = COLOR(5, 5, 5);
			cram16[0] = COLOR(0, 0, 0);
			if (audio && !usersound)
				HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
				
			//TODO Screen Flash Here

		}
		
		if (usersound)
		{
			HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);
			usersound = 0;
		}

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}
		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_MANUALLAG);
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (change)
			{
				clicks[pos] = (y - 96) * speed;
				drawoffset = 1;
				if (clicks[pos] >= 0)
					change = 0;
					
				if (audio)
				{
					if (clicks[pos] == 0)
					{
						//HwMdPSGSetChandVol(0, 0);
						HwMdPSGSetFrequency(0, 1000);
					}
					else {
						//HwMdPSGSetChandVol(0, 0);
						HwMdPSGSetFrequency(0, 500);
					}
						
					HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MAX);
					usersound = 1;
				}
			}
		}

		if (pressedButton & SEGA_CTRL_B)
		{
			view++;
			if(view > 2)
				view = 0;
		}

		if (pressedButton & SEGA_CTRL_C)
		{
			audio = !audio;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
			variation = !variation;
			if(!variation)
				vary = 0;
			draw = 1;
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			done = 1;
		}

		if (drawoffset)
		{
			u16 ppos = 0;

			pal = 0x0000;
			intToStr(clicks[pos], str, 1);

			if (clicks[pos] == 0)
				pal = 0x4000;
			if (clicks[pos] < 0)
				pal = 0x2000;

			ppos = pos + 1;
			intToStr(ppos, str, 1);
			HwMdPuts("Offset", 0x0000, 2, pos);
			HwMdPuts(str, 0x0000, 9, pos);
			HwMdPuts(":", 0x0000, pos == 9 ? 11 : 10, pos);
			intToStr(clicks[pos], str, 1);
			HwMdPuts(str, pal, pos == 9 ? 13 : 12, pos);
			ppos = strlen(str);

			if (clicks[pos] == 1)
				HwMdPuts(" frame    ", pal, pos == 9 ? 13 : 12 + ppos, pos);
			else
				HwMdPuts(" frames   ", pal, pos == 9 ? 13 : 12 + ppos, pos);

			if (clicks[pos] >= 0)
				pos++;

			if (pos > 9)
				done = 1;

			drawoffset = 0;
		}

		if (draw)
		{
			HwMdPuts("Audio:", 0x0000, 24, 0);

			if (audio)
				HwMdPuts("on ", 0x0000, 31, 0);
			else
				HwMdPuts("off", 0x0000, 31, 0);

			HwMdPuts("Timing:", 0x0000, 24, 1);

			if (variation)
				HwMdPuts("random  ", 0x0000, 32, 1);
			else
				HwMdPuts("rhythmic", 0x0000, 32, 1);

			draw = 0;
		}

		if (y > 132 + vary)
		{
			speed = -1;
			change = 1;
			if (variation)
			{
				if (random() % 2)
					vary = random() % 7;
				else
					vary = -1 * random() % 7;
			}
		}

		if (y < 60 + vary)
		{
			speed = 1;
			change = 1;

			if (variation)
			{
				if (random() % 2)
					vary = random() % 7;
				else
					vary = -1 * random() % 7;
			}
		}

		y += speed;
		x2 += speed;

		drawSprite(MARKER_TILE3, x, 96, 32, 32, 0, 0);
		drawSprite(MARKER_TILE1, x, y, 32, 32, 0, 0);

		if (y == 96)							// Red on the spot
			cram16[2] = COLOR(31, 0, 0);

		if (y == 95 || y == 97)				// Green one pixel before or after
			cram16[2] = COLOR(0, 31, 0);

		if (y == 98 || y == 94)				// Back to white two pixels before or after
			cram16[2] = COLOR(31, 31, 31);

		if (view == 0 || view == 2)
			drawSprite(MARKER_TILE1, x, y, 32, 32, 0, 0);
		else
			drawSprite(MARKER_TILE1, 320, 224, 32, 32, 0, 0);

		if (view == 1 || view == 2)
			drawSprite(MARKER_TILE2, x2, y2, 32, 32, 0, 0);
		else
			drawSprite(MARKER_TILE2, 320, 224, 32, 32, 0, 0);

		if (y == 96)						// Half the screen?        
		{
			if(audio)
			{
				HwMdPSGSetFrequency(0, 1000);
				HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MAX);
			}

			//TODO Screen Flash Here

		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}

	if (pos > 9)
	{	
		int totald = 0, cnt, tot;
		u16 total = 0, count = 0, c = 0;

		HwMdClearScreen();

		marsVDP256Start();

		Hw32xScreenFlip(0);

		Hw32xFlipWait();

		HwMdPSGSetEnvelope(0, PSG_ENVELOPE_MIN);

		for(c = 0; c < 10; c++)
		{
			if (clicks[c] != 0xFF)
			{
				intToStr(clicks[c], str, 1);

				pal = 0x0000;
				if (clicks[c] == 0)
					pal = 0x4000;

				if (clicks[c] < 0)
					pal = 0x2000;

				if (clicks[c] >= 0)
				{
					total += clicks[c];
					count++;
				}
			}
			HwMdPuts(str, pal, 10, c + 7);
		}

		HwMdPuts("+", 0x0000, 8, 11);

		if (count > 0)
		{
			u16 h = 10;
			u16 v = 18;
			const int framerate = MARS_VDP_DISPMODE & MARS_NTSC_FORMAT ? 22.801467 : 23.011360;
			
			HwMdPuts("----", 0x0000, h - 2, v++);

			cnt = intToFix32(count);
			tot = intToFix32(total);

			intToStr(total, str, 1);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			HwMdPuts("/", 0x0000, h++, v);

			intToStr(count, str, 1);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			HwMdPuts("=", 0x0000, h++, v);

			totald = fix32Div(tot, cnt);
			fix32ToStr(totald, str, 4);
			HwMdPuts(str, 0x0000, h, v);
			h += strlen(str);
			h -= 2;
			HwMdPuts("frames", 0x0000, h, v);
			HwMdPuts("These are your reflexes, not a", 0x0000, 5, ++v);
			HwMdPuts("lag test. A frame is", 0x0000, 5, ++v);
			fix32ToStr(framerate, str, 6);
			HwMdPuts(str, 0x0000, 26, v);
			HwMdPuts("ms.", 0x0000, 32, v);

			h = 14;
			v = 12;
			cnt = fix32Mul(totald, framerate);
			fix32ToStr(cnt, str, 2);
			HwMdPuts(str, 0x4000, h, v);
			h += strlen(str);
			HwMdPuts(" milliseconds", 0x0000, h, v);

			if (total < 5)
				HwMdPuts("EXCELLENT REFLEXES!", 0x4000, 14, 15);

			if (total == 0)
				HwMdPuts("INCREDIBLE REFLEXES!!", 0x4000, 14, 15);
		}

		loadPalette(&BACKGROUND_PAL[0], &BACKGROUND_PAL[255],0);
		drawBG(BACKGROUND_TILE);
			
		drawLineTable(4);
			
		Hw32xScreenFlip(0);
			
		done = 0;
			
		while (!done)
		{
			button = MARS_SYS_COMM8;

			if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
			{
				button = MARS_SYS_COMM10;
			}

			pressedButton = button & ~oldButton;
    		oldButton = button;
		
			if (pressedButton & SEGA_CTRL_START || pressedButton & SEGA_CTRL_B)
			{
				done = 1;
			}
		}
	}
}

void vt_horizontal_stripes()
{
	u16 done = 0;
	int test = 1;
	int manualtest = 1;
	int pal = 1;
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

	const u16 h_bars_pal[2] = {
		0x7FFF,0x0000
	};

	const u16 h_bars_alt_pal[2] = {
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
				switch (pal) {
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

		pal++;

		if(pal > 2){
			pal = 1;
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
	int pal = 1;
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

	const u16 v_bars_pal[2] = {
		0x7FFF,0x0000
	};

	const u16 v_bars_alt_pal[2] = {
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
				switch (pal) {
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

		pal++;

		if(pal > 2){
			pal = 1;
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
	int pal = 1;
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
				switch (pal) {
					case 1:
						loadPalette(&checkerboard_alt_pal[0], &checkerboard_alt_pal[2], 0);
					break;
				
					case 2:
						loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);
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

		if (pressedButton & SEGA_CTRL_Z)
		{
			DrawHelp(HELP_CHECK);
			marsVDP256Start();
			loadPalette(&checkerboard_pal[0], &checkerboard_pal[2], 0);
		}

		pal++;

		if(pal > 2){
			pal = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
	return;
}

void vt_backlitzone_test()
{
	int done = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	//vu8 background_color;
	int x = 160;
	int y = 112;
	int blockColor_1 = 1;
	int backgroundColor_2 = 2;
	int block = 2;

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

		//clearScreen_Fill8bit();

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
	}
	return;
}

void at_sound_test()
{
	int done = 0;
	//int frameDelay = 5;
	int xcurse = 2;
	int ycurse = 1;
	int psgoff = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	//sound_t JUMP;
	//sound_t BEEP;

	//Hw32xAudioInit();

	MDPSG_init();

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

		mars_drawTextwShadow("Sound Test", 54, 35, fontColorGreen, fontColorGray);

		mars_drawTextwShadow("32X PCM", 64, 66, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("Left", 20, 80, ycurse == 1 && xcurse == 1 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Center", 68, 80, ycurse == 1 && xcurse == 2 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("Right", 130, 80, ycurse == 1 && xcurse == 3 ? fontColorRed : fontColorWhite, ycurse == 1 && xcurse == 3 ? fontColorBlack : fontColorGray);

		mars_drawTextwShadow("Genesis PSG Channel", 18, 110, fontColorGreen, fontColorGray);
		mars_drawTextwShadow("0", 74, 124, ycurse == 2 && xcurse == 1 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 1 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("1", 84, 124, ycurse == 2 && xcurse == 2 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 2 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("2", 94, 124, ycurse == 2 && xcurse == 3 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 3 ? fontColorBlack : fontColorGray);
		mars_drawTextwShadow("3", 104, 124, ycurse == 2 && xcurse == 4 ? fontColorRed : fontColorWhite, ycurse == 2 && xcurse == 4 ? fontColorBlack : fontColorGray);

		button = MARS_SYS_COMM8;

		if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE)
		{
			button = MARS_SYS_COMM10;
		}

		pressedButton = button & ~oldButton;
    	oldButton = button;

    	if (pressedButton & SEGA_CTRL_RIGHT)
		{
			xcurse++;
		 	if(xcurse > 3 && ycurse == 1)
		 		xcurse = 1;
			else if (xcurse > 4 && ycurse == 2)
			{
				xcurse = 1;
			}
		 }

		if (pressedButton & SEGA_CTRL_LEFT)
		{
		 	xcurse--;
			if(xcurse < 1 && ycurse == 1)
		 		xcurse = 3;
			else if (xcurse < 1 && ycurse == 2)
			{
				xcurse = 4;
			}
		}

		if (pressedButton & SEGA_CTRL_UP)
		{
			ycurse++;
		 	if(ycurse > 2)
		 		ycurse = 1;
			else if (ycurse == 2 && xcurse > 4)
			{
				ycurse = 1;
				xcurse = 3;
			}
		}

		if (pressedButton & SEGA_CTRL_DOWN)
		{
		 	ycurse--;
		 	if(ycurse < 1)
		 		ycurse = 2;
			else if (ycurse == 2 && xcurse > 4)
			{
				ycurse = 1;
				xcurse = 3;
			}
		}

		if (pressedButton & SEGA_CTRL_START)
		{
			screenFadeOut(1);
		 	done = 1;
		}

		if (pressedButton & SEGA_CTRL_A)
		{
			if (xcurse == 1 && ycurse == 1)
			{
				//Hw32xAudioLoad(&JUMP, "jump");
				//Hw32xAudioPlay(&JUMP, 1, 1);  // Left Channel Only
				//Hw32xAudioFree(&JUMP);
			}
			if (xcurse == 2 && ycurse == 1)
			{
				//Hw32xAudioLoad(&JUMP, "jump");
				//Hw32xAudioPlay(&JUMP, 1, 3);  // Center
				//Hw32xAudioFree(&JUMP);
			}
			if (xcurse == 3 && ycurse == 1)
			{
				//Hw32xAudioLoad(&JUMP, "jump");
				//Hw32xAudioPlay(&JUMP, 1, 2);  // Right Channel Only
				//Hw32xAudioFree(&JUMP);
			}
			if (xcurse == 1 && ycurse == 2)
			{
				HwMdPSGSetChandVol(0, 0);
				HwMdPSGSetFrequency(0, 200);
				if (psgoff == 0)
					psgoff = 80;
			}
			if (xcurse == 2 && ycurse == 2)
			{
				HwMdPSGSetChandVol(1, 0);
				HwMdPSGSetFrequency(1, 2000);
				if (psgoff == 0)
					psgoff = 80;
			}
			if (xcurse == 3 && ycurse == 2)
			{
				HwMdPSGSetChandVol(2, 0);
				HwMdPSGSetFrequency(2, 4095);
				if (psgoff == 0)
					psgoff = 80;
			}
			if (xcurse == 4 && ycurse == 2)
			{
				HwMdPSGSetChandVol(3, 0);
				HwMdPSGSetNoise(PSG_NOISE_TYPE_WHITE, PSG_NOISE_FREQ_TONE3);
				HwMdPSGSetFrequency(3, 500);
				if (psgoff == 0)
					psgoff = 80;
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

		if(psgoff)
		{
			psgoff--;
			if(psgoff == 0)
				MDPSG_stop();
		}
	}
    return;
}

void ht_controller_test()
{
	int done = 0;
	int frameDelay = 5;
	u16 button, pressedButton, oldButton = 0xFFFF;
	u16 button2, pressedButton2, oldButton2 = 0xFFFF;
	extern const u16 BACKGROUND_PAL[];
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
			HwMdPuts(buffer, 0x2000, 32, 0);
			intToHex(address+448, buffer, 8);
			HwMdPuts(buffer, 0x2000, 32, 27);

			if(docrc)
			{
				intToHex(crc, buffer, 8);
				HwMdPuts(buffer, 0x4000, 32, 14);
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
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	u32	checksum = 0;

	loadPalette(&BACKGROUND_PAL[0], &BACKGROUND_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		drawBG(BACKGROUND_TILE);

		ShowMessageAndData("Sega 32X BIOS Data at", address, 0x0000, 8, 4, 4);
		PrintBIOSInfo(address);

		checksum = CalculateCRC(address, 0x0000800);

		doMBIOSID(checksum, address);

		////ShowMessageAndData("32X S BIOS CRC32:", schecksum, 0x4000, 8, 6, 22);
		//ShowMessageAndData("", schecksum, 0x4000, 8, 0, 193);

		////HwMdPuts(schecksum, 0x4000, 11, 21);

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
    	oldButton = button;

		if (pressedButton & SEGA_CTRL_START)
		{
		 	done = 1;
		}

		if(pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);
	}
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
      	HwMdPuts("FAILED", 0x2000, 12, 10);
        return 0;
      }
   }
   HwMdPuts("ALL OK", 0x4000, 12, 10);
   return 1;
}

int Check32XRAMWithValue(char * message, u32 start, u32 end, u8 value, int pos)
{
	int memoryFail = 0;
	
	HwMdPuts(message, 0x0000, 12, pos);
	
	memoryFail = Check32XRegion(value, start, end - start);
	
	if(memoryFail != MEMORY_OK)
	{
		ShowMessageAndData("FAILED", memoryFail, 6, 0x2000, 12, pos+1);
		return 0;
	}
	
	HwMdPuts("ALL OK", 0x4000, 16, pos+1);
	return 1;
}

void ht_test_32x_sdram()
{
	int done = 0;
	int draw = 0;
	int test = 0;
	u16 button, pressedButton, oldButton = 0xFFFF;
	extern const u16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));

	loadPalette(&BACKGROUND_PAL[0], &BACKGROUND_PAL[255],0);

	Hw32xScreenFlip(0);

	while (!done) 
	{
		Hw32xFlipWait();

		button = MARS_SYS_COMM8;

		pressedButton = button & ~oldButton;
    	oldButton = button;

		drawBG(BACKGROUND_TILE);

		ShowMessageAndData("32X SDRAM", 0x6000000, 0x0000, 7, 10, 4);

		if ((draw = 1))
		{
			switch (test)
			{
				case 1:
					Check32XRAMWithValue("Setting to 0x00", 0x06002000, 0x06030000, 0x00, 10);
					Hw32xSleep(1000);
				case 2:
					Check32XRAMWithValue("Setting to 0xFF", 0x06002000, 0x06030000, 0xFF, 12);
					Hw32xSleep(1000);
				case 3:
					Check32XRAMWithValue("Setting to 0x55", 0x06002000, 0x06030000, 0x55, 14);
					Hw32xSleep(500);
				case 4:
					Check32XRAMWithValue("Setting to 0xAA", 0x06002000, 0x06030000, 0xAA, 16);
				case 5:
					draw = 0;		
			}
		}

		if (pressedButton & SEGA_CTRL_START)
		{
		 	done = 1;
		}

		if(pressedButton & SEGA_CTRL_B)
		{
			done = 1;
		}

		drawLineTable(4);

		Hw32xScreenFlip(0);

		test++;
	}
    return;
}