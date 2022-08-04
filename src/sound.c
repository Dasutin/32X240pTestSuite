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

#include <stdint.h>
#include "string.h"
#include "32x.h"
#include "hw_32x.h"
#include "sound.h"

int sysarg_args_vol = 0;

int16_t __attribute__((aligned(16))) snd_buffer[MIXSAMPLES*2*2]; // Two buffers of MIXSAMPLES words of stereo pwm audio
static channel_t __attribute__((aligned(16))) channel[MIXCHANNELS];

static uint8_t snd_bufidx = 0;
static uint8_t snd_init = 0, snd_stopmix = 0;

static unsigned char isAudioActive = 0;
static unsigned char sndMute = 0;  // Mute flag
static short sndVol = MAXVOL*2;  // Internal volume
static short sndUVol = MAXVOL;  // User-selected volume

static void end_channel(unsigned char);

void sec_dma1_handler(void)
{
	static int32_t which = 0;

	SH2_DMA_CHCR1; // Read TE
	SH2_DMA_CHCR1 = 0; // Clear TE

	if (which)
	{
		SH2_DMA_SAR1 = ((uint32_t)&snd_buffer[0]) | 0x20000000;
		SH2_DMA_TCR1 = 1024; // Number longs
		SH2_DMA_CHCR1 = 0x18E5; // Dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled

		sound_fillBuffer(&snd_buffer[0]);
	}
	else {
		SH2_DMA_SAR1 = ((uint32_t)&snd_buffer[0]) | 0x20000000;
		SH2_DMA_TCR1 = 1023; // Number longs
		SH2_DMA_CHCR1 = 0x18E5; // Dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled

		sound_fillBuffer(&snd_buffer[0]);
	}

	which ^= 1; // flip audio buffer
}

void Mars_Sec_InitSoundDMA(void)
{
	uint16_t sample, ix;

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

	SH2_DMA_VCR1 = 72; // Set exception vector for DMA channel 1
	SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0F00; // Set DMA INT to priority 15

	// Init the sound hardware
	MARS_PWM_MONO = 1;
	MARS_PWM_MONO = 1;
	MARS_PWM_MONO = 1;
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		MARS_PWM_CYCLE = (((23011361 << 1) / (SAMPLE_RATE) + 1) >> 1) + 1; // for NTSC clock
	else
		MARS_PWM_CYCLE = (((22801467 << 1) / (SAMPLE_RATE) + 1) >> 1) + 1; // for PAL clock
	MARS_PWM_CTRL = 0x0185; // TM = 1, RTP, RMD = right, LMD = left

	sample = SAMPLE_MIN;

	// Ramp up to SAMPLE_CENTER to avoid click in audio (real 32X)
	while (sample < SAMPLE_CENTER)
	{
		for (ix = 0; ix < (SAMPLE_RATE * 2) / (SAMPLE_CENTER - SAMPLE_MIN); ix++)
		{
			while (MARS_PWM_MONO & 0x8000); // Wait while full
			MARS_PWM_MONO = sample;
		}
		sample++;
	}

	snd_bufidx = 0;
	snd_init = 1;
	snd_stopmix = 0;

	Mars_Sec_StartSoundMixer();
}

void Mars_Sec_StopSoundMixer(void)
{
	SH2_DMA_CHCR1; // Read TE
	SH2_DMA_CHCR1 = 0; // Clear TE

	snd_stopmix = 1;
}

void Mars_Sec_StartSoundMixer(void)
{
	snd_stopmix = 0;

	// Fill first buffer
	//Hw32xAudioCallback((unsigned long)&snd_buffer);
	sound_fillBuffer(&snd_buffer[0]);

	// Start DMA
	sec_dma1_handler();

	isAudioActive = 1;

	// SetSH2SR(2);
}

// Audio Data Loading Code

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

void sound_toggleMute(void)
{
	sndMute = !sndMute;
}

void sound_volume(char d)
{
	if ((d < 0 && sndUVol > 0) || (d > 0 && sndUVol < MAXVOL)) 
	{
		sndUVol += d;
		sndVol = sndUVol << 1;
	}
}

char sound_play(sound_t *sound, char loop, char selectch)
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

    //while (MARS_SYS_COMM6 == 3);
    Mars_StartSoundMixer();

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

void sound_pause(char pause)
{
    if (pause == 1)
        isAudioActive = 0;
    else
        isAudioActive = 1;
}

void sound_stopChannel(unsigned char chan)
{
    if (chan < 0 || chan > MIXCHANNELS)
        return;

    while (MARS_SYS_COMM6 == 3);
    MARS_SYS_COMM6 = 2;

    CacheClearLine(&channel[chan]);
    if (channel[chan].snd) end_channel(chan);

    MARS_SYS_COMM6 = 1;
}

void sound_stopSound(sound_t *sound)
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

int sound_isPlaying(sound_t *sound)
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

void sound_stopAllChannels(void)
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

void sound_load(sound_t *snd, char *name)
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

void sound_free(sound_t *s)
{
	s->valid = 0;
}

void sound_fillBuffer(unsigned long buff)
{
    unsigned char c;
    short s;
    unsigned int i;
    unsigned int *stream = (unsigned int *)(buff | 0x20000000);
    //unsigned char sisAudioActive = *(unsigned char *)((unsigned int)&isAudioActive | 0x20000000);
    short ssndVol = *(short *)((unsigned int)&sndVol | 0x20000000);
    //unsigned char ssndMute = *(unsigned char *)((unsigned int)&sndMute | 0x20000000);
    channel_t *schannel = (channel_t *)((unsigned int)channel | 0x20000000);

    if (snd_init)
	{
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
                	else {  // Ending
                        if (schannel[c].loop > 0) schannel[c].loop--;
                        	if (schannel[c].loop)
                        	{  // Just loop
                            	schannel[c].buf = schannel[c].snd->buf;
                            	schannel[c].len = schannel[c].snd->len;
                            	s += ADJVOL(*schannel[c].buf - 0x80);
                            	schannel[c].buf++;
                            	schannel[c].len--;
                        	}
                    		else {
                        		end_channel(c); // End for real
                        }
                    }
                }
            }
        //if (ssndMute)
        //stream[i] = ((unsigned long)516<<16) | (unsigned long)516;
        //else 
        //{
            s >>= 4;
            if (s > 512) s = 512;
            else if (s < -512) s = -512;
            s += 516;
            stream[i] = ((unsigned long)s<<16) | (unsigned long)s;
        //}
        }
    } 
    else {
        for (i = 0; i < MIXSAMPLES; i++)
        stream[i] = ((unsigned long)516<<16) | (unsigned long)516;
    }
}
