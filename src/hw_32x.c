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

/*
 * Licensed under the BSD license
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * 32X by Chilly Willy
 */

#include "32x.h"
#include "hw_32x.h"
#include "perf.h"
#include "string.h"
#include "shared_objects.h"
#include "draw.h"
#include "sound.h"
#include "diagnostic_hw.h"

#define COLOR_BITS 15
#define COLOR_PRI  (1 << COLOR_BITS)
#define COLOR_MASK (COLOR_PRI - 1)

int old_camera_x, old_camera_y;
int main_camera_x, main_camera_y;
int camera_x, camera_y;

static int X = 0, Y = 0;
static int MX = 40, MY = 25;
static int init = 0;
static unsigned short fgc = 0, bgc = 0;
static short fgs = -1, bgs = -1;
static unsigned short fgp = COLOR_PRI, bgp = COLOR_PRI;

static volatile const uint8_t *new_palette;
static volatile char new_pri;
static volatile short new_alias_destination = -1;
static volatile short new_alias_source;
static volatile short new_alias_count;
static volatile unsigned short new_alias_priority;
static volatile unsigned short text_palette_enabled;

extern volatile u8 diagnosticRuntimeActive;

static void apply_text_palette(volatile unsigned short *palette)
{
	palette[203] = COLOR(0, 0, 0);
	palette[205] = COLOR(31, 31, 31);
	palette[206] = COLOR(31, 0, 0);
	palette[207] = COLOR(0, 31, 0);
	palette[208] = COLOR(5, 5, 5);
	palette[209] = COLOR(0, 0, 0);
	palette[210] = COLOR(8, 8, 8);
	palette[211] = COLOR(8, 0, 0);
	palette[212] = COLOR(0, 8, 0);
	palette[213] = COLOR(0, 0, 31);
	palette[214] = COLOR(0, 0, 8);
}

int nodraw = 0;

int32_t canvas_width = 320;
int32_t canvas_height = 224;

extern drawsprcmd_t slave_drawsprcmd;
extern drawspr4cmd_t slave_drawspr4cmd;
extern drawtileslavecmd_t slave_drawtilecmd;

static volatile unsigned int mars_vblank_count = 0;

#define UNCACHED_CURFB (*(short *)((int)&currentFB|0x20000000))

u32 schecksum = 0;

volatile unsigned short dmaDone = 1;

void pri_vbi_handler(void)
{
	int i;
	volatile unsigned short *palette = &MARS_CRAM;
	mars_vblank_count++;
	if (diagnosticRuntimeActive)
		return;

	if ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0)
		return;

	if (new_palette)
	{
		for (i = 0; i < 256; i++)
		{
			unsigned short priority = (i == bgs) ? bgp : fgp;
			palette[i] = (COLOR(new_palette[0] >> 3, new_palette[1] >> 3, new_palette[2] >> 3) & COLOR_MASK) | priority;
			new_palette += 3;
		}

		if (fgs >= 0)
			fgc = palette[fgs] | (fgc & COLOR_PRI);
		if (bgs >= 0)
			bgc = palette[bgs] | (bgc & COLOR_PRI);
	}
	else if (new_pri)
	{
		for (i = 0; i < 256; i++)
			palette[i] = (palette[i] & COLOR_MASK) | (i == bgs ? bgp : fgp);
	}

	if (fgs >= 0)
		palette[fgs] = fgc;
	if (bgs >= 0)
		palette[bgs] = bgc;

	if (new_alias_destination >= 0)
	{
		for (i = 0; i < new_alias_count; i++)
			palette[new_alias_destination + i] =
				(palette[new_alias_source + i] & COLOR_MASK) | new_alias_priority;
		new_alias_destination = -1;
	}

	if (text_palette_enabled)
		apply_text_palette(palette);

	new_palette = NULL;
	new_pri = 0;
}

unsigned Hw32xGetTicks(void)
{
	return mars_vblank_count;
}

void pri_dma1_handler(void)
{
	SH2_DMA_CHCR1; // Read TE
	SH2_DMA_CHCR1 = 0; // Clear TE
}

int Hw32xDetectPAL()
{
	return (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) == 0;
}

void Hw32xSetFGColor(int s, int r, int g, int b)
{
	volatile unsigned short *palette = &MARS_CRAM;
	fgs = s;
	if (s < 0)
		return;
	fgc = (COLOR(r, g, b) & COLOR_MASK) | fgp;
	palette[fgs] = fgc;
	new_pri = 1;
}

