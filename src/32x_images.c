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
#include "string.h"
#include "32x.h"
#include "lzss_decode.h"
#include "shared_objects.h"

#define PIXEL_WRITE_BUFFER_SIZE_B 8
#define FRAMEBUFFER_ROW_EXTENSION 16
#define IS_MIRRORED 1

#define fontColorWhite 204
#define fontColorRed 205
#define fontColorGreen 206
#define fontColorGray 207
#define fontColorBlack 208

#define IS_TRANSPARENT 1
static const int TRANSPARENT_PIXEL_COLOR = 0;
static const vu8 pixelWords[PIXEL_WRITE_BUFFER_SIZE_B] = {0,0,0,0,0,0,0,0};
/*
* Loads all colors from region in ROM defined by paletteStart to paletteEnd
* into the CRAM
* @param paletteStart - pointer to starting position of palette data
* @param paletteEnd - pointer to end position of palette data
* @param paletteOffset - offset in the CRAM to start writing the palette data to. Normally is 0.
*/
void loadPalette(const char *paletteStart, const char *paletteEnd, const int paletteOffset)
{
	int numColors;
	int i;
	vu16 *cram16 = &MARS_CRAM;
	vu16 *pal16 = (vu16*) paletteStart;
	
	numColors = ((paletteEnd)-(paletteStart));
	for (i = paletteOffset; i < numColors+paletteOffset; i++)
	{
		cram16[i] = pal16[i-paletteOffset] & 0x7FFF;
	}
}

/*
* Writes the lzss encoded image to the target buffer in memory
* @param imageStart - in: pointer to starting position of image data
* @param imageEnd - in: pointer to end position of image data
* @param targetMem - out: pointer to memory buffer to write uncompressed image data
*/
void loadLzssToRam(const char *imageStart, const char *imageEnd, vu8 *targetMem)
{
	//decode the image
	lzss_decode(imageStart, (int)((imageEnd)-(imageStart)), (vu16*) targetMem);	
}

void drawLzssBG2(const char *imageStart, const char *imageEnd, int fbOffset){
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	//vu16 *cram16 = &MARS_CRAM;
	//vu16 *pal16 = (vu16*) paletteStart;
	
	//int numColors;
	//int i;
	const u16 lineTableEnd = 0x100;
	//uint16 lineOffs;

	// Wait for the SH2 to gain access to the VDP
	//while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) {}

	// Set 8-bit paletted color mode, 224 lines
	//MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;
	/*
	numColors = ((paletteEnd)-(paletteStart)) / 2;
	for (i = paletteOffset; i < numColors+paletteOffset; i++)
	{
		cram16[i] = pal16[i-paletteOffset] & 0x7FFF;
	}*/
	//loadPalette(paletteStart, paletteEnd, paletteOffset);

	//MARS_VDP_FBCTL = currentFB;
	
	//decode lzss image to spriteBuffer
	lzss_decode(imageStart, (int)((imageEnd)-(imageStart)), frameBuffer16  + lineTableEnd + fbOffset);
	//lzss_decode(&ninjastanding, (int)((&ninjastanding_end)-(&ninjastanding)), (vu16*) &spriteBuffer);
	
}

/*
* Writes the lzss encoded image to the framebuffer, assuming the image will be fullscreen (320 x 224)
* @param imageStart - pointer to starting position of image data
* @param imageEnd - pointer to end position of image data
*/
void drawLzssBG(const char *imageStart, const char *imageEnd)
{
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	//vu16 *cram16 = &MARS_CRAM;
	//vu16 *pal16 = (vu16*) paletteStart;
	
	//int numColors;
	int i;
	const u16 lineTableEnd = 0x100;
	u16 lineOffs;

	// Wait for the SH2 to gain access to the VDP
	//while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) {}

	// Set 8-bit paletted color mode, 224 lines
	//MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;
	/*
	numColors = ((paletteEnd)-(paletteStart)) / 2;
	for (i = paletteOffset; i < numColors+paletteOffset; i++)
	{
		cram16[i] = pal16[i-paletteOffset] & 0x7FFF;
	}*/
	//loadPalette(paletteStart, paletteEnd, paletteOffset);

	//MARS_VDP_FBCTL = currentFB;
	
	//decode lzss image to spriteBuffer
	lzss_decode(imageStart, (int)((imageEnd)-(imageStart)), frameBuffer16  + 0x100);
	//lzss_decode(&ninjastanding, (int)((&ninjastanding_end)-(&ninjastanding)), (vu16*) &spriteBuffer);
	
	// Set up the line table
	lineOffs = lineTableEnd;
	for (i = 0; i < 256; i++)
	{
		frameBuffer16[i] = lineOffs;
		lineOffs += 160;
	}
}



