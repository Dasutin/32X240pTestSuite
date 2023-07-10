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

#ifndef _SHARED_OBJECTS_
#define _SHARED_OBJECTS_

// Constants
#define MASTER_STATUS_OK 1
#define MASTER_LOCK 4
#define SLAVE_STATUS_OK 2
#define SLAVE_LOCK 8

#define PAUSED 1
#define UNPAUSED 0

#define TRUE 1
#define FALSE 0

extern u8 paused;

extern unsigned short int currentFB;

#define fontBackgroundColorBlack 203
#define fontColorWhite 204
#define fontColorRed 205
#define fontColorGreen 206
#define fontColorGray 207
#define fontColorBlack 208
#define fontColorWhiteHighlight 209
#define fontColorRedHighlight 210
#define fontColorGreenHighlight 211

extern void drawMainBG(void);
extern void drawBGwGil(void);
extern void redrawBGwGil(void);
extern void drawQRCode(u16 x, u16 y, u16 xWidth, u16 yWidth);
extern void loadTextPalette(void);
extern void drawResolution(void);
extern void loadMainBGwGilPalette(void);
extern void cleanup(void);
extern void initMainBG(void);
extern void initMainBGwGil(void);
extern void marsVDP256Start(void);
extern void marsVDP32KStart(void);
extern void swapBuffers(void);
void CRC32_reset();
void CRC32_update(u8 data);
u32 CRC32_finalize();
u32 CalculateCRC(u32 startAddress, u32 size);
int memcmp1(const void *s1, const void *s2, int n);

// 384 seems to be the ideal value - anything thing
// increases the odds of hitting the "0xFF screen shift
// register bug"

// canvas_width + scrollwidth
extern uint32_t canvas_pitch;
// canvas_height + scrollheight
extern uint32_t canvas_yaw;

void setRandomSeed(u16 seed);
u16 random();

#endif // _SHARED_OBJECTS_