void Hw32xSetBGColor(int s, int r, int g, int b)
{
	volatile unsigned short *palette = &MARS_CRAM;
	bgs = s;
	if (s < 0)
		return;
	bgc = (COLOR(r, g, b) & COLOR_MASK) | bgp;
	palette[bgs] = bgc;
	new_pri = 1;
}

void Hw32xSetPaletteColor(int index, int r, int g, int b)
{
	volatile unsigned short *palette = &MARS_CRAM;
	unsigned short color;

	if (index < 0 || index >= 256)
		return;

	color = COLOR(r, g, b) & COLOR_MASK;
	if (index == fgs)
	{
		fgc = color | (fgc & COLOR_PRI);
		color = fgc;
	}
	if (index == bgs)
	{
		bgc = color | (bgc & COLOR_PRI);
		color = bgc;
	}

	palette[index] = color;
}

void Hw32xSetPalette(const uint8_t *palette)
{
	int i;
	volatile unsigned short *target = &MARS_CRAM;

	text_palette_enabled = 0;
	if (diagnosticRuntimeActive)
	{
		for (i = 0; i < 256; i++)
		{
			unsigned short priority = (i == bgs) ? bgp : fgp;

			target[i] = (COLOR(palette[0] >> 3, palette[1] >> 3,
				palette[2] >> 3) & COLOR_MASK) | priority;
			palette += 3;
		}
		new_palette = NULL;
		return;
	}
	new_palette = palette;
}

void Hw32xEnableTextPalette(void)
{
	text_palette_enabled = 1;
	apply_text_palette(&MARS_CRAM);
}

int Hw32xPauseTextPalette(void)
{
	int was_enabled = text_palette_enabled != 0;

	text_palette_enabled = 0;
	return was_enabled;
}

void Hw32xResumeTextPalette(int wasEnabled)
{
	text_palette_enabled = wasEnabled ? 1 : 0;
}

void Hw32xSetFGOverlayPriorityBit(int priority)
{
	fgp = priority ? COLOR_PRI : 0;
	fgc = (fgc & COLOR_MASK) | fgp;
	new_pri = 1;
}

void Hw32xSetBGOverlayPriorityBit(int priority)
{
	bgp = priority ? COLOR_PRI : 0;
	bgc = (bgc & COLOR_MASK) | bgp;
	new_pri = 1;
}

void Hw32xSetPalettePriorityAliases(int destination, int source, int count, int priority)
{
	if (destination < 0 || source < 0 || count <= 0 ||
		destination + count > 256 || source + count > 256)
		return;

	new_alias_source = source;
	new_alias_count = count;
	new_alias_priority = priority ? COLOR_PRI : 0;
	new_alias_destination = destination;
}

typedef struct {
	uint16_t *table;
	uint32_t count;
	int32_t current;
	int32_t pitch;
	int32_t wrap;
	int32_t hscroll;
} line_table_context_t;

extern void sh2_update_line_table(const line_table_context_t *context);

void Hw32xUpdateLineTable(int hscroll, int vscroll, int lineskip)
{
	uint32_t perf_start = perf_master_ticks();
	int i;
	int i_lineskip;
	const int ymask = canvas_yaw - 1;
	const int pitch = canvas_pitch >> 1;
	uint16_t *frameBuffer16 = (uint16_t *)&MARS_FRAMEBUFFER;

	hscroll += 0x100;

	if (lineskip == 0)
	{
		line_table_context_t context;
		context.table = frameBuffer16;
		context.count = canvas_height;
		context.current = pitch * vscroll;
		context.pitch = pitch;
		context.wrap = pitch * canvas_yaw;
		context.hscroll = hscroll;
		sh2_update_line_table(&context);
		perf_record(PERF_CPU_MASTER, PERF_METRIC_LINE_TABLE,
			perf_master_ticks() - perf_start);
		return;
	}

	i_lineskip = 0;
	for (i = 0; i < canvas_height / (lineskip + 1); i++)
	{
		int j = lineskip + 1;
		while (j)
		{
			frameBuffer16[i_lineskip + (lineskip + 1 - j)] = pitch * (vscroll & ymask) + hscroll; /* word offset of line */
			j--;
		}
		vscroll++;
		i_lineskip += lineskip + 1;
	}
	perf_record(PERF_CPU_MASTER, PERF_METRIC_LINE_TABLE,
		perf_master_ticks() - perf_start);
}

