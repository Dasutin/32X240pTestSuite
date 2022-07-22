/*
 * Licensed under the BSD license
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * 32X by Chilly Willy
 */
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "32x.h"
#include "hw_32x.h"
#include "string.h"
#include "shared_objects.h"
#include "draw.h"
#include "dtiles.h"
#include "sound.h"
#include "mars_ringbuf.h"

extern int fontColorWhite;
extern int fontColorRed;
extern int fontColorGreen;
extern int fontColorGray;
extern int fontColorBlack;

static int X = 0, Y = 0;
static int MX = 40, MY = 25;
static int init = 0;
static unsigned short fgc = 0, bgc = 0;
static unsigned char fgs = 0, bgs = 0;

static volatile const uint8_t *new_palette;

int sysarg_args_nosound = 0;
int sysarg_args_vol = 0;

int nodraw = 0;

int32_t canvas_width = 320; // +4 to avoid hitting that 0xXXFF bug in the shift register
int32_t canvas_height = 224;

extern drawsprcmd_t slave_drawsprcmd;
extern drawspr4cmd_t slave_drawspr4cmd;
extern drawtilelayerscmd_t slave_drawtilelayerscmd;

static volatile unsigned int mars_vblank_count = 0;

#define UNCACHED_CURFB (*(short *)((int)&currentFB|0x20000000))

u32 schecksum = 0;

volatile unsigned short dmaDone = 1;

void pri_vbi_handler(void)
{
    mars_vblank_count++;
    
    if (new_palette)
	{
        int i;
        volatile unsigned short *palette = &MARS_CRAM;

        if ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0)
		    return;

        for (i = 0; i < 256; i++)
        {
             palette[i] = COLOR(new_palette[0] >> 3, new_palette[1] >> 3, new_palette[2] >> 3);
             new_palette += 3;
        }

        new_palette = NULL;
	}
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
    int PAL_MODE = 1;

    return PAL_MODE;
}

void Hw32xSetFGColor(int s, int r, int g, int b)
{
    volatile unsigned short *palette = &MARS_CRAM;
    fgs = s;
    fgc = COLOR(r, g, b);
    palette[fgs] = fgc;
}

void Hw32xSetBGColor(int s, int r, int g, int b)
{
    volatile unsigned short *palette = &MARS_CRAM;
    bgs = s;
    bgc = COLOR(r, g, b);
    palette[bgs] = bgc;
}

void Hw32xSetPalette(const uint8_t *palette)
{
    new_palette = palette;
}

void Hw32xUpdateLineTable(int hscroll, int vscroll, int lineskip)
{
    int i;
    int i_lineskip;
    const int ymask = canvas_yaw - 1;
    const int pitch = canvas_pitch >> 1;
    uint16_t *frameBuffer16 = (uint16_t *)&MARS_FRAMEBUFFER;

    hscroll += 0x100;

    if (lineskip == 0)
    {
        unsigned count = canvas_height;
        unsigned n = ((unsigned)count + 7) >> 3;
        const int mpitch = pitch * canvas_yaw;

            #define DO_LINE() do { \
            if (ppitch >= mpitch) ppitch -= mpitch; \
            *frameBuffer16++ = ppitch + hscroll; /* word offset of line */ \
            ppitch += pitch; \
        } while(0)

        int ppitch = pitch * vscroll;
        switch (count & 7)
        {
        case 0: do { DO_LINE();
        case 7:      DO_LINE();
        case 6:      DO_LINE();
        case 5:      DO_LINE();
        case 4:      DO_LINE();
        case 3:      DO_LINE();
        case 2:      DO_LINE();
        case 1:      DO_LINE();
        } while (--n > 0);
        }
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
}

