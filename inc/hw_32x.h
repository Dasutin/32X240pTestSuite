/* 
 * 240p Test Suite for the Sega 32X
 * Port by Dasutin (Dustin Dembrosky)
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

#ifndef _HW_32X_H_
#define _HW_32X_H_

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MD_PAL_0 0x0100
#define MD_PAL_1 0x0200
#define MD_PAL_2 0x0300
#define MD_PAL_3 0x0400

#define MD_COLOR_PAL_GRAY  0x0100
#define MD_COLOR_PAL_RED   0x0200
#define MD_COLOR_PAL_GREEN 0x0300
#define MD_COLOR_PAL_BLUE  0x0400

#define PSG_ENVELOPE_MIN    15
#define PSG_ENVELOPE_MAX    0
#define PSG_NOISE_TYPE_PERIODIC 0
#define PSG_NOISE_TYPE_WHITE    1
#define PSG_NOISE_FREQ_CLOCK2   0
#define PSG_NOISE_FREQ_CLOCK4   1
#define PSG_NOISE_FREQ_CLOCK8   2
#define PSG_NOISE_FREQ_TONE3    3

#define HW32X_ATTR_DATA_ALIGNED __attribute__((section(".data"), aligned(16)))

extern int Hw32xDetectPAL();
extern void Hw32xSetFGColor(int s, int r, int g, int b);
extern void Hw32xSetBGColor(int s, int r, int g, int b);
extern void Hw32xSetPalette(const uint8_t *palette) HW32X_ATTR_DATA_ALIGNED;
extern void Hw32xInit(int vmode, int lineskip);
extern int Hw32xScreenGetX();
extern int Hw32xScreenGetY();
extern void Hw32xScreenSetXY(int x, int y);
extern void Hw32xScreenClear();
extern void Hw32xScreenPutChar(int x, int y, unsigned char ch);
extern void Hw32xScreenClearLine(int Y);
extern int Hw32xScreenPrintData(const char *buff, int size);
extern int Hw32xScreenPuts(const char *str);
extern void Hw32xScreenPrintf(const char *format, ...);
extern void Hw32xDelay(int ticks);
extern void Hw32xScreenFlip(int wait);
extern void Hw32xFlipWait();
extern unsigned long Hw32xGetTime(void);
extern void Hw32xSleep(int s);

extern unsigned short HwMdReadPad(int port);
extern unsigned char HwMdReadSram(unsigned short offset);
extern void HwMdWriteSram(unsigned char byte, unsigned short offset);
extern int HwMdReadMouse(int port);
extern void HwMdClearScreen(void);
extern void HwMdSetOffset(unsigned short offset);
extern void HwMdSetNTable(unsigned short word);
extern void HwMdSetVram(unsigned short word);
extern void HwMdPuts(char *str, int color, int x, int y);
extern void HwMdPutc(char chr, int color, int x, int y);
extern void HwMdScreenPrintf(int color, int x, int y, const char *format, ...);
extern void HwMdSetPal(unsigned short pal);
extern void HwMdSetColor(unsigned short color);
extern void HwMdSetColorPal(unsigned short pal, unsigned short color);
extern void HwMdPSGSetChannel(unsigned short word);
extern void HwMdPSGSetVolume(unsigned short word);
extern void HwMdPSGSetChandVol(unsigned short channel, unsigned short vol);
extern void HwMdPSGSendTone(unsigned short value, unsigned short value2);
extern void HwMdPSGSendNoise(unsigned short word);
extern void HwMdPSGSendEnvelope(unsigned short word);
extern void HwMdPSGSetFrequency(u8 channel, u16 value);
extern void HwMdPSGSetTone(u8 channel, u16 value);
extern void HwMdPSGSetNoise(u8 type, u8 frequency);
extern void HwMdPSGSetEnvelope(u8 channel, u8 value);

void Hw32xUpdateLineTable(int hscroll, int vscroll, int lineskip) HW32X_ATTR_DATA_ALIGNED;

static inline void Mars_R_SecWait(void)
{
	while (MARS_SYS_COMM4 != 0);
}

static inline void Mars_InitSoundDMA(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 6;
	Mars_R_SecWait();
}

static inline void Mars_StopSoundMixer(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 7;
	Mars_R_SecWait();
}

static inline void Mars_StartSoundMixer(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = 8;
	Mars_R_SecWait();
}

extern void Hw32xSecWait(void);

extern int Hw32xInitSoundDMA(void);
extern void Hw32xSecondaryBIOS(void);

void pri_vbi_handler(void) HW32X_ATTR_DATA_ALIGNED;
void pri_dma1_handler(void) HW32X_ATTR_DATA_ALIGNED;

unsigned Hw32xGetTicks(void) HW32X_ATTR_DATA_ALIGNED;

int secondary_task(int cmd) HW32X_ATTR_DATA_ALIGNED;
void secondary(void) HW32X_ATTR_DATA_ALIGNED;

#endif /* _HW_32X_H_ */