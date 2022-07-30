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

#include <stdint.h>
#include <strings.h>
#include "string.h"
#include "32x.h"
#include "sound.h"
#include "mars_ringbuf.h"

unsigned short __attribute__((aligned(16))) sndbuf[MIXSAMPLES*2*2]; // two buffers of MIXSAMPLES words of stereo pwm audio
static channel_t __attribute__((aligned(16))) channel[MIXCHANNELS];

static unsigned char isAudioActive = 0;
static unsigned char sndMute = 0;  // Mute flag
static short sndVol = MAXVOL*2;  // Internal volume
static short sndUVol = MAXVOL;  // User-selected volume

static void end_channel(unsigned char);

extern unsigned short sndbuf[];

marsrb_t soundbuf;

void secondary_dma1_handler(void) SND_ATTR_SDRAM;
void secondary_dma_kickstart(void) SND_ATTR_SDRAM;

uint16_t* snddma_get_buf(int channels, int num_samples) {
    uint16_t* p;

    p = (uint16_t*)Mars_RB_GetWriteBuf(&soundbuf, 8);
    if (!p)
        return NULL;
    *p++ = channels;
    *p++ = num_samples;
    Mars_RB_CommitWrite(&soundbuf);

    return (uint16_t*)Mars_RB_GetWriteBuf(&soundbuf, num_samples * channels);
}

uint16_t* snddma_get_buf_mono(int num_samples) {
    return snddma_get_buf(1, num_samples);
}

uint16_t* snddma_get_buf_stereo(int num_samples) {
    return snddma_get_buf(2, num_samples);
}

void snddma_wait(void) {
    Mars_RB_WaitReader(&soundbuf, 0);
}

void snddma_submit(void) {
    Mars_RB_CommitWrite(&soundbuf);
}

unsigned snddma_length(void)
{
    return Mars_RB_Len(&soundbuf);
}

 void secondary_dma_kickstart(void)
{
    static short kickstart_samples[16] __attribute__((aligned(16))) = {
        SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER,
        SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER
    };

    SH2_DMA_SAR1 = (intptr_t)kickstart_samples;
    SH2_DMA_TCR1 = sizeof(kickstart_samples) / sizeof(kickstart_samples[0]);
    SH2_DMA_DAR1 = 0x20004038; // storing a word here will the MONO channel
    SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
} 

void sec_dma1_handler(void)
{
    //Mars_RB_CommitRead(&sndbuf);

    //SH2_DMA_CHCR1; // read TE
    //SH2_DMA_CHCR1 = 0; // clear TE

    //if (Mars_RB_Len(&sndbuf) == 0)
    //{
        // sound buffer UNDERRUN
        secondary_dma_kickstart();
    //    return;
    //}

    Hw32xAudioCallback((unsigned long)&sndbuf);

    //static short kickstart_samples[16] __attribute__((aligned(16))) = {
    //    SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER,
    //    SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER, SAMPLE_CENTER
    //};

    //SH2_DMA_SAR1 = (intptr_t)kickstart_samples;
    //SH2_DMA_TCR1 = sizeof(kickstart_samples) / sizeof(kickstart_samples[0]);
    //SH2_DMA_DAR1 = 0x20004038; // storing a word here will the MONO channel
    //SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled

    //short* p = Mars_RB_GetReadBuf(&sndbuf, 8);
    //int num_channels = *p++;
    //int num_samples = *p++;
    //Mars_RB_CommitRead(&sndbuf);

    //p = Mars_RB_GetReadBuf(&sndbuf, num_samples * num_channels);

    SH2_DMA_SAR1 = ((unsigned long)&sndbuf) | 0x20000000;
    SH2_DMA_TCR1 = NUM_SAMPLES;
    SH2_DMA_CHCR1 = 0x18E1; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled
    //if (num_channels == 2)
    //{

    Hw32xAudioCallback((unsigned long)&sndbuf + MAX_NUM_SAMPLES * 4);

    SH2_DMA_DAR1 = 0x20004034; // storing a long here will set left and right
    SH2_DMA_CHCR1 = 0x18e5; // dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    //}
   // else
    //{
    //    SH2_DMA_DAR1 = 0x20004038; // storing a word here will set the MONO channel
    //    SH2_DMA_CHCR1 = 0x14e5; // dest fixed, src incr, size word, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr enabled, clear TE, dma enabled
    //}

    Hw32xAudioCallback((unsigned long)&sndbuf);
}

