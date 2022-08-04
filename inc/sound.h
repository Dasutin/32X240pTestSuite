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

#ifndef _SOUND_H_
#define _SOUND_H_

#include <stdint.h>
#include <stdio.h>

#define SAMPLE_MIN          2
#define SAMPLE_CENTER     517
#define SAMPLE_MAX       1032
#define FREQ            22050
#define CHANNELS            1
#define MAXVOL             16
#define MIXCHANNELS         8
#define MIXSAMPLES       1024
#define SAMPLE_RATE     22050
#define MAX_NUM_SAMPLES  1024
#define NUM_SAMPLES      1024

#define NUM_SOUND_FILES 1

#define SND_ATTR_SDRAM  __attribute__((section(".data"), aligned(16)))

#define ADJVOL(S) ((S)*ssndVol)

typedef struct sound_t {
	unsigned char *buf;
	unsigned long len;
	unsigned char valid;
} sound_t;

typedef struct {
	sound_t *snd;
	unsigned char *buf;
	unsigned long len;
	char loop;
	char pan;			// When get around to making stereo sfx
	unsigned char pad[2]; // Pad to one cache line
} channel_t;

void sound_fillBuffer(unsigned long buffer);
void sound_toggleMute(void);
void sound_volume(char d);
char sound_play(sound_t *sound, char loop, char selectch);
void sound_pause(char pause);
void sound_stopChannel(unsigned char chan);
void sound_stopSound(sound_t *sound);
int sound_isPlaying(sound_t *sound);
void sound_stopAllChannels(void);
void sound_load(sound_t *snd, char *name);
void sound_free(sound_t *s);

extern int16_t snd_buffer[];

extern char *soundFileName[NUM_SOUND_FILES];
extern int soundFileSize[NUM_SOUND_FILES];
extern int soundFilePtr[NUM_SOUND_FILES];

typedef void *sound_file_t;

void Mars_Sec_InitSoundDMA(void);
void Mars_Sec_StopSoundMixer(void);
void Mars_Sec_StartSoundMixer(void);

extern sound_file_t *sound_file_open(char *name);
extern int sound_file_seek(sound_file_t *file, long offset, int origin);
extern int sound_file_read(sound_file_t *file, void *buf, size_t size, size_t count);
extern void *sound_file_mmap(sound_file_t *file, long offset);

#endif /* _SOUND_H_ */