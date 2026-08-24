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

#include "types.h"
#include "string.h"
#include "32x.h"
#include "hw_32x.h"
#include "shared_objects.h"
#include "draw.h"
#include "font_tiles.h"
#include "perf.h"

#define PIXEL_WRITE_BUFFER_SIZE_B 8
#define FRAMEBUFFER_ROW_EXTENSION 16
static const int TRANSPARENT_PIXEL_COLOR = 0;

// * Loads all colors from region in ROM defined by paletteStart to paletteEnd
// * into the CRAM
// * @param paletteStart - pointer to starting position of palette data
// * @param paletteEnd - pointer to end position of palette data
// * @param paletteOffset - offset in the CRAM to start writing the palette data to. Normally is 0.

void loadPalette(const u16 *paletteStart[], const u16 *paletteEnd[], const int paletteOffset)
{
	int numColors;
	int i;
	vu16 *cram16 = &MARS_CRAM;
	vu16 *pal16 = (vu16*) paletteStart;

	numColors = ((paletteEnd) - (paletteStart));
	for (i = paletteOffset; i < numColors + paletteOffset; i++)
		cram16[i] = pal16[i - paletteOffset] & 0x7FFF;
}

// * Draws blank pixels to rectangle specified by x, y, xWidth and yWidth (height)

void clearArea(vu16 x, vu16 y, int xWidth, int yWidth)
{
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	vu16 xOff;
	int bufCnt;
	int size = (xWidth * yWidth) / 2;
	const u16 lineTableEnd = 0x100;
	u16 fbOff = lineTableEnd;
	int drawWidth = 0;

	// Offset the number of pixels in each line to start to draw the image
	xOff = x / 2;
	// Y-offset for top of sprite to correct line in framebuffer
	fbOff = lineTableEnd;
	// X-offset from start of first line
	fbOff = fbOff + (y * 160);
	// Draw spriteBuffer to the framebuffer
	fbOff = fbOff + xOff;

	drawWidth = 0;
	for (bufCnt = 0; bufCnt < size; bufCnt++)
	{
		// Write word to framebuffer
		frameBuffer16[fbOff] = TRANSPARENT_PIXEL_COLOR;
		fbOff++;
		drawWidth++;
		if (drawWidth >= (xWidth / 2))
		{
			// Reset line
			drawWidth = 0;
			fbOff = fbOff + (160 - ((xWidth / 2) + xOff)) + xOff;
		}
	}
}

static void autofill_words(unsigned address, unsigned count, u16 value)
{
	while (count)
	{
		unsigned page_words = 0x100 - (address & 0xFF);
		unsigned chunk = count < page_words ? count : page_words;

		MARS_VDP_FILLEN = chunk - 1;
		MARS_VDP_FILADR = address;
		MARS_VDP_FILDAT = value;
		while (MARS_VDP_FBCTL & MARS_VDP_FEN);

		address += chunk;
		count -= chunk;
	}
}

void fillRow8(int x, int y, int width, u8 color)
{
	vu16 *fb;
	unsigned address;
	u16 packed = ((u16)color << 8) | color;
	int physical_x;
	int words;

	if (y < 0 || y >= canvas_height || width <= 0)
		return;
	if (x < 0)
	{
		width += x;
		x = 0;
	}
	if (x >= canvas_width)
		return;
	if (x + width > canvas_width)
		width = canvas_width - x;
	if (width <= 0)
		return;

	physical_x = x + window_canvas_x;
	address = 0x100 + y * (canvas_pitch >> 1) + (physical_x >> 1);
	fb = (vu16 *)&MARS_FRAMEBUFFER + address;

	if (physical_x & 1)
	{
		*fb = (*fb & 0xFF00) | color;
		fb++;
		address++;
		width--;
	}

	words = width >> 1;
	if (words >= 8)
		autofill_words(address, words, packed);
	else
		while (words--)
			*fb++ = packed;

	if (width & 1)
	{
		fb = (vu16 *)&MARS_FRAMEBUFFER + address + (width >> 1);
		*fb = (*fb & 0x00FF) | ((u16)color << 8);
	}
}