void Hw32xInit(int vmode, int lineskip)
{
	volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;
	int priority = vmode & (MARS_VDP_PRIO_32X | MARS_VDP_PRIO_68K);
	int i;

	// Wait for the SH2 to gain access to the VDP
	while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0);

	UNCACHED_CURFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	vmode &= ~(MARS_VDP_PRIO_32X | MARS_VDP_PRIO_68K);
	if (vmode == MARS_VDP_MODE_256)
	{
		// Set 8-bit paletted mode, 224 lines
		MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256 | priority;

		// Initialize both framebuffers

		// Flip the framebuffer selection bit and wait for it to take effect
		MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
		UNCACHED_CURFB ^= 1;
		// Rewrite line table
		Hw32xUpdateLineTable(0, 0, lineskip);
		// Clear screen
		for (i = 0x100; i < 0x10000; i++)
			frameBuffer16[i] = 0;

		// Flip the framebuffer selection bit and wait for it to take effect
		MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
		UNCACHED_CURFB ^= 1;
		// Rewrite line table
		Hw32xUpdateLineTable(0, 0, lineskip);
		// Clear screen
		for (i = 0x100; i < 0x10000; i++)
			frameBuffer16[i] = 0;

		MX = 40;
		MY = 28 / (lineskip + 1);
	}
	else if (vmode == MARS_VDP_MODE_32K)
	{
		// Set 16-bit direct mode, 224 lines
		MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_32K | priority;

		// Initialize both framebuffers

		// Flip the framebuffer selection bit and wait for it to take effect
		MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
		UNCACHED_CURFB ^= 1;
		// rewrite line table
		for (i = 0; i < canvas_height / (lineskip + 1); i++)
		{
			if (lineskip)
			{
				int j = lineskip + 1;
				while (j)
				{
					frameBuffer16[i * (lineskip +1 ) + (lineskip + 1 - j)] = i * canvas_pitch + 0x100;	// Word offset of line
					j--;
				}
			} else {
				if (i < 200)
					frameBuffer16[i] = i * canvas_pitch + 0x100;			// Word offset of line
				else
					frameBuffer16[i] = 200 * canvas_pitch + 0x100;		// Word offset of line
			}
		}
		// Clear screen
		for (i = 0x100; i < 0x10000; i++)
			frameBuffer16[i] = 0;

		// Flip the framebuffer selection bit and wait for it to take effect
		MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
		UNCACHED_CURFB ^= 1;
		// Rewrite line table
		for (i = 0; i < canvas_height / (lineskip + 1); i++)
		{
			if (lineskip)
			{
				int j = lineskip + 1;
				while (j)
				{
					frameBuffer16[i * (lineskip + 1) + (lineskip + 1 - j)] = i * canvas_pitch + 0x100;	// Word offset of line
					j--;
				}
			} else {
				if (i < 200)
					frameBuffer16[i] = i* canvas_pitch + 0x100;		// Word offset of line
				else
					frameBuffer16[i] = 200* canvas_pitch + 0x100;	// Word offset of line
			}
		}
		// Clear screen
		for (i = 0x100; i < 0x10000; i++)
			frameBuffer16[i] = 0;

		MX = 40;
		MY = 25 / (lineskip+1);
	}

	Hw32xSetFGColor(255,31,31,31);
	Hw32xSetBGColor(0,0,0,0);
	X = Y = 0;
	init = vmode;
}


int Hw32xScreenGetX()
{
	return X;
}

int Hw32xScreenGetY()
{
	return Y;
}

void Hw32xScreenSetXY(int x, int y)
{
	if (x < MX && x >= 0)
		X = x;
	if (y < MY && y >= 0)
		Y = y;
}

void Hw32xScreenClear()
{
	int i;
	int l = (init == MARS_VDP_MODE_256) ? canvas_pitch *224/2 + 0x100 : canvas_pitch * 200 + 0x100;
	volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;

	// A caller may have requested an asynchronous flip since the last wait.
	UNCACHED_CURFB = MARS_VDP_FBCTL & MARS_VDP_FS;

	// Clear screen
	for (i = 0x100; i < l; i++)
		frameBuffer16[i] = 0;

	// Flip the framebuffer selection bit and wait for it to take effect
	MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
	while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
	UNCACHED_CURFB ^= 1;

	// clear screen
	for (i = 0x100; i < l; i++)
		frameBuffer16[i] = 0;

	Hw32xSetFGColor(255,31,31,31);
	Hw32xSetBGColor(0,0,0,0);
	X = Y = 0;
}