/*
* Draws a compressed image to position on MARS framebuffer
* @param spriteStart - pointer to starting position of image data
* @param spriteEnd - pointer to end position of image data
* @param x - x pixel coordinate of top-left corner of the image 
* @param y - y pixel coordinate of top-left corner of the image
* @param xWidth - vertical size of image to be drawn in pixels
* @param yWidth - horizontal size of image to be drawn in pixels 
*/
void drawLzssSprite(char *spriteStart, char *spriteEnd, vu16 x, vu16 y, int xWidth, int yWidth)
{
	//uint16 currentFB=0;
	u16 lineOffs;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	//vu16 *cram16 = &MARS_CRAM;
	//vu16 *pal16 = (vu16*) paletteStart;
	vu16 spriteBuffer[20000];
	int size = (xWidth * yWidth) / 2;
	int drawWidth = 0;
	
	//int numColors;
	vu16 xOff;
	int i;
	int bufCnt;
	
	const u16 lineTableEnd = 0x100;
	u16 fbOff = lineTableEnd;
	
	
	
	// Wait for the SH2 to gain access to the VDP
	//while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) {}

	// Set 8-bit paletted color mode, 224 lines
	//MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;

	/*numColors = ((paletteEnd)-(paletteStart)) / 2;
	for (i = 0; i < numColors; i++)
	{
		cram16[i] = pal16[i] & 0x7FFF;
	}*/

	//MARS_VDP_FBCTL = currentFB;
	//offset the number of pixels in each line to start to draw the image
	xOff = x / 2;
	
	//decode lzss image to spriteBuffer
	lzss_decode(spriteStart, (int)((spriteEnd)-(spriteStart)), (vu16*) &spriteBuffer);
	//lzss_decode(&ninjastanding, (int)((&ninjastanding_end)-(&ninjastanding)), (vu16*) &spriteBuffer);
	
    //for(;;)
    //{
		//MARS_VDP_FBCTL = currentFB ^ 1;
		//while ((MARS_VDP_FBCTL & MARS_VDP_FS) == currentFB) {}
		//currentFB ^= 1;
		
		fbOff = lineTableEnd;
		//y-offset for top of sprite to correct line in framebuffer
		fbOff = fbOff + (y * 160);
		//x-offset from start of first line
		fbOff = fbOff + xOff;
		//draw spriteBuffer to the framebuffer
		drawWidth = 0;
		for (bufCnt = 0; bufCnt < size; bufCnt++)
		{
			//write word to framebuffer
			if(spriteBuffer[bufCnt] > TRANSPARENT_PIXEL_COLOR)
			{
				frameBuffer16[fbOff] = spriteBuffer[bufCnt];
			}
			//reset spriteBuffer
			spriteBuffer[bufCnt] = 0;
			fbOff++;
			drawWidth++;
			if(drawWidth >= (xWidth/2)){
				//reset line if past the width of the image
				drawWidth = 0;
				fbOff = fbOff + (160 - ((xWidth/2) + xOff)) + xOff;
			}
		}
		
		bufCnt = 0;
		drawWidth = 0;
		
		// Set up the line table
		lineOffs = lineTableEnd;
		for (i = 0; i < 256; i++)
		{
			frameBuffer16[i] = lineOffs;
			lineOffs += 160;
		}
    //}
	//clear spriteBuffer
	for(i = size; i < 20000; i++){
		spriteBuffer[i] = 0;
	}
}