void fillRect8(int x, int y, int width, int height, u8 color)
{
	int row;

	if (x == 0 && y == 0 && width >= canvas_width &&
		height >= canvas_height)
	{
		autofill_words(0x100, (canvas_pitch * canvas_height) >> 1,
			((u16)color << 8) | color);
		return;
	}

	if (y < 0)
	{
		height += y;
		y = 0;
	}
	if (y + height > canvas_height)
		height = canvas_height - y;
	if (height <= 0)
		return;

	for (row = 0; row < height; row++)
		fillRow8(x, y + row, width, color);
}

void fillScreen8Pitched(int pitch, int height, u8 color)
{
	if (pitch <= 0 || height <= 0)
		return;
	autofill_words(0x100, ((unsigned)pitch * (unsigned)height) >> 1,
		((u16)color << 8) | color);
}

extern unsigned char msx[];
void my_debug_put_char_8(const int x, const int y, const unsigned char ch, const vu8* fgColor, const vu8* bgColor)
{
	vu8 *fb = (vu8*)&MARS_FRAMEBUFFER;
	int i, j;
	u8 *font;
	int vram, vram_ptr;

	vram = 0x200 + x;
	vram += (y *(SCREEN_WIDTH + FRAMEBUFFER_ROW_EXTENSION)) + 8;

	font = &msx[ (int)ch * 8];

	for (i = 0; i < 8; i++, font++)
	{
		vram_ptr = vram;
		for (j = 0; j < 8; j++)
		{
			if ((*font & (128 >> j)))
				fb[vram_ptr] = fgColor[0];
			else
				fb[vram_ptr] = bgColor[0];
			vram_ptr++;
		}
		vram += (SCREEN_WIDTH + FRAMEBUFFER_ROW_EXTENSION);
	}
}

// * Print non-nul terminated strings

int myScreenPrintData(const char *buff, const int x, const int y, const vu8* fgColor, const vu8* bgColor)
{
	int i, size, xOff;
	char c;

	size = strlen(buff);
	xOff = x;

	for (i = 0; i < size; i++)
	{
		c = buff[i];
		my_debug_put_char_8(xOff, y, c, fgColor, bgColor);
		// Move 8 bytes
		xOff += PIXEL_WRITE_BUFFER_SIZE_B - 1;
	}
	return i;
}

// * Draw line table to framebuffer
// * @param xOff - the x offset in words for each line in the line table

void drawLineTable(const int xOff)
{
	// Screen line width in words plus 4 extra words on each side
	const int lineWidth = 160 + 8;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	const u16 lineTableEnd = 0x100;
	u16 lineOffs = lineTableEnd + xOff;

	// Set up the line table
	for (int i = 0; i < 256; i++)
	{
		frameBuffer16[i] = lineOffs;
		// This made a warping effect!
		lineOffs += lineWidth;
	}
}

void drawText(const char *str, int x, int y, int palOffs) ATTR_DATA_ALIGNED;
void drawTextwHighlight(const char *str, int x, int y, int textpalOffs,
	int shadowpalOffs) ATTR_DATA_ALIGNED;

typedef struct {
	vu8 *dst;
	const uint8_t *masks;
	const uint16_t *pairs;
	const uint8_t *singles;
	uint32_t pitch;
	uint32_t odd_x;
} font_glyph_context_t;

static uint8_t font_row_masks[96][8] ATTR_CACHE_ALIGNED;
static int font_masks_ready;

extern void sh2_font_glyph(const font_glyph_context_t *context);

static void prepare_font_masks(void)
{
	unsigned character;

	if (font_masks_ready)
		return;
	for (character = 0; character < 96; character++) {
		unsigned base = (character >> 4) * 1024 +
			(character & 15) * 8;
		unsigned row;
		for (row = 0; row < 8; row++) {
			unsigned pixel;
			uint8_t mask = 0;
			for (pixel = 0; pixel < 8; pixel++)
				if (font_tile[base + row * 128 + pixel])
					mask |= 0x80 >> pixel;
			font_row_masks[character][row] = mask;
		}
	}
	font_masks_ready = 1;
}

