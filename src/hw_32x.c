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
#include "string.h"
#include "shared_objects.h"

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

int sysarg_args_nosound = 0;
int sysarg_args_vol = 0;

uint32_t canvas_width = 320+4; // +4 to avoid hitting that 0xXXFF bug in the shift register
uint32_t canvas_height = 224;

// 384 seems to be the ideal value - anything thing 
// increases the odds of hitting the "0xFF screen shift
// register bug"
uint32_t canvas_pitch = 320; // canvas_width + scrollwidth
uint32_t canvas_yaw = 224; // canvas_height + scrollheight

static volatile unsigned int mars_vblank_count = 0;

#define UNCACHED_CURFB (*(short *)((int)&currentFB|0x20000000))

u32 schecksum = 0;

volatile unsigned short dmaDone = 1;

void pri_vbi_handler(void)
{
    mars_vblank_count++;
}

void pri_dma1_handler(void)
{
    SH2_DMA_CHCR1; // Read TE
    SH2_DMA_CHCR1 = 0; // Clear TE
}

int Hw32xDetectPAL()
{
    int PAL_MODE = 1;

    //int PAL_VIDEO_MODE = MARS_VDP_DISPMODE & MARS_PAL_FORMAT;

    //if (PAL_VIDEO_MODE)
        //PAL_MODE = 1;
    //else
        //PAL_MODE = 0;

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

/* void Hw32xInit(int vmode, int lineskip)
{
    volatile unsigned short *frameBuffer16 = &MARS_FRAMEBUFFER;
    int i;

    // Wait for the SH2 to gain access to the VDP
    while ((MARS_SYS_INTMSK & MARS_SH2_ACCESS_VDP) == 0) ;

    if (vmode == MARS_VDP_MODE_256)
    {
        // Set 8-bit paletted mode, 224 lines
        MARS_VDP_DISPMODE = MARS_224_LINES | MARS_VDP_MODE_256;

        // Init both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // rewrite line table
        for (i=0; i<224/(lineskip+1); i++)
        {
            int j = lineskip + 1;
            while (j)
            {
                frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i*160 + 0x100; // Word offset of line
                j--;
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
        for (i=0; i<224/(lineskip+1); i++)
        {
            int j = lineskip + 1;
            while (j)
            {
                frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i*160 + 0x100; // Word offset of line
                j--;
            }
        }
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

        // Init both framebuffers

        // Flip the framebuffer selection bit and wait for it to take effect
        MARS_VDP_FBCTL = UNCACHED_CURFB ^ 1;
        while ((MARS_VDP_FBCTL & MARS_VDP_FS) == UNCACHED_CURFB) ;
        UNCACHED_CURFB ^= 1;
        // Rewrite line table
        for (i=0; i<224/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i*320 + 0x100; // Word offset of line
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i*320 + 0x100; // Word offset of line
                else
                    frameBuffer16[i] = 200*320 + 0x100; // Word offset of line
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
        for (i=0; i<224/(lineskip+1); i++)
        {
            if (lineskip)
            {
                int j = lineskip + 1;
                while (j)
                {
                    frameBuffer16[i*(lineskip+1) + (lineskip + 1 - j)] = i*320 + 0x100; // Word offset of line
                    j--;
                }
            }
            else
            {
                if (i<200)
                    frameBuffer16[i] = i*320 + 0x100; // Word offset of line
                else
                    frameBuffer16[i] = 200*320 + 0x100; // Word offset of line
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
} */

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
    int l = (init == MARS_VDP_MODE_256) ? 320*224/2 + 0x100 : 320*200 + 0x100;
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

/* void main_dma1_handler(void)
{
    SH2_DMA_CHCR1; // read TE
    SH2_DMA_CHCR1 = 0; // clear TE

    // Flip the framebuffer selection bit
    currentFB ^= 1;
    MARS_VDP_FBCTL = currentFB;

    dmaDone = 1;
}
 */

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

int secondary_task(int cmd)
{
    switch (cmd) {
    case 1:
        snddma_secondary_init(22050);
        return 1;
    case 2:
        return 1;
    case 3:
        /* ClearCacheLines(&slave_drawsprcmd, (sizeof(drawsprcmd_t) + 15) / 16);
        draw_handle_drawspritecmd(&slave_drawsprcmd);
        return 1; */
    case 4:
        return 1;
    case 5:
        /* ClearCacheLines((uintptr_t)&canvas_width & ~15, 1);
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
        draw_tile_layer(&slave_drawtilelayerscmd); */
        return 1;
    default:
        break;
    }

    return 0;
}

void secondary(void)
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

// Audio Data Loading Code -----------------------------------------------------------------------------------------

/* static int foffs[NUM_AUDIO_FILES];

// Open Data File
audio_file_t *audio_file_open(char *name)
{
    int ix;

    for (ix=0; ix<NUM_AUDIO_FILES; ix++)
        if (!strcasecmp(name, audioFileName[ix]))
        {
            foffs[ix] = 0;
            return (audio_file_t *)(ix + 1);
        }

    return (audio_file_t *)0;
}

// Size
//int data_file_size(data_file_t *file)
//{
//  return fileSize[(int)file - 1];
//}

// Seek
int audio_file_seek(audio_file_t *file, long offset, int origin)
{
    switch (origin)
    {
        case SEEK_SET:
            foffs[(int)file - 1] = offset;
            break;
        case SEEK_CUR:
            foffs[(int)file - 1] += offset;
            break;
        case SEEK_END:
            foffs[(int)file - 1] = audioFileSize[(int)file - 1] + offset;
            break;
    }

    return foffs[(int)file - 1];
}

// Tell
//int data_file_tell(data_file_t *file)
//{
//    return foffs[(int)file - 1];
//}

// Read
int audio_file_read(audio_file_t *file, void *buf, size_t size, size_t count)
{
    memcpy(buf, (char *)(audioFilePtr[(int)file - 1] + foffs[(int)file - 1]), size * count);
    return size * count;
}

// Memory Map
void *audio_file_mmap(audio_file_t *file, long offset)
{
    return (void *)(audioFilePtr[(int)file - 1] + offset);
}

// Audio Code -----------------------------------------------------------------------------------------

#define ADJVOL(S) ((S)*ssndVol)

unsigned short __attribute__((aligned(16))) sndbuf[MIXSAMPLES*2*2]; // two buffers of MIXSAMPLES words of stereo pwm audio

//short __attribute__((aligned(16))) sndbuf[MIXSAMPLES*2]; // two buffers of MIXSAMPLES words of stereo pwm audio

static channel_t __attribute__((aligned(16))) channel[MIXCHANNELS];

static unsigned char isAudioActive = FALSE;
static unsigned char sndMute = FALSE;  // Mute flag
static short sndVol = MAXVOL*2;  // Internal volume
static short sndUVol = MAXVOL;  // User-selected volume

static void end_channel(unsigned char);

// Callback -- This is also where all sound mixing is done

void Hw32xAudioCallback(unsigned long buff)
{
    unsigned char c;
    short s;
    unsigned int i;
    unsigned int *stream = (unsigned int *)(buff | 0x20000000);
    unsigned char sisAudioActive = *(unsigned char *)((unsigned int)&isAudioActive | 0x20000000);
    short ssndVol = *(short *)((unsigned int)&sndVol | 0x20000000);
    unsigned char ssndMute = *(unsigned char *)((unsigned int)&sndMute | 0x20000000);
    channel_t *schannel = (channel_t *)((unsigned int)channel | 0x20000000);

    if (sisAudioActive) {
        for (i = 0; i < MIXSAMPLES; i++) 
        {
            s = 0;
            for (c = 0; c < MIXCHANNELS; c++) 
            {
                if (schannel[c].loop != 0) // Channel is active
                {  
                    if (schannel[c].len > 0) // Not ending
                    {
                        s += ADJVOL(*schannel[c].buf - 0x80);
                        schannel[c].buf++;
                        schannel[c].len--;
                    }
                else 
                    {  // Ending
                        if (schannel[c].loop > 0) schannel[c].loop--;
                        if (schannel[c].loop) 
                        {  // Just loop
                            schannel[c].buf = schannel[c].snd->buf;
                            schannel[c].len = schannel[c].snd->len;
                            s += ADJVOL(*schannel[c].buf - 0x80);
                            schannel[c].buf++;
                            schannel[c].len--;
                        }
                    else 
                        {  
                        end_channel(c); // End for real
                        }
                    }
                }
            }
        if (ssndMute)
        stream[i] = ((unsigned long)516<<16) | (unsigned long)516;
        else 
        {
            s >>= 4;
            if (s > 512) s = 512;
            else if (s < -512) s = -512;
            s += 516;
            stream[i] = ((unsigned long)s<<16) | (unsigned long)s;
        }
        }
    } 
    else 
    {
        for (i = 0; i < MIXSAMPLES; i++)
        stream[i] = ((unsigned long)516<<16) | (unsigned long)516;
    }
}

static void end_channel(unsigned char c)
{
    channel[c].loop = 0;
    channel[c].snd = NULL;
}

void Hw32xAudioInit(void)
{
    unsigned char c;

    if (sysarg_args_vol != 0)
    {
        sndUVol = sysarg_args_vol;
        sndVol = sndUVol << 1;
    }

    for (c = 0; c < MIXCHANNELS; c++)
        channel[c].loop = 0;  // Deactivate

    isAudioActive = TRUE;
}

void Hw32xAudioShutdown(void)
{
    isAudioActive = FALSE;
}

//
// Toggle mute
//
// When muted, sounds are still managed but not sent to the dsp, hence
// it is possible to un-mute at any time.
//

 void Hw32xAudioToggleMute(void)
{
    sndMute = !sndMute;
}

void Hw32xAudioVolume(char d)
{
    if ((d < 0 && sndUVol > 0) || (d > 0 && sndUVol < MAXVOL)) 
    {
        sndUVol += d;
        sndVol = sndUVol << 1;
    }
}

//
// Play a sound
//
// loop: number of times the sound should be played, -1 to loop forever
// returns: channel number, or -1 if none was available
//
// NOTE if sound is already playing, simply reset it (i.e. can not have
// twice the same sound playing
//

/* char Hw32xAudioPlay(sound_t *sound, char loop, char selectch)
{
    unsigned char c;

    switch (selectch)
    {
        case 1:
            MARS_PWM_CTRL = 0x0182;  // Left Channel Only
        break;

        case 2:
            MARS_PWM_CTRL = 0x0184;  // Right Channel Only
        break;

        case 3:
            MARS_PWM_CTRL = 0x0185;  // Center
        break;
    }

    if (!isAudioActive) return -1;
    if (sound == NULL) return -1;

    while (MARS_SYS_COMM6 == 3);
    MARS_SYS_COMM6 = 2;

    c = 0;
    CacheClearLine(&channel[0]);

    while ((channel[c].snd != sound || channel[c].loop == 0) && channel[c].loop != 0 && c < MIXCHANNELS)
    {
        c++;
        CacheClearLine(&channel[c]);
    }

    if (c == MIXCHANNELS)
        c = -1;

    if (c >= 0) 
    {
        channel[c].loop = loop;
        channel[c].snd = sound;
        channel[c].buf = sound->buf;
        channel[c].len = sound->len;
    }

    MARS_SYS_COMM6 = 1;

    return c;
} */

/* // Pause
void Hw32xAudioPause(char pause)
{
    if (pause == TRUE)
        isAudioActive = FALSE;
    else
        isAudioActive = TRUE;
}

// Stop a channel
void Hw32xAudioStopChannel(unsigned char chan)
{
    if (chan < 0 || chan > MIXCHANNELS)
        return;

    while (MARS_SYS_COMM6 == 3);
    MARS_SYS_COMM6 = 2;

    CacheClearLine(&channel[chan]);
    if (channel[chan].snd) end_channel(chan);

    MARS_SYS_COMM6 = 1;
}

// Stop a sound
void Hw32xAudioStopAudio(sound_t *sound)
{
    unsigned char i;

    if (!sound) return;

    while (MARS_SYS_COMM6 == 3);
    MARS_SYS_COMM6 = 2;

    for (i = 0; i < MIXCHANNELS; i++)
    {
        CacheClearLine(&channel[i]);
        if (channel[i].snd == sound) end_channel(i);
    }

    MARS_SYS_COMM6 = 1;
}

// See if a sound is playing
int Hw32xAudioIsPlaying(sound_t *sound)
{
    unsigned char i, playing;

    while (MARS_SYS_COMM6 == 3) ;
    MARS_SYS_COMM6 = 2;

    playing = 0;
    for (i = 0; i < MIXCHANNELS; i++)
    {
        CacheClearLine(&channel[i]);
        if (channel[i].snd == sound) playing = 1;
    }

    MARS_SYS_COMM6 = 1;

    return playing;
}


// Stops all channels
void Hw32xAudioStopAllChannels(void)
{
    unsigned char i;

    while (MARS_SYS_COMM6 == 3) ;
    MARS_SYS_COMM6 = 2;

    for (i = 0; i < MIXCHANNELS; i++)
    {
        CacheClearLine(&channel[i]);
        if (channel[i].snd) end_channel(i);
    }

    MARS_SYS_COMM6 = 1;
}

// Load a sound
void Hw32xAudioLoad(sound_t *snd, char *name)
{
    audio_file_t *afd;

    memset(snd, 0, sizeof(sound_t));

    afd = audio_file_open(name);
    if (afd)
    {
        unsigned char p[4];
        audio_file_seek(afd, 0x28, SEEK_SET);
        audio_file_read(afd, p, 1, 4);
        snd->len = (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
        snd->buf = audio_file_mmap(afd, 0x2C);
        snd->valid = TRUE;
    }
}

void Hw32xAudioFree(sound_t *s)
{
    s->valid = FALSE;
}

// Slave SH2 support code ----------------------------------------------

void task_handler(void)
{
    MARS_SYS_COMM4 = 0; // Done
} */

/* void slave(void)
{
    u16 sample, ix;

    // Init DMA
    SH2_DMA_SAR0 = 0;
    SH2_DMA_DAR0 = 0;
    SH2_DMA_TCR0 = 0;
    SH2_DMA_CHCR0 = 0;
    SH2_DMA_DRCR0 = 0;
    SH2_DMA_SAR1 = 0;
    SH2_DMA_DAR1 = 0x20004034; // Storing a long here will set left and right
    SH2_DMA_TCR1 = 0;
    SH2_DMA_CHCR1 = 0;
    SH2_DMA_DRCR1 = 0;
    SH2_DMA_DMAOR = 1; // Enable DMA

    // Init the Audio hardware
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
        MARS_PWM_CYCLE = (((23011361 << 1)/SAMPLE_RATE + 1) >> 1) + 1; // for NTSC clock
    else
        MARS_PWM_CYCLE = (((22801467 << 1)/SAMPLE_RATE + 1) >> 1) + 1; // for PAL clock
    MARS_PWM_CTRL = 0x0185; // TM = 0, RTP, RMD = right, LMD = left
    //MARS_PWM_CTRL = 0x0184; // Right only
    //MARS_PWM_CTRL = 0x0183; // Left only

    sample = SAMPLE_MIN;
    // Ramp up to SAMPLE_CENTER to avoid click in audio (real 32X)
    while (sample < SAMPLE_CENTER)
    {
        //for (ix=0; ix<SAMPLE_RATE/SAMPLE_CENTER; ix++)
        for (ix=0; ix<(SAMPLE_RATE*2)/(SAMPLE_CENTER - SAMPLE_MIN); ix++)
        {
            while (MARS_PWM_MONO & 0x8000) ; // wait while full
            MARS_PWM_MONO = sample;
        }
        sample++;
    }

    while (1)
    {
        if (MARS_SYS_COMM4)
            task_handler();

        if (MARS_SYS_COMM6 == 2)
            continue; // Locked by Master SH2

        if (MARS_SYS_COMM6 == 0)
        {
            MARS_SYS_COMM6 = 3;
            // Fill first buffer
            Hw32xAudioCallback((unsigned long)&sndbuf);
            MARS_SYS_COMM6 = 1; // Sound subsystem running
        }

        // Only do audio when audio subsytem initialized and unlocked
        while (MARS_SYS_COMM6 == 1)
        {
            // Start DMA on first buffer and fill second
            SH2_DMA_SAR1 = ((unsigned long)&sndbuf) | 0x20000000;
            SH2_DMA_TCR1 = NUM_SAMPLES; // number longs
            SH2_DMA_CHCR1 = 0x18E1; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled

            while (MARS_SYS_COMM6 == 2) ;
            MARS_SYS_COMM6 = 3;
            Hw32xAudioCallback((unsigned long)&sndbuf + MAX_NUM_SAMPLES * 4);
            MARS_SYS_COMM6 = 1;

            // Wait on DMA
            while (!(SH2_DMA_CHCR1 & 2)) // wait on TE
            {
                if (MARS_SYS_COMM4)
                    task_handler();
            }

            // Start DMA on second buffer and fill first
            SH2_DMA_SAR1 = ((unsigned long)&sndbuf + MAX_NUM_SAMPLES * 4) | 0x20000000;
            SH2_DMA_TCR1 = NUM_SAMPLES; // Number longs
            SH2_DMA_CHCR1 = 0x18E1; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled

            while (MARS_SYS_COMM6 == 2) ;
            MARS_SYS_COMM6 = 3;
            Hw32xAudioCallback((unsigned long)&sndbuf);
            MARS_SYS_COMM6 = 1;

            // Wait on DMA
            while (!(SH2_DMA_CHCR1 & 2)) // Wait on TE
            {
                if (MARS_SYS_COMM4)
                    task_handler();
            }
        }
    }
} */ 