void Hw32xInit(int vmode, int lineskip)
{
    volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;
    int i;

    // Wait for the SH2 to gain access to the VDP
    while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) ;

    if (vmode == MARS_VDP_MODE_256)
    {
        // Set 8-bit paletted mode, 224 lines
        MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;

        // Initialize both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // Rewrite line table
        Hw32xUpdateLineTable(0, 0, lineskip);
        // Clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // Rewrite line table
        Hw32xUpdateLineTable(0, 0, lineskip);
        // Clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        MX = 40;
        MY = 28/(lineskip+1);
    }
    else if (vmode == MARS_VDP_MODE_32K)
    {
        // Set 16-bit direct mode, 224 lines
        MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_32K;

        // Initialize both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        for (i=0; i<canvas_height/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i* canvas_pitch + 0x100;   // Word offset of line
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i* canvas_pitch + 0x100;         // Word offset of line
                else
                    frameBuffer16[i] = 200* canvas_pitch + 0x100;       // Word offset of line
            }
        }
        // Clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // Rewrite line table
        for (i=0; i<canvas_height/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i* canvas_pitch + 0x100;   // Word offset of line
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i* canvas_pitch + 0x100;     // Word offset of line
                else
                    frameBuffer16[i] = 200* canvas_pitch + 0x100;   // Word offset of line
            }
        }
        // Clear screen
        for (i=0x100; i<0x10000; i++)
            frameBuffer16[i] = 0;

        MX = 40;
        MY = 25/(lineskip+1);
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
    if( x<MX && x>=0 )
        X = x;
    if( y<MY && y>=0 )
        Y = y;
}

void Hw32xScreenClear()
{
    int i;
    int l = (init == MARS_VDP_MODE_256) ? canvas_pitch *224/2 + 0x100 : canvas_pitch *200 + 0x100;
    volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;

    // Clear screen
    for (i=0x100; i<l; i++)
        frameBuffer16[i] = 0;

    // Flip the framebuffer selection bit and wait for it to take effect
    MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
    while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
    UNCACHED_CURFB ^= 1;

    // clear screen
    for (i=0x100; i<l; i++)
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
    ticks *= 17;            // NTSC
  else
    ticks *= 20;            // PAL 

  if (!ticks_base)
    ticks_base = ticks;

  return ticks - ticks_base;
}

void Hw32xSleep(int s)
{
  if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
    Hw32xDelay(s/17);       // NTSC
  else
    Hw32xDelay(s/20);       // PAL
}

extern unsigned char msx[];

static void debug_put_char_16(int x, int y, unsigned char ch)
{
    volatile unsigned short *fb = &MARS_FRAMEBUFFER;
    int i,j;
    unsigned char *font;
    int vram, vram_ptr;

    if(!init)
    {
        return;
    }

    vram = 0x100 + x * 8;
    vram += (y * 8 * 320);

    font = &msx[ (int)ch * 8];

    for (i=0; i<8; i++, font++)
    {
        vram_ptr  = vram;
        for (j=0; j<8; j++)
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
    int i,j;
    unsigned char *font;
    int vram, vram_ptr;

    if(!init)
    {
        return;
    }

    vram = 0x200 + x * 8;
    vram += (y * 8 * 320);

    font = &msx[ (int)ch * 8];

    for (i=0; i<8; i++, font++)
    {
        vram_ptr  = vram;
        for (j=0; j<8; j++)
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
    {
        debug_put_char_8(x, y, ch);
    }
    else if (init == MARS_VDP_MODE_32K)
    {
        debug_put_char_16(x, y, ch);
    }
}

void Hw32xScreenClearLine(int Y)
{
    int i;

    for (i=0; i < MX; i++)
    {
        Hw32xScreenPutChar(i, Y, ' ');
    }
}

// Print non-nul terminated strings
int Hw32xScreenPrintData(const char *buff, int size)
{
    int i;
    char c;

    if(!init)
    {
        return 0;
    }

    for (i = 0; i<size; i++)
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
   va_list  opt;
   char     buff[128];
   int      n;

   va_start(opt, format);
   n = vsnprintf(buff, (size_t)sizeof(buff), format, opt);
   va_end(opt);
   buff[sizeof(buff) - 1] = 0;

   Hw32xScreenPutsn(buff, n);
}

void Hw32xDelay(int ticks)
{
    unsigned long ct = mars_vblank_count + ticks;
    while (mars_vblank_count < ct) ;
}

void Hw32xScreenFlip(int wait)
{
    // Flip the framebuffer selection bit
    MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
    if (wait)
    {
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
    }
}

void Hw32xFlipWait()
{
    while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
    UNCACHED_CURFB ^= 1;
}

// Mega Drive Command Support Code ---------------------------------------------

unsigned short HwMdReadPad(int port)
{
    if (port == 0)
        return MARS_SYS_COMM8;
    else if (port == 1)
        return MARS_SYS_COMM10;
    else
        return SEGA_CTRL_NONE;
}

unsigned char HwMdReadSram(unsigned short offset)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = offset;
    MARS_SYS_COMM0 = 0x0100;                    // Read SRAM
    while (MARS_SYS_COMM0) ;
    return MARS_SYS_COMM2 & 0x00FF;
}

void HwMdWriteSram(unsigned char byte, unsigned short offset)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = offset;
    MARS_SYS_COMM0 = 0x0200 | byte;             // Write SRAM
    while (MARS_SYS_COMM0) ;
}