void drawText(const char *str, int x, int y, int palOffs)
{
	uint32_t perf_start = perf_master_ticks();
	uint8_t color = palOffs + 1;
	uint8_t singles[2] = {0, color};
	uint16_t pairs[4] = {0, color, (uint16_t)color << 8,
		(uint16_t)color * 0x101};
	font_glyph_context_t context;
	vu8 *fb = (vu8 *)&MARS_OVERWRITE_IMG + 0x200;
	int i;

	prepare_font_masks();
	context.pairs = pairs;
	context.singles = singles;
	context.pitch = canvas_pitch;

	for (i = 0; i < 40; i++)
	{
		unsigned c = (uint8_t)str[i];
		int glyph_x = x + i * 8;
		if (!c)
			break;
		if (c < ' ' || c >= 128)
			c = '?';
		c -= ' ';

		if (glyph_x >= canvas_width)
			break;
		if (glyph_x + 8 <= 0 || y + 8 <= 0 || y >= canvas_height)
			continue;

		context.masks = font_row_masks[c];
		if (glyph_x >= 0 && glyph_x + 8 <= canvas_width &&
			y >= 0 && y + 8 <= canvas_height) {
			context.dst = fb + y * canvas_pitch + glyph_x;
			context.odd_x = glyph_x & 1;
			sh2_font_glyph(&context);
		} else {
			int row;
			for (row = 0; row < 8; row++) {
				int screen_y = y + row;
				int pixel;
				uint8_t mask = context.masks[row];
				if (screen_y < 0 || screen_y >= canvas_height)
					continue;
				for (pixel = 0; pixel < 8; pixel++) {
					int screen_x = glyph_x + pixel;
					if (screen_x >= 0 && screen_x < canvas_width &&
						(mask & (0x80 >> pixel)))
						fb[screen_y * canvas_pitch + screen_x] = color;
				}
			}
		}
	}
	perf_record(PERF_CPU_MASTER, PERF_METRIC_TEXT,
		perf_master_ticks() - perf_start);
}

void drawTextwHighlight(const char *str, int x, int y, int textpalOffs,
	int shadowpalOffs)
{
	drawText(str, x + 1, y + 1, shadowpalOffs);
	drawText(str, x, y, textpalOffs);
}

void screenFadeOut(int fadeSpeed)
{
	u16 frameDelay = fadeSpeed;
	u16 len = 16;
	u16 r, g, b;
	u16 tempcolor;
	vu16 temppal[256];
	vu16 *cram16 = &MARS_CRAM;
	int textPaletteEnabled = Hw32xPauseTextPalette();

	while (len != 0)
	{
		for (int i = 0; i <= 255; i++)
			temppal[i] = cram16[i];

		for (int i = 0; i <= 255; i++)
		{
			u16 priority = temppal[i] & 0x8000;
			tempcolor = temppal[i] & 0x7FFF;
			if (tempcolor != 0x0000)
			{
				r = tempcolor & 0x1F;
				g = tempcolor>>5 & 0x1F;
				b = tempcolor>>10 & 0x1F;
				r = r > 1 ? r - 2 : 0;
				g = g > 1 ? g - 2 : 0;
				b = b > 1 ? b - 2 : 0;
				tempcolor = COLOR(r,g,b) & 0x7FFF;
				temppal[i] = tempcolor | priority;
			}
		}
		for (int i = 0; i <= 255; i++)
			cram16[i] = temppal[i];

		Hw32xDelay(frameDelay);

		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		len--;
	}
	Hw32xScreenClear();
	Hw32xResumeTextPalette(textPaletteEnabled);
	return;
}

void drawTextwBackground(const char *str, int x, int y, int palOffs)
{
	int screenOffs, fontOffs;
	int c;
	vu8 *fb = (volatile unsigned char*)&MARS_FRAMEBUFFER + 0x200;

	for (int i = 0; i < 40; i++)
	{
		c = str[i];
		if (!c) break;

		c -= ' ';
		screenOffs = y * 320 + i * 8 + x;
		fontOffs = (c >> 4) << 10;
		fontOffs += (c & 15) << 3;
		for (int t = 0; t < 8; t++)
		{
			for (int s = 0; s < 8; s++)
			{
				if (fontwbackground_tile[fontOffs + s])
					fb[screenOffs + s] = fontwbackground_tile[fontOffs + s] + palOffs;
			}
			screenOffs += 320;
			fontOffs += 128;
		}
	}
}

void clearScreen_Fill8bit()
{
	fillRect8(0, 0, canvas_width, canvas_height, 0);
}

void clearScreen_Fill16bit(u16 color)
{
	fillRect8(0, 0, canvas_width, canvas_height, color);
}

void setColor(int index, int r, int g, int b)
{
	Hw32xSetPaletteColor(index, r, g, b);
}