/*
* Draws a compressed image to position on MARS framebuffer allowing you to flip the image using mirror param.
* 
* @param spriteStart - pointer to starting position of image data
* @param spriteEnd - pointer to end position of image data
* @param x - x pixel coordinate of top-left corner of the image 
* @param y - y pixel coordinate of top-left corner of the image
* @param xWidth - vertical size of image to be drawn in pixels
* @param yWidth - horizontal size of image to be drawn in pixels
* @param mirror - 0 for normal 1 for flipped along y-axis  
*/
void drawLzssSprite2(char *spriteStart, char *spriteEnd, vu16 x, vu16 y, int xWidth, int yWidth, int mirror)
{

	//each byte represents the color for each pixel. Allows us to reverse which we can't do with words
	vu8 *frameBuffer8 = (vu8*) &MARS_FRAMEBUFFER;
	vu8 spriteBuffer2[40000];
	int size = (xWidth * yWidth);
	//int drawWidth = 0;
	vu16 xOff;
	int i;
	int bufCnt;
	int colPos,rowPos,yCount,xCount;
	const u16 lineTableEnd = 0x100;
	u32 fbOff = lineTableEnd;
	
	
	
	
	// Wait for the SH2 to gain access to the VDP
	//while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) {}


	//offset the number of pixels in each line to start to draw the image
	xOff = x;
	
	//decode lzss image to spriteBuffer
	lzss_decode(spriteStart, (int)((spriteEnd)-(spriteStart)), (vu16*) &spriteBuffer2);
		
		fbOff = lineTableEnd * 2;
		//y-offset for top of sprite to correct line in framebuffer
		fbOff = fbOff + (y * 320);
		//x-offset from start of first line
		fbOff = fbOff + xOff;
		//draw spriteBuffer to the framebuffer
		//drawWidth = 0;
		bufCnt = 0;
		colPos = 0;
		yCount = 0;
		xCount = 0;
		rowPos = 0;
		for (rowPos = 0; rowPos < yWidth; rowPos++)
		{
			//for(xCount = 0; xCount < (xWidth / 2); xCount++)
			for(xCount = 0; xCount < (xWidth ); xCount++)
			{		
				//if mirror is 1 that tells us to flip the column
				if(mirror == 1){
					colPos = xWidth - xCount;
				}else{
					colPos = xCount;
				}
				//bufCnt = yPos*(xWidth / 2) + xPos;
				bufCnt = rowPos * (xWidth) + colPos;

				//write byte to framebuffer if not transparent
				if(spriteBuffer2[bufCnt] > TRANSPARENT_PIXEL_COLOR)
				{
					frameBuffer8[fbOff] = spriteBuffer2[bufCnt];
				}
				//reset spriteBuffer
				spriteBuffer2[bufCnt] = 0;
				fbOff++;
			}
			//reset the "line" in framebuffer if past the width of the image
			fbOff = fbOff + (320 - ((xWidth) + xOff)) + xOff;
		}
		
		bufCnt = 0;
		//drawWidth = 0;
		
		// Set up the line table
		//lineOffs = lineTableEnd;
		//for (i = 0; i < 256; i++)
		//{
		//	frameBuffer16[i] = lineOffs;
		//	lineOffs += 160;
		//}
    //}
	//clear remaining data in spriteBuffer
	for(i = size; i < 40000; i++){
		spriteBuffer2[i] = 0;
	}
}


/* 
* Draws blank pixels to rectangle specified by x, y, xWidth and yWidth (height)
*/
void clearArea(vu16 x, vu16 y, int xWidth, int yWidth){
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	vu16 xOff;
	//int i;
	int bufCnt;
	int size = (xWidth * yWidth) / 2;
	const u16 lineTableEnd = 0x100;
	u16 fbOff = lineTableEnd;
	int drawWidth = 0;
	
	//offset the number of pixels in each line to start to draw the image
	xOff = x / 2;
	
	fbOff = lineTableEnd;
	//y-offset for top of sprite to correct line in framebuffer
	fbOff = fbOff + (y * 160);
	//x-offset from start of first line
	fbOff = fbOff + xOff;
	//draw spriteBuffer to the framebuffer
	drawWidth = 0;
	for (bufCnt = 0; bufCnt < size; bufCnt++)
	{
		//write word to framebuffer
		frameBuffer16[fbOff] = TRANSPARENT_PIXEL_COLOR;
		fbOff++;
		drawWidth++;
		if(drawWidth >= (xWidth/2)){
			//reset line
			drawWidth = 0;
			fbOff = fbOff + (160 - ((xWidth/2) + xOff)) + xOff;
		}
	}
}




