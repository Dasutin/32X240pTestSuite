/*
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
 * Copyright (C)2011-2026 Artemio Urbina
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

#ifndef _32X_IMAGES_H_
#define _32X_IMAGES_H_

#include "types.h"

#define MARS_SDRAM_CACHE	(*(volatile unsigned short *)0x06000000)
#define MARS_CACHE_OFFSET	0x20000000

/*
* Loads all colors from region in ROM defined by paletteStart to paletteEnd
* into the CRAM
* @param paletteStart - pointer to starting position of palette data
* @param paletteEnd - pointer to end position of palette data
* @param paletteOffset - offset in the CRAM to start writing the palette data to. Normally is 0.
*/
void loadPalette(const u16 *paletteStart, const u16 *paletteEnd, const int paletteOffset);

void fillRow8(int x, int y, int width, u8 color);
void fillRect8(int x, int y, int width, int height, u8 color);
void fillScreen8Pitched(int pitch, int height, u8 color);

/*
* Draws blank pixels to rectangle specified by x, y, xWidth and yWidth (height)
*/
void clearArea(vu16 x, vu16 y, int xWidth, int yWidth);

/*
* Draw line table to framebuffer
*/
void drawLineTable(int xOff);

void my_debug_put_char_8(int x, int y, unsigned char ch, vu8* fgColor, vu8* bgColor);
int myScreenPrintData(const char *buff, const int x, const int y, const vu8* fgColor, const vu8* bgColor );
void drawText(const char *str, int x, int y, int palOffs) ATTR_DATA_ALIGNED;
void drawTextwHighlight(const char *str, int x, int y, int textpalOffs,
	int shadowpalOffs) ATTR_DATA_ALIGNED;
void drawTextwBackground(const char *str, int x, int y, int palOffs);
void screenFadeOut(int fadeSpeed);
void clearScreen_Fill8bit();
void clearScreen_Fill16bit(u16 color);
void setColor(int index, int r, int g, int b);

#endif // _32X_IMAGES_H_