int HwMdReadMouse(int port)
{
    unsigned int mouse1, mouse2;
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM0 = 0x0500|port;               // Tell 68000 to read mouse
    while (MARS_SYS_COMM0 == (0x0500|port)) ;   // Wait for mouse value
    mouse1 = MARS_SYS_COMM0;
    mouse2 = MARS_SYS_COMM2;
    MARS_SYS_COMM0 = 0;                         // Tells 68000 we got the mouse value
    return (int)((mouse1 << 16) | mouse2);
}

void HwMdClearScreen(void)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM0 = 0x0600;                    // Clear Screen (Name Table B)
    while (MARS_SYS_COMM0) ;
}

void HwMdSetOffset(unsigned short offset)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = offset;
    MARS_SYS_COMM0 = 0x0700;                    // Set offset (into either Name Table B or VRAM)
    while (MARS_SYS_COMM0) ;
}

void HwMdSetNTable(unsigned short word)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = word;
    MARS_SYS_COMM0 = 0x0800;                    // Set word at offset in Name Table B
    while (MARS_SYS_COMM0) ;
}

void HwMdSetVram(unsigned short word)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = word;
    MARS_SYS_COMM0 = 0x0900;                    // Set word at offset in VRAM
    while (MARS_SYS_COMM0) ;
}

void HwMdPuts(char *str, int color, int x, int y)
{
    HwMdSetOffset(((y<<6) | x) << 1);
    while (*str)
        HwMdSetNTable(((*str++ - 0x20) & 0xFF) | color);
}

void HwMdPutc(char chr, int color, int x, int y)
{
    HwMdSetOffset(((y<<6) | x) << 1);
    HwMdSetNTable(((chr - 0x20) & 0xFF) | color);
}

void HwMdScreenPrintf(int color, int x, int y, const char *format, ...)
{
   va_list  opt;
   char     buff[128];
   int      n;

   va_start(opt, format);
   n = vsnprintf(buff, (size_t)sizeof(buff), format, opt);
   va_end(opt);
   buff[sizeof(buff) - 1] = 0;

   HwMdPuts(buff, color, x, y);
}

void HwMdSetPal(unsigned short pal)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = pal;                    
    MARS_SYS_COMM0 = 0x0A00;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
}

void HwMdSetColor(unsigned short color)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = color;                    
    MARS_SYS_COMM0 = 0x0B00;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
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
    while (MARS_SYS_COMM0) ;
}

