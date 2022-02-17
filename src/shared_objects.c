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
#include "shared_objects.h"
#include "32x.h"
#include "hw_32x.h"
#include "main.h"
#include "help.h"
#include "patterns.h"
#include "32x_images.h"
#include "graphics.h"
#include "tests.h"

u8 paused = PAUSED;
u16 currentFB = 0;
vu16 overwriteImg16;
u32 _state = ~0L;
u16 randbase;

static const u32 crc32_table[] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
};

int fontColorWhite = 204;
int fontColorRed = 205;
int fontColorGreen = 206;
int fontColorGray = 207;
int fontColorBlack = 208;

void DrawMainBG()
{
	extern const u16 BACKGROUND_PAL[];
	extern const u8 BACKGROUND_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	u16 *frameBuffer16 = &MARS_FRAMEBUFFER;

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_PAL[i] & 0x7FFF;
	}

	memcpy(frameBuffer16 + 0x100, BACKGROUND_TILE, 320*224);
}

void DrawMainBGwGillian()
{
	extern const u16 BACKGROUND_W_GILLIAN_PAL[];
	extern const u8 BACKGROUND_W_GILLIAN_TILE[] __attribute__((aligned(16)));
	vu16 *cram16 = &MARS_CRAM;
	u16 *frameBuffer16 = &MARS_FRAMEBUFFER;

	for (int i = 0; i < 27; i++){
		cram16[i] = BACKGROUND_W_GILLIAN_PAL[i] & 0x7FFF;
	}

	memcpy(frameBuffer16 + 0x100, BACKGROUND_W_GILLIAN_TILE, 320*224);
}

void loadTextPalette()
{
	vu16 *cram16 = &MARS_CRAM;

	cram16[205] = COLOR(31,31,31); // 204 is White
	cram16[206] = COLOR(31,0,0);   // 205 is Red
	cram16[207] = COLOR(0,31,0);   // 206 is Green
	cram16[208] = COLOR(5,5,5);    // 207 is Gray
	cram16[209] = COLOR(0,0,0);    // 208 is Black
}

// Converts an integer to the relevant ascii char where 0 = ascii 65 ('A')
char ascii(int letterIndex)
{
	int asciiOffset = 65;
	char newChar;
	newChar = (char)(asciiOffset + letterIndex);
	return newChar;
}

void cleanup()
{
	Hw32xScreenClear();
	Hw32xSetBGColor(0,0,0,0);
}

void marsVDP256Start(void)
{
	Hw32xInit(MARS_VDP_MODE_256, 0);
}

void marsVDP32KStart(void)
{
	Hw32xInit(MARS_VDP_MODE_32K, 0);
}

void swapBuffers()
{
	MARS_VDP_FBCTL = currentFB ^ 1;
	while ((MARS_VDP_FBCTL & MARS_VDP_FS) == currentFB) {}
	currentFB ^= 1;	
}

void handle_input()
{
	u16 button, pressedButton, oldButton = 0;

	//The type is either 0xF if no controller is present, 1 if a six button pad is present, or 0 if a three button pad is present. The buttons are SET to 1 if the corresponding button is pressed, and consist of:
	//(0 0 0 1 M X Y Z S A C B R L D U) or (0 0 0 0 0 0 0 0 S A C B R L D U)

	// MARS_SYS_COMM10 holds the current button values: - - - - M X Y Z S A C B R L D U
	button = MARS_SYS_COMM8;

	if ((button & SEGA_CTRL_TYPE) == SEGA_CTRL_NONE){
		button = MARS_SYS_COMM10; //If controller 1 isn't detected, try using controller 2
	}

	pressedButton = button & ~oldButton;
    oldButton = button;
	
	while (MARS_SYS_COMM6 == SLAVE_LOCK) ; // wait until slave isn't blocking
	MARS_SYS_COMM6 = MASTER_LOCK; //tell slave to wait
	
	//pause when start is first pressed only
	if (pressedButton & SEGA_CTRL_START )
	{
	}
	/*else if (pressedButton & SEGA_CTRL_UP )
	{
		sprintf(keyPressedText,"Key Pressed: Up");
	}
	else if (pressedButton & SEGA_CTRL_DOWN )
	{
		sprintf(keyPressedText,"Key Pressed: Down");	
	}
	else if (pressedButton & SEGA_CTRL_LEFT )
	{
		sprintf(keyPressedText,"Key Pressed: Left");
	}
	else if (pressedButton & SEGA_CTRL_RIGHT )
	{
		sprintf(keyPressedText,"Key Pressed: Right");
	}
	else if (pressedButton & SEGA_CTRL_A)
	{
		sprintf(keyPressedText,"Key Pressed: A");
	}
	
	else if (pressedButton & SEGA_CTRL_B)
	{
		sprintf(keyPressedText,"Key Pressed: B");
	}
	
	else if (pressedButton & SEGA_CTRL_C)
	{
		sprintf(keyPressedText,"Key Pressed: C");
	}*/
	
	MARS_SYS_COMM6 = MASTER_STATUS_OK; //tell slave to resume
}

/*
CRC 32 based on work by Christopher Baker <https://christopherbaker.net>
*/

void CRC32_reset()
{
    _state = ~0L;
}

void CRC32_update(u8 data)
{
    u8 tbl_idx = 0;

    tbl_idx = _state ^ (data >> (0 * 4));
    _state = (*(u32*)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4));
    tbl_idx = _state ^ (data >> (1 * 4));
    _state = (*(u32*)(crc32_table + (tbl_idx & 0x0f)) ^ (_state >> 4));
}

u32 CRC32_finalize()
{
    return ~_state;
}

int memcmp1(const void *s1, const void *s2, int n)
{
    unsigned char u1, u2;

    for ( ; n-- ; s1++, s2++) {
	u1 = * (unsigned char *) s1;
	u2 = * (unsigned char *) s2;
	if ( u1 != u2) {
	    return (u1-u2);
	}
    }
    return 0;
}

u32 CalculateCRC(u32 startAddress, u32 size)
{
	u8 *bios = NULL;
	u32 address = 0, checksum = 0;

	CRC32_reset();

	bios = (void*)startAddress;
	for (address = 0; address < size; address ++)
	{
		u8 data;
		
		data = bios[address];

		CRC32_update(data);
	}

	checksum = CRC32_finalize();
	return checksum;
}

void setRandomSeed(u16 seed)
{
    // xor it with a random value to avoid 0 value
    randbase = seed ^ 0xD94B;
}

u16 random()
{
    randbase ^= (randbase >> 1) ^ MARS_SYS_COMM12;
    randbase ^= (randbase << 1);

    return randbase;
}