/*
* Draws an image to position on MARS framebuffer allowing you to flip the image using mirror param.
* 
* @param spriteBuffer - pointer to starting position of image data
* @param x - x pixel coordinate of top-left corner of the image 
* @param y - y pixel coordinate of top-left corner of the image
* @param xWidth - vertical size of image to be drawn in pixels
* @param yWidth - horizontal size of image to be drawn in pixels
* @param mirror - 0 for normal 1 for flipped along y-axis
* @param checkTransparency - 0 for not checked ie overwrite every pixel, including with zero, 1 for checking
* @param screenWrap - 0 for no screenWrap, 1 for screen wrapping  
*/
int drawSpriteMaster(const vu8 *spriteBuffer, const s16 x, const s16 y, const int xWidth, const int yWidth, const int mirror, const int checkTransparency, const int screenWrap)
{

	//MARS_VDP_MODE_256
	//each byte represents the color in CRAM for each pixel.
	vu8 *frameBuffer8 = NULL;//(vu8*) &MARS_FRAMEBUFFER;
	int xOff;
	int bufCnt=0;
	int rowPos=0;
	int xCount=0;
	int xOverflow=0;
	int lineEnd = 0;
	int spriteModEight = 0;
	int spriteStart = 0;
	int absX = 0;
	//Line table is 256 words ie 256 * 2 bytes
	const u16 lineTableEnd = 0x200;
	int fbOff;
	int p=0;
	//TODO this is always 1 (for 8 byte segments using the word_8byte copy functions
	const int pixelWriteBufferSizeWords =  1;//PIXEL_WRITE_BUFFER_SIZE_B/2;
	
	//overwrite buffer - ie zero is not written - what you need if you want transparency on sprites
	//TODO might have some problems if over screen edges?
	if(checkTransparency == IS_TRANSPARENT){
		frameBuffer8 = (vu8* ) &MARS_OVERWRITE_IMG;
	}else{
		frameBuffer8 = (vu8* ) &MARS_FRAMEBUFFER;
	}

	//offset the number of pixels in each line to start to draw the image
	xOff = (int)x;
	//if off the left edge of the screen, special care is needed
	if(x < -PIXEL_WRITE_BUFFER_SIZE_B){
		//need to draw in multiples of 8 so find, the portion of the sprite image to draw
		//offset the start of the line to compensate
		//this code fixed a glitch where you would see artifacts of the left edge of image off to the right
		absX = abs(x);
		spriteModEight = x%PIXEL_WRITE_BUFFER_SIZE_B;
		spriteStart = absX + spriteModEight;
		xOff = spriteModEight;
	}
	
	
	//move the framebuffer offset to start of the visible framebuffer?? 
	//Line table is 256 words ie 256 * 2 bytes
	fbOff = lineTableEnd;// - ( PIXEL_WRITE_BUFFER_SIZE_B - 1 );
	//y-offset for top of sprite to correct line in framebuffer
	fbOff = fbOff + (((int)y * (SCREEN_WIDTH + 16)) + 8);
	//x-offset from start of first line
	fbOff = fbOff + xOff;
	bufCnt = 0;
	xCount = 0;
	rowPos = 0;
	
	//if the image is totally off the left side of the screen skip it
	if( (xWidth + x) < 0 || x > SCREEN_WIDTH){
		return -1;
	}
	
	//loop for all the rows
	for (rowPos = 0; rowPos < yWidth; rowPos++)
	{
		p = 0;
		
		if(mirror == IS_MIRRORED){
			//increment a row
			bufCnt = bufCnt + xWidth;
		}
		
		xCount = 0;
		lineEnd = 0;
		//if off the left edge of the screen + our buffer, we should skip to the correct part of the spriteBuffer for each row
		if(x < -PIXEL_WRITE_BUFFER_SIZE_B){
			xCount = spriteStart;
			lineEnd = -PIXEL_WRITE_BUFFER_SIZE_B;
		}
		//for the row iterate over the columns
		for( ; xCount < xWidth; xCount+=PIXEL_WRITE_BUFFER_SIZE_B)
		{
			lineEnd += PIXEL_WRITE_BUFFER_SIZE_B;
			xOverflow = 0;
			//if mirror is 1 that tells us to flip the column
			if(mirror == IS_MIRRORED){
				//copy the next 8 bytes in reverse
				word_8byte_copy_bytereverse((void *)(frameBuffer8+fbOff), (void *)(&spriteBuffer[bufCnt-(xCount+PIXEL_WRITE_BUFFER_SIZE_B)]), pixelWriteBufferSizeWords);
				
			}else{
				//copy the next 8 bytes
				word_8byte_copy((void *)(frameBuffer8+fbOff), (void *)(&spriteBuffer[bufCnt+xCount]), pixelWriteBufferSizeWords);
				
			}
			
			
			//don't draw if you've gone over the screenwidth to the right side
			if(xOff + xCount + PIXEL_WRITE_BUFFER_SIZE_B > (SCREEN_WIDTH)){
				xOverflow = SCREEN_WIDTH - (xOff+xCount);
				//advance up to the end of this row
				//fbOff += PIXEL_WRITE_BUFFER_SIZE_B - (PIXEL_WRITE_BUFFER_SIZE_B - xOverflow);
				xCount = xWidth;
				
			} // if drawing something where x is off the left side 
			else if(x < -PIXEL_WRITE_BUFFER_SIZE_B) { 
				//increment to next position in FrameBuffer
				fbOff += PIXEL_WRITE_BUFFER_SIZE_B;//- xOverflow;
				xOverflow = SCREEN_WIDTH - (xOff+lineEnd) - PIXEL_WRITE_BUFFER_SIZE_B;
			} // default case
			else{
				//increment to next position in FrameBuffer
				fbOff += PIXEL_WRITE_BUFFER_SIZE_B;//- xOverflow;
				xOverflow = SCREEN_WIDTH - (xOff+xCount) - PIXEL_WRITE_BUFFER_SIZE_B;
			}

		}//end for xCount
		
		//increment a row if not "reversed" 
		if(mirror != IS_MIRRORED){
			bufCnt = bufCnt + xWidth;
		}
		//=((E10+G10+(16-G10)))-(C10+B10)+C10
		//reset the "line" in framebuffer if past the width of the image
		//fbOff = fbOff + ((SCREEN_WIDTH + xOverflow+(16-xOverflow)) - (xWidth + xOff)) + xOff;
		fbOff += (xOverflow + FRAMEBUFFER_ROW_EXTENSION) +xOff;
		
	}//end for rowPos
	//write any "leftover pixels? shouldn't happen
	
	return 0;
	
}