// Return number of milliseconds elapsed since first call
unsigned long Hw32xGetTime(void)
{
	static u32 ticks_base = 0;
	u32 ticks;

	ticks = MARS_SYS_COMM12;
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		ticks *= 17;	// NTSC
	else
		ticks *= 20;	// PAL

	if (!ticks_base)
		ticks_base = ticks;

	return ticks - ticks_base;
}

void Hw32xSleep(int s)
{
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		Hw32xDelay(s/17);	// NTSC
	else
		Hw32xDelay(s/20);	// PAL
}

extern unsigned char msx[];

static void debug_put_char_16(int x, int y, unsigned char ch)
{
	volatile unsigned short *fb = &MARS_FRAMEBUFFER;
	int i, j;
	unsigned char *font;
	int vram, vram_ptr;

	if (!init)
		return;

	vram = 0x100 + x * 8;
	vram += (y * 8 * 320);

	font = &msx[(int)ch * 8];

	for (i = 0; i < 8; i++, font++)
	{
		vram_ptr  = vram;
		for (j = 0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
				fb[vram_ptr] = fgc;
			else
				fb[vram_ptr] = bgc;
			vram_ptr++;
		}
		vram += 320;
	}
}

static void debug_put_char_8(int x, int y, unsigned char ch)
{
	volatile unsigned char *fb = (volatile unsigned char *)&MARS_FRAMEBUFFER;
	int i, j;
	unsigned char *font;
	int vram, vram_ptr;

	if (!init)
		return;

	vram = 0x200 + x * 8;
	vram += (y * 8 * 320);

	font = &msx[(int)ch * 8];

	for (i = 0; i < 8; i++, font++)
	{
		vram_ptr  = vram;
		for (j = 0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
				fb[vram_ptr] = fgs;
			else
				fb[vram_ptr] = bgs;
			vram_ptr++;
		}
		vram += 320;
	}
}

void Hw32xScreenPutChar(int x, int y, unsigned char ch)
{
	if (init == MARS_VDP_MODE_256)
		debug_put_char_8(x, y, ch);
	else if (init == MARS_VDP_MODE_32K)
		debug_put_char_16(x, y, ch);
}

void Hw32xScreenClearLine(int Y)
{
	int i;

	for (i = 0; i < MX; i++)
		Hw32xScreenPutChar(i, Y, ' ');
}

// Print non-nul terminated strings
int Hw32xScreenPrintData(const char *buff, int size)
{
	int i;
	char c;

	if (!init)
		return 0;

	for (i = 0; i < size; i++)
	{
		c = buff[i];
		switch (c)
		{
			case '\r':
				X = 0;
				break;
			case '\n':
				X = 0;
				Y++;
				if (Y >= MY)
					Y = 0;
				Hw32xScreenClearLine(Y);
				break;
			case '\t':
				X = (X + 4) & ~3;
				if (X >= MX)
				{
					X = 0;
					Y++;
					if (Y >= MY)
						Y = 0;
					Hw32xScreenClearLine(Y);
				}
				break;
			default:
				Hw32xScreenPutChar(X, Y, c);
				X++;
				if (X >= MX)
				{
					X = 0;
					Y++;
					if (Y >= MY)
						Y = 0;
					Hw32xScreenClearLine(Y);
				}
		}
	}
	return i;
}

int Hw32xScreenPutsn(const char *str, int len)
{
	int ret;

	ret = Hw32xScreenPrintData(str, len);

	return ret;
}

void Hw32xScreenPrintf(const char *format, ...)
{
	va_list opt;
	char buff[128];
	int n;

	va_start(opt, format);
	n = vsprintf(buff, format, opt);
	va_end(opt);
	buff[sizeof(buff) - 1] = 0;

	Hw32xScreenPutsn(buff, n);
}

void Hw32xDelay(int ticks)
{
	unsigned long ct = mars_vblank_count + ticks;
	while (mars_vblank_count < ct);
}

void Hw32xScreenFlip(int wait)
{
	// Flip the framebuffer selection bit
	MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
	if (wait)
	{
		while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB);
		UNCACHED_CURFB ^= 1;
	}
}