void HwMdPSGSetVolume(unsigned short word)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = word;
    MARS_SYS_COMM0 = 0x0D00;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
}

void HwMdPSGSetChandVol(unsigned short channel, unsigned short vol)
{
    HwMdPSGSetChannel(channel);
    HwMdPSGSetVolume(vol);
}

void HwMdPSGSendTone(unsigned short value1, unsigned short value2)
{

    while (MARS_SYS_COMM0) ;
    MARS_SYS_COMM2 = value1;                    // Send first half of data
    MARS_SYS_COMM0 = 0x0E00;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
    MARS_SYS_COMM2 = value2;                    // Send second half of data
    MARS_SYS_COMM0 = 0x0F00;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
}

void HwMdPSGSendNoise(unsigned short word)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = word;
    MARS_SYS_COMM0 = 0x1000;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
}

void HwMdPSGSendEnvelope(unsigned short word)
{
    while (MARS_SYS_COMM0) ;                    // Wait until 68000 has responded to any earlier requests
    MARS_SYS_COMM2 = word;
    MARS_SYS_COMM0 = 0x1100;                    // Send handle request flag
    while (MARS_SYS_COMM0) ;
}

void HwMdPSGSetFrequency(u8 channel, u16 value)
{
    u16 data;

    if (value)
    {
        if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT) data = 3579545 / (value * 32);
        else data = 3546893 / (value * 32);
        data = 3579545 / (value * 32);
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

// --------Put Secondary Calls here ---------

int secondary_task(int cmd)
{
    switch (cmd) {
    case 1:
        snddma_secondary_init(22050);
        return 1;
    case 2:
        return 1;
    case 3:
        ClearCacheLines(&slave_drawsprcmd, (sizeof(drawsprcmd_t) + 15) / 16);
        draw_handle_drawspritecmd(&slave_drawsprcmd);
        return 1;
    case 4:
        return 1;
    case 5:
        ClearCacheLines((uintptr_t)&canvas_width & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_height & ~15, 1);
        ClearCacheLines((uintptr_t)&window_canvas_x & ~15, 1);
        ClearCacheLines((uintptr_t)&window_canvas_y & ~15, 1);
        ClearCacheLines((uintptr_t)&old_camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&old_camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_pitch & ~15, 1);
        ClearCacheLines((uintptr_t)&canvas_yaw & ~15, 1);
        ClearCacheLines((uintptr_t)&camera_x & ~15, 1);
        ClearCacheLines((uintptr_t)&camera_y & ~15, 1);
        ClearCacheLines((uintptr_t)&nodraw & ~15, 1);
        ClearCacheLines(&slave_drawtilelayerscmd, (sizeof(drawtilelayerscmd_t) + 15) / 16);
        ClearCacheLines(&tm, (sizeof(tilemap_t) + 15) / 16);
        draw_handle_layercmd(&slave_drawtilelayerscmd);
        return 1;
    case 6: 
        Hw32xAudioCallback((unsigned long)&sndbuf);
        MARS_SYS_COMM4 = 7;
        return 1;
    case 7:
        SH2_DMA_SAR1 = ((unsigned long)&sndbuf) | 0x20000000;
        SH2_DMA_TCR1 = NUM_SAMPLES; // number longs
        SH2_DMA_CHCR1 = 0x18E1; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled
        return 1;
    case 8:
        MARS_SYS_COMM4 = 6;
        Hw32xAudioCallback((unsigned long)&sndbuf + MAX_NUM_SAMPLES * 4);
        MARS_SYS_COMM4 = 7;
        return 1;
    default:
        break;
    }

    return 0;
}

void secondary(void)                            // Slave waiting for commands (called by crt0.s)
{
    ClearCache();

    while (1) {
        int cmd;

        while ((cmd = MARS_SYS_COMM4) == 0) {}

        int res = secondary_task(cmd);
        if (res > 0) {
            MARS_SYS_COMM4 = 0;
        }
    }
}




