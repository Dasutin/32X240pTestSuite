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

#ifndef HW_32X_H
#define HW_32X_H

// Audio section
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

extern int Hw32xDetectPAL();
extern void Hw32xSetFGColor(int s, int r, int g, int b);
extern void Hw32xSetBGColor(int s, int r, int g, int b);
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

extern unsigned short sndbuf[];

extern int sysarg_args_nosound;
extern int sysarg_args_vol;

#define NUM_AUDIO_FILES 2

extern char *audioFileName[NUM_AUDIO_FILES];
extern int audioFileSize[NUM_AUDIO_FILES];
extern int audioFilePtr[NUM_AUDIO_FILES];

typedef void *audio_file_t;

extern audio_file_t *audio_file_open(char *name);
extern int audio_file_seek(audio_file_t *file, long offset, int origin);
extern int audio_file_read(audio_file_t *file, void *buf, size_t size, size_t count);
extern void *audio_file_mmap(audio_file_t *file, long offset);

extern void sysarg_init(int, char **);

#define FREQ 22050
#define CHANNELS 1
#define MAXVOL 16
#define MIXCHANNELS 8
#define MIXSAMPLES 1024
#define SAMPLE_RATE 22050
#define SAMPLE_CENTER 516
#define MAX_NUM_SAMPLES 1024
#define NUM_SAMPLES 1024
#define SAMPLE_MIN 2

#endif