void Hw32xFlipWait()
{
	while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
	UNCACHED_CURFB ^= 1;
}

// Mega Drive Command Support Code

#define MD_SHARED_BUFFER       ((volatile uint16_t *)0x2401FF00u)
#define MD_TEXT_BUFFER_BYTES   120
#define MD_CMD_PUTS            0x1700
#define MD_CMD_SCROLL_PLANES   0x1800
#define MD_CMD_CONTROLLERS     0x1900
#define MD_CONTROLLER_DEBUG_REPLY 0xFFFF

static unsigned short mdControllerTypes[MD_CONTROLLER_COUNT];
static unsigned short mdControllerPortTypes[2];
static unsigned short mdControllerPortSupports[2];

unsigned short HwMdReadPad(int port)
{
	if (port == 0)
		return MARS_SYS_COMM8;
	else if (port == 1)
		return MARS_SYS_COMM10;
	else
		return SEGA_CTRL_NONE;
}

static void HwMdControllerCommand(unsigned short operation)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM0 = MD_CMD_CONTROLLERS | operation;
	while (MARS_SYS_COMM0);
}

void HwMdControllerEnable(void)
{
	int i;

	for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		mdControllerTypes[i] = MD_CONTROLLER_TYPE_UNKNOWN;
	mdControllerPortTypes[0] = MD_CONTROLLER_PORT_UNKNOWN;
	mdControllerPortTypes[1] = MD_CONTROLLER_PORT_UNKNOWN;
	mdControllerPortSupports[0] = MD_CONTROLLER_SUPPORT_OFF;
	mdControllerPortSupports[1] = MD_CONTROLLER_SUPPORT_OFF;

	HwMdControllerCommand(0);
	Hw32xDelay(2);
	HwMdControllerReadPortInfo(mdControllerPortTypes, mdControllerPortSupports);
}

void HwMdControllerDisable(void)
{
	HwMdControllerCommand(1);
}

void HwMdControllerReset(void)
{
	HwMdControllerCommand(2);
	Hw32xDelay(2);
	HwMdControllerReadPortInfo(mdControllerPortTypes, mdControllerPortSupports);
}

static void HwMdControllerReadPayload(unsigned short *words)
{
	unsigned short verify[6];
	int i;

	do
	{
		words[0] = MARS_SYS_COMM2;
		words[1] = MARS_SYS_COMM6;
		words[2] = MARS_SYS_COMM8;
		words[3] = MARS_SYS_COMM10;
		words[4] = MARS_SYS_COMM12;
		words[5] = MARS_SYS_COMM14;

		verify[0] = MARS_SYS_COMM2;
		verify[1] = MARS_SYS_COMM6;
		verify[2] = MARS_SYS_COMM8;
		verify[3] = MARS_SYS_COMM10;
		verify[4] = MARS_SYS_COMM12;
		verify[5] = MARS_SYS_COMM14;

		for (i = 0; i < 6; i++)
		{
			if (words[i] != verify[i])
				break;
		}
	}
	while (i != 6);
}

static unsigned short HwMdControllerDecodeType(unsigned short packed)
{
	if (packed == 0)
		return MD_CONTROLLER_TYPE_PAD3;
	if (packed == 1)
		return MD_CONTROLLER_TYPE_PAD6;
	return MD_CONTROLLER_TYPE_UNKNOWN;
}

void HwMdControllerReadSnapshot(unsigned short *states,
	unsigned short *types, unsigned short *portTypes,
	unsigned short *portSupports)
{
	unsigned short words[6];
	int i;

	HwMdControllerReadPayload(words);

	states[0] = words[0] & 0x0FFF;
	states[1] = ((words[0] >> 12) | ((words[1] & 0x00FF) << 4)) & 0x0FFF;
	states[2] = ((words[1] >> 8) | ((words[2] & 0x000F) << 8)) & 0x0FFF;
	states[3] = (words[2] >> 4) & 0x0FFF;
	states[4] = words[3] & 0x0FFF;
	states[5] = ((words[3] >> 12) | ((words[4] & 0x00FF) << 4)) & 0x0FFF;
	states[6] = ((words[4] >> 8) | ((words[5] & 0x000F) << 8)) & 0x0FFF;
	states[7] = (words[5] >> 4) & 0x0FFF;

	for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		types[i] = mdControllerTypes[i];

	if (portTypes)
	{
		portTypes[0] = mdControllerPortTypes[0];
		portTypes[1] = mdControllerPortTypes[1];
	}
	if (portSupports)
	{
		portSupports[0] = mdControllerPortSupports[0];
		portSupports[1] = mdControllerPortSupports[1];
	}
}