void snddma_secondary_init(int sample_rate)
{
    uint16_t sample, ix;

    //Mars_RB_ResetRead(&sndbuf);

    // init DMA
    SH2_DMA_SAR0 = 0;
    SH2_DMA_DAR0 = 0;
    SH2_DMA_TCR0 = 0;
    SH2_DMA_CHCR0 = 0;
    SH2_DMA_DRCR0 = 0;
    SH2_DMA_SAR1 = 0;
    SH2_DMA_DAR1 = 0x20004034; // storing a word here will the MONO channel
    SH2_DMA_TCR1 = 0;
    SH2_DMA_CHCR1 = 0;
    SH2_DMA_DRCR1 = 0;
    SH2_DMA_DMAOR = 1; // enable DMA

    SH2_DMA_VCR1 = 72; // set exception vector for DMA channel 1
    SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0F00; // set DMA INT to priority 15
    
    // init the sound hardware
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    MARS_PWM_MONO = 1;
    if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
        MARS_PWM_CYCLE = (((23011361 << 1) / (sample_rate) + 1) >> 1) + 1; // for NTSC clock
    else
        MARS_PWM_CYCLE = (((22801467 << 1) / (sample_rate) + 1) >> 1) + 1; // for PAL clock
    MARS_PWM_CTRL = 0x0185; // TM = 1, RTP, RMD = right, LMD = left

    sample = SAMPLE_MIN;

    // ramp up to SAMPLE_CENTER to avoid click in audio (real 32X)
    while (sample < SAMPLE_CENTER)
    {
        for (ix = 0; ix < (sample_rate * 2) / (SAMPLE_CENTER - SAMPLE_MIN); ix++)
        {
            while (MARS_PWM_MONO & 0x8000); // wait while full
            MARS_PWM_MONO = sample;
        }
        sample++;
    }

    secondary_dma_kickstart();

    SetSH2SR(2);
}

void snddma_init(int sample_rate)
{
    Mars_RB_ResetAll(&sndbuf);
}

// Audio Data Loading Code -----------------------------------------------------------------------------------------

static int foffs[NUM_SOUND_FILES];

// Open Data File
sound_file_t *sound_file_open(char *name)
{
    int ix;

    for (ix=0; ix<NUM_SOUND_FILES; ix++)
        if (!strcasecmp(name, soundFileName[ix]))
        {
            foffs[ix] = 0;
            return (sound_file_t *)(ix + 1);
        }

    return (sound_file_t *)0;
}

// Size
//int data_file_size(data_file_t *file)
//{
//  return fileSize[(int)file - 1];
//}

// Seek
int sound_file_seek(sound_file_t *file, long offset, int origin)
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
            foffs[(int)file - 1] = soundFileSize[(int)file - 1] + offset;
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
int sound_file_read(sound_file_t *file, void *buf, size_t size, size_t count)
{
    memcpy(buf, (char *)(soundFilePtr[(int)file - 1] + foffs[(int)file - 1]), size * count);
    return size * count;
}

// Memory Map
void *sound_file_mmap(sound_file_t *file, long offset)
{
    return (void *)(soundFilePtr[(int)file - 1] + offset);
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

    isAudioActive = 1;
}

void Hw32xAudioShutdown(void)
{
    isAudioActive = 0;
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

char Hw32xAudioPlay(sound_t *sound, char loop, char selectch)
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
}

// Pause
void Hw32xAudioPause(char pause)
{
    if (pause == 1)
        isAudioActive = 0;
    else
        isAudioActive = 1;
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
    sound_file_t *afd;

    memset(snd, 0, sizeof(sound_t));

    afd = sound_file_open(name);
    if (afd)
    {
        unsigned char p[4];
        sound_file_seek(afd, 0x28, SEEK_SET);
        sound_file_read(afd, p, 1, 4);
        snd->len = (p[3]<<24) | (p[2]<<16) | (p[1]<<8) | p[0];
        snd->buf = sound_file_mmap(afd, 0x2C);
        snd->valid = 1;
    }
}

void Hw32xAudioFree(sound_t *s)
{
    s->valid = 0;
}

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