/*
* Draws an image to position on MARS framebuffer allowing you to flip the image using mirror param.
* 
* @param spriteBuffer - pointer to starting position of image data to read
* @param x - x pixel coordinate of top-left corner of the image 
* @param y - y pixel coordinate of top-left corner of the image
* @param xWidth - vertical size of image to be drawn in pixels
* @param yWidth - horizontal size of image to be drawn in pixels
* @param mirror - 0 for normal 1 for flipped along y-axis
* @param screenWrap - 0 for no screenWrap, 1 for screen wrapping    
*/
int drawSprite(const vu8 *spriteBuffer, const s16 x, const s16 y, const int xWidth, const int yWidth, const int mirror, const int screenWrap)
{
	//call drawSprite with transparency enabled
	return drawSpriteMaster(spriteBuffer,  x, y, xWidth, yWidth, mirror, 1, screenWrap);
}

void drawS(vu8* spriteBuffer, u16 x, u16 y, u16 xWidth, u16 yWidth)
{
   vu8 *frameBuffer = (vu8*) &MARS_OVERWRITE_IMG;
   // dst frame buffer pointer (X + Y offseted)
   vu8* dst = &frameBuffer[0x100 + (y * 320) + (x + 256)];
   // src sprite pointer
   vu8* src = spriteBuffer;

   const u16 xw = xWidth;
   const int dstStep = 320 - xw;
   
   u16 row = yWidth;
   
   while (row--)
   {
      u16 col = xw;
     
      while(col--) *dst++ = *src++;
     
      dst += dstStep;
   }
}