void HwMdControllerReadPortInfo(unsigned short *portTypes,
	unsigned short *portSupports)
{
	unsigned short packedTypes;
	int i;

	MARS_SYS_COMM2 = 0xAAAA;
	HwMdControllerCommand(3);
	while (MARS_SYS_COMM2 != MD_CONTROLLER_DEBUG_REPLY);

	packedTypes = MARS_SYS_COMM6;
	for (i = 0; i < MD_CONTROLLER_COUNT; i++)
		mdControllerTypes[i] =
			HwMdControllerDecodeType((packedTypes >> (i << 1)) & 3);
	mdControllerPortTypes[0] = MARS_SYS_COMM8;
	mdControllerPortTypes[1] = MARS_SYS_COMM10;
	mdControllerPortSupports[0] = MARS_SYS_COMM12;
	mdControllerPortSupports[1] = MARS_SYS_COMM14;

	portTypes[0] = mdControllerPortTypes[0];
	portTypes[1] = mdControllerPortTypes[1];
	portSupports[0] = mdControllerPortSupports[0];
	portSupports[1] = mdControllerPortSupports[1];
	/* The 68000 writes COMM2 last for both reply and state packets. */
	while (MARS_SYS_COMM2 == MD_CONTROLLER_DEBUG_REPLY);
}

unsigned short HwMdControllerReadState(int controller)
{
	unsigned short states[MD_CONTROLLER_COUNT];
	unsigned short types[MD_CONTROLLER_COUNT];

	if (controller < 0 || controller >= MD_CONTROLLER_COUNT)
		return 0;
	HwMdControllerReadSnapshot(states, types, 0, 0);
	return states[controller];
}

unsigned short HwMdControllerReadType(int controller)
{
	unsigned short states[MD_CONTROLLER_COUNT];
	unsigned short types[MD_CONTROLLER_COUNT];

	if (controller < 0 || controller >= MD_CONTROLLER_COUNT)
		return MD_CONTROLLER_TYPE_UNKNOWN;
	HwMdControllerReadSnapshot(states, types, 0, 0);
	return types[controller];
}

unsigned short HwMdControllerReadPortType(int port)
{
	unsigned short portTypes[2];
	unsigned short portSupports[2];

	if (port < 0 || port > 1)
		return MD_CONTROLLER_PORT_UNKNOWN;
	HwMdControllerReadPortInfo(portTypes, portSupports);
	return portTypes[port];
}

unsigned short HwMdControllerReadPortSupport(int port)
{
	unsigned short portTypes[2];
	unsigned short portSupports[2];

	if (port < 0 || port > 1)
		return MD_CONTROLLER_SUPPORT_OFF;
	HwMdControllerReadPortInfo(portTypes, portSupports);
	return portSupports[port];
}

unsigned char HwMdReadSram(unsigned short offset)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = offset;
	MARS_SYS_COMM0 = 0x0100;					// Read SRAM
	while (MARS_SYS_COMM0);
	return MARS_SYS_COMM2 & 0x00FF;
}

void HwMdWriteSram(unsigned char byte, unsigned short offset)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = offset;
	MARS_SYS_COMM0 = 0x0200 | byte;				// Write SRAM
	while (MARS_SYS_COMM0);
}

int HwMdReadMouse(int port)
{
	unsigned int mouse1, mouse2;
	while (MARS_SYS_COMM0);							// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM0 = 0x0500 | port;					// Tell 68000 to read mouse
	while (MARS_SYS_COMM0 == (0x0500 | port));		// Wait for mouse value
	mouse1 = MARS_SYS_COMM0;
	mouse2 = MARS_SYS_COMM2;
	MARS_SYS_COMM0 = 0;								// Tells 68000 we got the mouse value
	return (int)((mouse1 << 16) | mouse2);
}

void HwMdClearScreen(void)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM0 = 0x0600;					// Clear Screen (Name Table B)
	while (MARS_SYS_COMM0);
}

void HwMdSetOffset(unsigned short offset)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = offset;
	MARS_SYS_COMM0 = 0x0700;					// Set offset (into either Name Table B or VRAM)
	while (MARS_SYS_COMM0);
}

