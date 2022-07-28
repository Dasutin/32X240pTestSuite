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

#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdint.h>

#define SAMPLE_MIN         2
#define SAMPLE_CENTER    517
#define SAMPLE_MAX      1032
#define FREQ 22050
#define CHANNELS 1
#define MAXVOL 16
#define MIXCHANNELS 8
#define MIXSAMPLES 1024
#define SAMPLE_RATE 22050
#define MAX_NUM_SAMPLES 1024
#define NUM_SAMPLES 1024

#define SND_ATTR_SDRAM  __attribute__((section(".data"), aligned(16)))

#define ADJVOL(S) ((S)*ssndVol)

extern unsigned short sndbuf[];

typedef struct {
  unsigned char *buf;
  unsigned long len;
  unsigned char valid;
} sound_t;

typedef struct {
	sound_t *snd;
	unsigned char *buf;
	unsigned long len;
	char loop;
    char pan;       // When get around to making stereo sfx
    unsigned char pad[2]; // Pad to one cache line
} channel_t;

void snddma_submit(void) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf(int channels, int num_samples) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf_mono(int num_samples) SND_ATTR_SDRAM;
uint16_t* snddma_get_buf_stereo(int num_samples) SND_ATTR_SDRAM;

static inline uint16_t s16pcm_to_u16pwm(int16_t s) {
    s = (s >> 5) + SAMPLE_CENTER;
    return (s < 0) ? SAMPLE_MIN : (s > SAMPLE_MAX) ? SAMPLE_MAX : s;
}

void snddma_secondary_init(int sample_rate);
void snddma_init(int sample_rate);
void secondary_dma_kickstart(void);
unsigned snddma_length(void)SND_ATTR_SDRAM;
void snddma_wait(void) SND_ATTR_SDRAM;

extern void Hw32xAudioCallback(unsigned long buffer);
extern void Hw32xAudioInit(void);
extern void Hw32xAudioShutdown(void);
extern void Hw32xAudioToggleMute(void);
extern void Hw32xAudioVolume(char d);
extern char Hw32xAudioPlay(sound_t *sound, char loop, char selectch);
extern void Hw32xAudioPause(char pause);
extern void Hw32xAudioStopChannel(unsigned char chan);
extern void Hw32xAudioStopAudio(sound_t *sound);
extern int Hw32xAudioIsPlaying(sound_t *sound);
extern void Hw32xAudioStopAllChannels(void);
extern void Hw32xAudioLoad(sound_t *snd, char *name);
extern void Hw32xAudioFree(sound_t *s);


extern int sysarg_args_nosound;
extern int sysarg_args_vol;

#define NUM_SOUND_FILES 2

extern char *soundFileName[NUM_SOUND_FILES];
extern int soundFileSize[NUM_SOUND_FILES];
extern int soundFilePtr[NUM_SOUND_FILES];

typedef void *sound_file_t;

extern sound_file_t *sound_file_open(char *name);
extern int sound_file_seek(sound_file_t *file, long offset, int origin);
extern int sound_file_read(sound_file_t *file, void *buf, size_t size, size_t count);
extern void *sound_file_mmap(sound_file_t *file, long offset);

extern void sysarg_init(int, char **);

#endif /* _SOUND_H_ */