/*
* Draws a background image on MARS framebuffer allowing you to flip the image using mirror param, no transparency
* 
* @param spriteBuffer - pointer to starting position of image data
*
*/
int drawBG(const vu8 *spriteBuffer)
{
	//draw full screen background image with no transparency
	return drawSpriteMaster(spriteBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
}

/* 
* Draws pixels to fill rectangle specified by x, y, xWidth and yWidth (height). Must be on screen.
* xWidth must be sized in multiples of 8 wide (8 pixels, 16 pixels, etc..)
*
*/
void drawFillRect(const s16 x, const s16 y, const int xWidth, const int yWidth, vu8* color){
	//MARS_VDP_MODE_256
	//each byte represents the color in CRAM for each pixel.
	vu8 *frameBuffer8 = (vu8* )&MARS_FRAMEBUFFER;
	int xOff;
	int rowPos=0;
	int xCount=0;
	const u16 lineTableEnd = 0x200;
	const int pixelWriteBufferSizeWords =  1;
	int fbOff = 0;
	//uint8 color[8] = {8,8,8,8,8,8,8,8};
	
	//offset the number of pixels in each line to start to draw the image
	xOff = (int)x;
	//move the framebuffer offset to start of the visible framebuffer?? 
	//Line table is 256 words ie 256 * 2 bytes
	fbOff = lineTableEnd;// - ( PIXEL_WRITE_BUFFER_SIZE_B - 1 );
	//y-offset for top of sprite to correct line in framebuffer
	fbOff = fbOff + (((int)y * (SCREEN_WIDTH + 16)) + 8);
	//x-offset from start of first line
	fbOff = fbOff + xOff;
	

	//draw rectangle to the framebuffer
	//loop for all the rows (y-axis)
	for (rowPos = 0; rowPos < yWidth; rowPos++)
	{
		//for the row iterate over the columns (x-axis)
		for(xCount=0 ; xCount < xWidth; xCount+=PIXEL_WRITE_BUFFER_SIZE_B)
		{
			//copy the color to framebuffer
			word_8byte_copy((void *)(frameBuffer8+fbOff), (void *)(color), pixelWriteBufferSizeWords);
			fbOff += PIXEL_WRITE_BUFFER_SIZE_B;
		}
		//reset framebuffer offset to next line
		fbOff += (SCREEN_WIDTH - (xOff + xWidth) + FRAMEBUFFER_ROW_EXTENSION) + xOff;
	}
}

/* 
* Draws pixels to outline a rectangle specified by x, y, xWidth and yWidth (height). Must be on screen.
* xWidth must be sized in multiples of 8 wide (8 pixels, 16 pixels, etc..)
*
*/
void drawRect(const s16 x, const s16 y, const int xWidth, const int yWidth, vu8* color){
	//MARS_VDP_MODE_256
	//each byte represents the color in CRAM for each pixel.
	vu8 *frameBuffer8 = (vu8* )&MARS_FRAMEBUFFER;
	int xOff;
	int rowPos=0;
	int xCount=0;
	const u16 lineTableEnd = 0x200;
	const int pixelWriteBufferSizeWords =  1;
	int fbOff = 0;
	//uint8 color[8] = {8,8,8,8,8,8,8,8};
	
	//offset the number of pixels in each line to start to draw the image
	xOff = (int)x;
	//move the framebuffer offset to start of the visible framebuffer?? 
	//Line table is 256 words ie 256 * 2 bytes
	fbOff = lineTableEnd;// - ( PIXEL_WRITE_BUFFER_SIZE_B - 1 );
	//y-offset for top of sprite to correct line in framebuffer
	fbOff = fbOff + (((int)y * (SCREEN_WIDTH + FRAMEBUFFER_ROW_EXTENSION)) + PIXEL_WRITE_BUFFER_SIZE_B);
	//x-offset from start of first line
	fbOff = fbOff + xOff;
	

	//draw rectangle to the framebuffer
	//loop for all the rows (y-axis)
	for (rowPos = 0; rowPos < yWidth; rowPos++)
	{
		//draw horizontal line
		if(rowPos == 0 || rowPos + 1 == yWidth)
		{
			//for the row iterate over the columns (x-axis)
			for(xCount=0 ; xCount < xWidth; xCount+=PIXEL_WRITE_BUFFER_SIZE_B)
			{
				//copy the color to framebuffer
				word_8byte_copy((void *)(frameBuffer8+fbOff), (void *)(color), pixelWriteBufferSizeWords);
				fbOff += PIXEL_WRITE_BUFFER_SIZE_B;
			}
		}
		else //draw pixel at start of line and at end
		{
			frameBuffer8[fbOff] = color[0];
			//skip to end of line
			fbOff += xWidth;
			frameBuffer8[fbOff-1] = color[0];
		}
		//reset framebuffer offset to next line
		fbOff += (SCREEN_WIDTH - (xOff + xWidth) + FRAMEBUFFER_ROW_EXTENSION) + xOff;
	}
}

extern unsigned char msx[];
void my_debug_put_char_8(const int x,const int y,const unsigned char ch, const vu8* fgColor, const vu8* bgColor)
{
    vu8 *fb = (vu8*)&MARS_FRAMEBUFFER;
    int i,j;
    u8 *font;
    int vram, vram_ptr;

    vram = 0x200 + x;
    vram += (y *(SCREEN_WIDTH + FRAMEBUFFER_ROW_EXTENSION))+8;

    font = &msx[ (int)ch * 8];

    for (i=0; i<8; i++, font++)
    {
        vram_ptr  = vram;
        for (j=0; j<8; j++)
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

/* Print non-nul terminated strings */
int myScreenPrintData(const char *buff, const int x, const int y, const vu8* fgColor, const vu8* bgColor )
{
    int i;
    char c;
	int size;
	int xOff;
	
	size = strlen(buff);
	xOff = x;

    for (i = 0; i<size; i++)
    {
        c = buff[i];
        my_debug_put_char_8(xOff,y,c, fgColor, bgColor);
		//move 8 bytes
		xOff+=PIXEL_WRITE_BUFFER_SIZE_B-1;
    }

    return i;
}

/*
* Draw line table to framebuffer
* @param xOff - the x offset in words for each line in the line table
*/
void drawLineTable(const int xOff){
	int i;
	//screen line width in words plus 4 extra words on each side
	const int lineWidth = 160 + 8;
	vu16 *frameBuffer16 = &MARS_FRAMEBUFFER;
	const u16 lineTableEnd = 0x100;
	u16 lineOffs = lineTableEnd + xOff;
	
	
	// Set up the line table
	for (i = 0; i < 256; i++)
	{
		
		frameBuffer16[i] = lineOffs;
		//this made a warping effect!
		//lineOffs += xOff + (SCREEN_WIDTH / 2);
		lineOffs += lineWidth;
	}
}

void mars_drawText(const char *str, int x, int y, int palOffs)
{
	int c;
	vu8 *fb = (vu8 *)&MARS_FRAMEBUFFER + 0x100;
	unsigned char *font;
	int screenOffs;
	int fontOffs;
	static unsigned short fc = 0;
	extern u8 FONT_HIGHLIGHT_TILE[];

	for (int i = 0; i < 40; i++) {
		c = str[i];
		if (!c) break;

		c -= ' ';
		screenOffs = y*320 + i * 8 + x;
		fontOffs = (c >> 4) << 10;
		fontOffs += (c & 15) << 3;
		for (int t = 0; t < 8; t++) {
			for (int s = 0; s < 8; s++) {
				font = FONT_HIGHLIGHT_TILE[fontOffs + s];
				if (font) font += palOffs;
				if (FONT_HIGHLIGHT_TILE[fontOffs + s])
					fb[screenOffs + s] = FONT_HIGHLIGHT_TILE[fontOffs + s] + palOffs;
			}
			screenOffs += 320;
			fontOffs += 128;
		}
	}
}

void mars_drawTextwShadow(const char *str, int x, int y, int textpalOffs, int shadowpalOffs)
{
	mars_drawText(str, x+2, y+1, shadowpalOffs);
	mars_drawText(str, x, y, textpalOffs);
}

void screenFadeOut(int fadeSpeed)
{
	int frameDelay = fadeSpeed;
	int len = 31;
	int r,g,b;
	u16 tempcolor;
	vu16 temppal[256];
	vu16 *cram16 = &MARS_CRAM;
	while(len != 0)
	{
		for (int i = 0; i <= 255; i++) {
			temppal[i] = cram16[i];
		}
	
		for (int i = 0; i <= 255; i++) {
			tempcolor = temppal[i] & 0x7FFF;
			if(tempcolor != 0x0000) {
				r = tempcolor & 0x1F;
				g = tempcolor>>5 & 0x1F;
				b = tempcolor>>10 & 0x1F;
				if(r != 0x0000)
					r--;
				if(g != 0x0000)
					g--;
				if(b != 0x0000)
					b--;
				tempcolor = COLOR(r,g,b);
				temppal[i] = tempcolor & 0x7FFF;
			}
		}
		for (int i = 0; i <= 255; i++){
			cram16[i] = temppal[i] & 0x7FFF;
		}
		Hw32xDelay(frameDelay);
			
		currentFB ^= 1;
		MARS_VDP_FBCTL = currentFB;

		len--;
	}
	Hw32xScreenClear();
	return;
}