void HwMdSetNTable(unsigned short word)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x0800;					// Set word at offset in Name Table B
	while (MARS_SYS_COMM0);
}

void HwMdSetVram(unsigned short word)
{
	while (MARS_SYS_COMM0);						// Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x0900;					// Set word at offset in VRAM
	while (MARS_SYS_COMM0);
}

void HwMdPuts(const char *str, int color, int x, int y)
{
	unsigned short offset = ((y << 6) | x) << 1;

	while (*str)
	{
		volatile uint16_t *shared = MD_SHARED_BUFFER;
		volatile uint8_t *text = (volatile uint8_t *)(shared + 3);
		unsigned short length = 0;
		unsigned short i;

		while (str[length] && length < MD_TEXT_BUFFER_BYTES)
			length++;

		/* The 68000 temporarily owns the shared framebuffer window. */
		Mars_R_SecWait();
		while (MARS_SYS_COMM0);
		shared[0] = offset;
		shared[1] = color;
		shared[2] = length;
		for (i = 0; i < length; i++)
			text[i] = str[i];
		MARS_SYS_COMM0 = MD_CMD_PUTS;
		while (MARS_SYS_COMM0);

		str += length;
		offset += length << 1;
	}
}

void HwMdPutc(char chr, int color, int x, int y)
{
	char text[2] = { chr, 0 };
	HwMdPuts(text, color, x, y);
}

void HwMdScreenPrintf(int color, int x, int y, const char *format, ...)
{
	va_list opt;
	char buff[64];

	va_start(opt, format);
	vsprintf(buff, format, opt);
	va_end(opt);
	buff[sizeof(buff) - 1] = 0;

	HwMdPuts(buff, color, x, y);
}

void HwMdSetPal(unsigned short pal)
{
	while (MARS_SYS_COMM0);                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = pal;
	MARS_SYS_COMM0 = 0x0A00;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdSetColor(unsigned short color)
{
	while (MARS_SYS_COMM0);                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = color;
	MARS_SYS_COMM0 = 0x0B00;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdSetColorPal(unsigned short pal, unsigned short color)
{
	HwMdSetPal(pal);
	HwMdSetColor(color);
}

void HwMdPSGSetChannel(unsigned short word)
{
	while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x0C00;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdPSGSetVolume(unsigned short word)
{
	while (MARS_SYS_COMM0);                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x0D00;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdPSGSetChandVol(unsigned short channel, unsigned short vol)
{
	HwMdPSGSetChannel(channel);
	HwMdPSGSetVolume(vol);
}

void HwMdPSGSendTone(unsigned short value1, unsigned short value2)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM2 = ((value1 & 0xFF) << 8) | (value2 & 0xFF);
	MARS_SYS_COMM0 = 0x0E00;                    // Send both PSG bytes
	while (MARS_SYS_COMM0);
}

void HwMdPSGSendNoise(unsigned short word)
{
	while (MARS_SYS_COMM0);                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x1000;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdPSGSendEnvelope(unsigned short word)
{
	while (MARS_SYS_COMM0);                    // Wait until 68000 has responded to any earlier requests
	MARS_SYS_COMM2 = word;
	MARS_SYS_COMM0 = 0x1100;                    // Send handle request flag
	while (MARS_SYS_COMM0);
}

void HwMdPSGSetFrequency(u8 channel, u16 value)
{
	u16 data;

	if (value)
	{
		if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
			data = 3579545 / (value * 32);
		else
			data = 3546893 / (value * 32);
	}
	else data = 0;

	HwMdPSGSetTone(channel, data);
}

void HwMdPSGSetTone(u8 channel, u16 value)
{
	vu8 value1;
	vu8 value2;

	value1 = 0x80 | ((channel & 3) << 5) | (value & 0xF);
	value2 = (value >> 4) & 0x3F;

	HwMdPSGSendTone(value1, value2);
}

void HwMdPSGSetNoise(u8 type, u8 frequency)
{
	vu8 value;

	value = 0xE0 | ((type & 1) << 2) | (frequency & 0x3);
	HwMdPSGSendNoise(value);
}

void HwMdPSGSetEnvelope(u8 channel, u8 value)
{
	vu8 data;

	data = 0x90 | ((channel & 3) << 5) | (value & 0xF);
	HwMdPSGSendEnvelope(data);
}

#define HwMDPlaneNum(plane) ((plane) >= 'A' && (plane) <= 'B' ? (plane) - 'A' : ((plane) >= 'a' && (plane) <= 'b' ? (plane) - 'a' : (plane) & 1))

void HwMdClearPlanes(void)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM0 = 0x1200;
	while (MARS_SYS_COMM0);
}

void HwMdReloadFont(void)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM0 = 0x1500;
	while (MARS_SYS_COMM0);
}

void HwMdSetPlaneBitmap(char plane, void *data)
{
	while (MARS_SYS_COMM0);
	*(volatile uintptr_t *)&MARS_SYS_COMM12 = (uintptr_t)data;
	MARS_SYS_COMM0 = 0x1300 + HwMDPlaneNum(plane);
	while (MARS_SYS_COMM0);
}

void HwMdHScrollPlane(char plane, int hscroll)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM2 = hscroll;
	MARS_SYS_COMM0 = 0x1400 + HwMDPlaneNum(plane);
	while (MARS_SYS_COMM0);
}

void HwMdVScrollPlane(char plane, int vscroll)
{
	while (MARS_SYS_COMM0);
	MARS_SYS_COMM2 = vscroll;
	MARS_SYS_COMM0 = 0x1402 + HwMDPlaneNum(plane);
	while (MARS_SYS_COMM0);
}

void HwMdSetPlaneScrolls(int hscroll_a, int vscroll_a,
	int hscroll_b, int vscroll_b)
{
	volatile uint16_t *shared = MD_SHARED_BUFFER;

	Mars_R_SecWait();
	while (MARS_SYS_COMM0);
	shared[0] = hscroll_a;
	shared[1] = vscroll_a;
	shared[2] = hscroll_b;
	shared[3] = vscroll_b;
	MARS_SYS_COMM0 = MD_CMD_SCROLL_PLANES;
	while (MARS_SYS_COMM0);
}

// Put Secondary Calls here

int secondary_task(int cmd)
{
	int drawcnt;

	switch (cmd)
	{
	case 1:
		return 1;
	case 2:
	{
		uint32_t perf_start;
		ClearCacheLines(&slave_drawsprcmd, (sizeof(drawsprcmd_t) + 15) / 16);
		perf_start = perf_slave_ticks();
		draw_handle_drawspritecmd(&slave_drawsprcmd);
		perf_record_sprite(PERF_CPU_SLAVE, slave_drawsprcmd.sdata,
			perf_slave_ticks() - perf_start);
		return 1;
	}
	case 3:
	{
		uint32_t perf_start;
		ClearCacheLines(&slave_drawtilecmd,
			(sizeof(drawtileslavecmd_t) + 15) / 16);
		perf_start = perf_slave_ticks();
		drawcnt = draw_handle_layercmd(&slave_drawtilecmd.draw);
		perf_record(PERF_CPU_SLAVE, PERF_METRIC_TILEMAP,
			perf_slave_ticks() - perf_start);
		drawcnt = ((drawcnt + 1) << 2) | MARS_SYS_COMM4;
		MARS_SYS_COMM4 = drawcnt;
		while (MARS_SYS_COMM4 == drawcnt);
		return 1;
	}
	case 4:
	case 5:
		return 1;
	case 6:
		Mars_Sec_InitSoundDMA();
		return 1;
	case 7:
		Mars_Sec_StopSoundMixer();
		return 1;
	case 8:
		Mars_Sec_StartSoundMixer();
		return 1;
	case MARS_SEC_CMD_PWM_TEST_START:
		Mars_Sec_StartTestPWMTone();
		return 1;
	case MARS_SEC_CMD_PWM_TEST_STOP:
		Mars_Sec_StopTestPWMTone();
		return 1;
	case MARS_SEC_CMD_SDRAM_PARK:
		Hw32xSecondaryPark();
		return 1;
	case DIAG_SEC_ENTER:
		diagnosticSecondaryAgent();
		return 1;
	default:
		break;
	}

	return 0;
}

// Slave waiting for commands (called by crt0.s)

void secondary(void)
{
	ClearCache();

	while (1)
	{
		int cmd;

		while ((cmd = MARS_SYS_COMM4) == 0) {}

		int res = secondary_task(cmd);
		if (res > 0)
		{
			MARS_SYS_COMM4 = 0;
		}
	}
}
