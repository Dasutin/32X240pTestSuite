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

#include "32x.h"
#include "hw_32x.h"
#include "types.h"
#include "string.h"
#include "sound.h"
#include "perf.h"

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

int sysarg_args_vol = 0;

#define SND_BUFFER_COUNT 2
#define SND_BUFFER_STRIDE (MIXSAMPLES * 2)
#define SND_DMA_TRANSFER_COUNT (SND_BUFFER_STRIDE / 2)
#define PWM_STARTUP_RAMP_SAMPLES (SAMPLE_RATE / 4)
#define MARS_SYS_INTMSK_BYTE (*(volatile uint8_t *)0x20004001u)

// Two buffers of MIXSAMPLES 32-bit stereo PWM samples
int16_t __attribute__((aligned(16))) snd_buffer[SND_BUFFER_STRIDE * SND_BUFFER_COUNT];

static channel_t __attribute__((aligned(16))) channel[MIXCHANNELS];
static int32_t mix_accumulator[MIXSAMPLES] ATTR_CACHE_ALIGNED;

volatile uint32_t sound_dma_late_count;

extern void sh2_mix_clear(int32_t *accumulator, unsigned count);
extern void sh2_mix_channel(int32_t *accumulator, const uint8_t *source,
	unsigned count, int volume);
extern void sh2_mix_finalize(uint32_t *stream, const int32_t *accumulator,
	unsigned count);
extern void sh2_mix_silence(uint32_t *stream, unsigned count);

typedef struct {
	volatile uint8_t interested[2];
	volatile uint8_t turn;
	uint8_t reserved[13];
} sound_sync_t;

#define SOUND_LOCK_PRIMARY 0
#define SOUND_LOCK_SECONDARY 1

static sound_sync_t sound_sync ATTR_CACHE_ALIGNED;

static inline volatile sound_sync_t *sound_channel_sync(void)
{
	return (volatile sound_sync_t *)((uintptr_t)&sound_sync | 0x20000000u);
}

static inline void sound_lock_channels(unsigned owner)
{
	volatile sound_sync_t *sync = sound_channel_sync();
	unsigned other = owner ^ 1;

	sync->interested[owner] = 1;
	sync->turn = other;
	__asm__ volatile ("" ::: "memory");
	while (sync->interested[other] && sync->turn == other) {}
}

static inline void sound_unlock_channels(unsigned owner)
{
	__asm__ volatile ("" ::: "memory");
	sound_channel_sync()->interested[owner] = 0;
}

static uint8_t snd_bufidx = 0;
static uint8_t snd_init = 0, snd_stopmix = 0;

static unsigned char isAudioActive = 0;

typedef struct {
	uint32_t frequency;
	uint16_t route;
	volatile uint8_t accepted;
	uint8_t reserved;
} pwm_test_request_t;

static pwm_test_request_t pwm_test_request ATTR_CACHE_ALIGNED;
static uint32_t pwm_test_phase;
static uint32_t pwm_test_step;
static uint8_t pwm_test_active;
static uint8_t pwm_test_resume_dma;

/* One cycle of a full-scale signed sine.  The phase accumulator selects the
 * sample, so frequency is independent of the table length and wraps without
 * a buffer seam. */
static const int8_t pwm_test_sine[256] = {
	   0,   3,   6,   9,  12,  16,  19,  22,  25,  28,  31,  34,  37,  40,  43,  46,
	  49,  51,  54,  57,  60,  63,  65,  68,  71,  73,  76,  78,  81,  83,  85,  88,
	  90,  92,  94,  96,  98, 100, 102, 104, 106, 107, 109, 111, 112, 113, 115, 116,
	 117, 118, 120, 121, 122, 122, 123, 124, 125, 125, 126, 126, 126, 127, 127, 127,
	 127, 127, 127, 127, 126, 126, 126, 125, 125, 124, 123, 122, 122, 121, 120, 118,
	 117, 116, 115, 113, 112, 111, 109, 107, 106, 104, 102, 100,  98,  96,  94,  92,
	  90,  88,  85,  83,  81,  78,  76,  73,  71,  68,  65,  63,  60,  57,  54,  51,
	  49,  46,  43,  40,  37,  34,  31,  28,  25,  22,  19,  16,  12,   9,   6,   3,
	   0,  -3,  -6,  -9, -12, -16, -19, -22, -25, -28, -31, -34, -37, -40, -43, -46,
	 -49, -51, -54, -57, -60, -63, -65, -68, -71, -73, -76, -78, -81, -83, -85, -88,
	 -90, -92, -94, -96, -98,-100,-102,-104,-106,-107,-109,-111,-112,-113,-115,-116,
	-117,-118,-120,-121,-122,-122,-123,-124,-125,-125,-126,-126,-126,-127,-127,-127,
	-127,-127,-127,-127,-126,-126,-126,-125,-125,-124,-123,-122,-122,-121,-120,-118,
	-117,-116,-115,-113,-112,-111,-109,-107,-106,-104,-102,-100, -98, -96, -94, -92,
	 -90, -88, -85, -83, -81, -78, -76, -73, -71, -68, -65, -63, -60, -57, -54, -51,
	 -49, -46, -43, -40, -37, -34, -31, -28, -25, -22, -19, -16, -12,  -9,  -6,  -3
};

int sound_isInitialized(void)
{
	return *(volatile uint8_t *)((uintptr_t)&snd_init | 0x20000000u) != 0;
}
// Mute flag
static unsigned char sndMute = 0;
// Internal volume
static short sndVol = MAXVOL*2;
// User-selected volume
static short sndUVol = MAXVOL;

static void end_channel(unsigned char);

void sec_dma1_handler(void)
{
	uint32_t dma_buffer;
	uint32_t mix_buffer;

	// Read TE
	SH2_DMA_CHCR1;
	// Clear TE
	SH2_DMA_CHCR1 = 0;

	dma_buffer = (uint32_t)&snd_buffer[snd_bufidx * SND_BUFFER_STRIDE];
	mix_buffer = (uint32_t)&snd_buffer[(snd_bufidx ^ 1) * SND_BUFFER_STRIDE];

	SH2_DMA_SAR1 = dma_buffer | 0x20000000;
	// Number of 32-bit stereo PWM samples in one buffer
	SH2_DMA_TCR1 = SND_DMA_TRANSFER_COUNT;
	// Dest fixed, src incr, size long, ext req, dack mem to dev, dack hi, dack edge, dreq rising edge, cycle-steal, dual addr, intr disabled, clear TE, dma enabled
	SH2_DMA_CHCR1 = 0x18E5;

	// Refill only the half not currently owned by DMA.
	sound_fillBuffer(mix_buffer);
	if (SH2_DMA_CHCR1 & 2)
		(*(volatile uint32_t *)((uintptr_t)&sound_dma_late_count |
			0x20000000u))++;
	snd_bufidx ^= 1;
}

void Mars_Sec_InitSoundDMA(void)
{
	uint16_t sample, ix;
	uint16_t rampSamplesPerStep;

	if (snd_init) {
		if (snd_stopmix)
			Mars_Sec_StartSoundMixer();
		return;
	}

	// Init DMA
	SH2_DMA_SAR0 = 0;
	SH2_DMA_DAR0 = 0;
	SH2_DMA_TCR0 = 0;
	SH2_DMA_CHCR0 = 0;
	SH2_DMA_DRCR0 = 0;
	SH2_DMA_SAR1 = 0;
	// Storing a long here will set left and right
	SH2_DMA_DAR1 = 0x20004034;
	SH2_DMA_TCR1 = 0;
	SH2_DMA_CHCR1 = 0;
	SH2_DMA_DRCR1 = 0;
	// Enable DMA
	SH2_DMA_DMAOR = 1;

	// Set exception vector for DMA channel 1
	SH2_DMA_VCR1 = 72;
	SH2_INT_IPRA = (SH2_INT_IPRA & 0xF0FF) | 0x0400;

	// Init the sound hardware
	MARS_PWM_MONO = 1;
	MARS_PWM_MONO = 1;
	MARS_PWM_MONO = 1;
	if (MARS_VDP_DISPMODE & MARS_NTSC_FORMAT)
		// NTSC clock
		MARS_PWM_CYCLE = (((23011361 << 1) / (SAMPLE_RATE) + 1) >> 1) + 1;
	else
		// PAL clock
		MARS_PWM_CYCLE = (((22801467 << 1) / (SAMPLE_RATE) + 1) >> 1) + 1;
	// TM = 1, RTP, RMD = right, LMD = left
	MARS_PWM_CTRL = 0x0185;

	sample = SAMPLE_MIN;
	rampSamplesPerStep = PWM_STARTUP_RAMP_SAMPLES /
		(SAMPLE_CENTER - SAMPLE_MIN);
	if (!rampSamplesPerStep)
		rampSamplesPerStep = 1;

	// Briefly ramp to center to avoid a cold-start click on real hardware.
	while (sample < SAMPLE_CENTER)
	{
		for (ix = 0; ix < rampSamplesPerStep; ix++)
		{
			// Wait while full
			while (MARS_PWM_MONO & 0x8000);
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
	SetSH2SR(15);
	SH2_DMA_CHCR1; // Read TE
	SH2_DMA_CHCR1 = 0; // Clear TE

	snd_stopmix = 1;
}

void Mars_Sec_StartSoundMixer(void)
{
	SetSH2SR(15);
	SH2_DMA_CHCR1;
	SH2_DMA_CHCR1 = 0;
	snd_stopmix = 0;
	snd_bufidx = 0;

	// Fill first buffer
	sound_fillBuffer((uint32_t)&snd_buffer[0]);

	// Start DMA
	sec_dma1_handler();

	isAudioActive = 1;

	SetSH2SR(2);
}

static void pwm_test_fill_center(void)
{
	MARS_PWM_MONO = SAMPLE_CENTER;
	MARS_PWM_MONO = SAMPLE_CENTER;
	MARS_PWM_MONO = SAMPLE_CENTER;
}

void Mars_Sec_StartTestPWMTone(void)
{
	volatile pwm_test_request_t *request = (volatile pwm_test_request_t *)
		((uintptr_t)&pwm_test_request | 0x20000000u);
	uint16_t route;

	SetSH2SR(15);
	request->accepted = 0;
	if (!snd_init || !request->frequency ||
		request->frequency > SAMPLE_RATE / 2) {
		SetSH2SR(2);
		return;
	}

	if (!pwm_test_active)
		pwm_test_resume_dma = !snd_stopmix;

	SH2_DMA_CHCR1;
	SH2_DMA_CHCR1 = 0;
	snd_stopmix = 1;
	MARS_SYS_INTMSK_BYTE &= (uint8_t)~0x01;
	MARS_PWM_CTRL = 0;
	pwm_test_fill_center();

	pwm_test_phase = 0;
	pwm_test_step = (uint32_t)((((uint64_t)request->frequency << 32) +
		SAMPLE_RATE / 2) / SAMPLE_RATE);
	route = request->route;
	if (route != 0x0002 && route != 0x0004 && route != 0x0005)
		route = 0x0005;

	MARS_SYS_PWMI_CLR = 0;
	MARS_SYS_PWMI_CLR;
	pwm_test_active = 1;

	MARS_PWM_CTRL = (3u << 8) | route;
	MARS_SYS_INTMSK_BYTE |= 0x01;
	request->accepted = 1;
	SetSH2SR(2);
}

void Mars_Sec_StopTestPWMTone(void)
{
	SetSH2SR(15);
	MARS_SYS_INTMSK_BYTE &= (uint8_t)~0x01;
	MARS_SYS_PWMI_CLR = 0;
	MARS_SYS_PWMI_CLR;

	if (!pwm_test_active) {
		SetSH2SR(2);
		return;
	}

	pwm_test_active = 0;
	MARS_PWM_CTRL = 0;
	pwm_test_fill_center();

	if (pwm_test_resume_dma) {
		MARS_PWM_CTRL = 0x0185;
		Mars_Sec_StartSoundMixer();
	} else {
		MARS_PWM_CTRL = 0x0005;
		SetSH2SR(2);
	}
}

void sec_pwm_tone_handler(void)
{
	unsigned count;

	if (!pwm_test_active || !pwm_test_step)
		return;

	for (count = 0; count < 3; count++) {
		int sample;

		if (MARS_PWM_MONO & 0x8000)
			break;
		sample = pwm_test_sine[pwm_test_phase >> 24];
		pwm_test_phase += pwm_test_step;
		MARS_PWM_MONO = (uint16_t)(SAMPLE_CENTER + sample * 2);
	}
}

int sound_test_pwm_start(uint32_t frequency, char selectch)
{
	volatile pwm_test_request_t *request = (volatile pwm_test_request_t *)
		((uintptr_t)&pwm_test_request | 0x20000000u);

	if (!frequency || frequency > SAMPLE_RATE / 2 || !sound_isInitialized())
		return 0;

	request->frequency = frequency;
	request->route = selectch == 1 ? 0x0002 :
		(selectch == 2 ? 0x0004 : 0x0005);
	request->accepted = 0;
	__asm__ volatile ("" ::: "memory");
	Mars_R_SecWait();
	MARS_SYS_COMM4 = MARS_SEC_CMD_PWM_TEST_START;
	Mars_R_SecWait();
	return request->accepted != 0;
}

void sound_test_pwm_stop(void)
{
	Mars_R_SecWait();
	MARS_SYS_COMM4 = MARS_SEC_CMD_PWM_TEST_STOP;
	Mars_R_SecWait();
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

signed char sound_play(sound_t *sound, signed char loop, char selectch)
{
	signed char c;

	if (!*(volatile uint8_t *)((uintptr_t)&isAudioActive | 0x20000000u))
		return -1;
	if (sound == NULL) return -1;

	sound_lock_channels(SOUND_LOCK_PRIMARY);

	for (c = 0; c < MIXCHANNELS; c++)
	{
		CacheClearLine(&channel[c]);
		if (channel[c].loop == 0 || channel[c].snd == sound)
			break;
	}

	if (c == MIXCHANNELS)
	{
		sound_unlock_channels(SOUND_LOCK_PRIMARY);
		return -1;
	}

	channel[c].loop = loop;
	channel[c].snd = sound;
	channel[c].buf = sound->buf;
	channel[c].len = sound->len;

	sound_unlock_channels(SOUND_LOCK_PRIMARY);

	switch (selectch)
	{
		case 1:
			// Left Channel Only
			MARS_PWM_CTRL = 0x0182;
			break;
		case 2:
			// Right Channel Only
			MARS_PWM_CTRL = 0x0184;
			break;
		case 3:
			// Center
			MARS_PWM_CTRL = 0x0185;
			break;
	}

	if (*(volatile uint8_t *)((uintptr_t)&snd_stopmix | 0x20000000u))
		Mars_StartSoundMixer();

	return c;
}

void sound_pause(char pause)
{
	volatile unsigned char *active = (volatile unsigned char *)
		((uintptr_t)&isAudioActive | 0x20000000u);

	if (pause == 1)
		*active = 0;
	else
		*active = 1;
}

void sound_stopChannel(unsigned char chan)
{
	if (chan >= MIXCHANNELS)
		return;

	sound_lock_channels(SOUND_LOCK_PRIMARY);

	CacheClearLine(&channel[chan]);
	if (channel[chan].snd) end_channel(chan);

	sound_unlock_channels(SOUND_LOCK_PRIMARY);
}

void sound_stopSound(sound_t *sound)
{
	unsigned char i;

	if (!sound) return;

	sound_lock_channels(SOUND_LOCK_PRIMARY);

	for (i = 0; i < MIXCHANNELS; i++)
	{
		CacheClearLine(&channel[i]);
		if (channel[i].snd == sound) end_channel(i);
	}

	sound_unlock_channels(SOUND_LOCK_PRIMARY);
}

int sound_isPlaying(sound_t *sound)
{
	unsigned char i, playing;

	sound_lock_channels(SOUND_LOCK_PRIMARY);

	playing = 0;
	for (i = 0; i < MIXCHANNELS; i++)
	{
		CacheClearLine(&channel[i]);
		if (channel[i].snd == sound) playing = 1;
	}

	sound_unlock_channels(SOUND_LOCK_PRIMARY);

	return playing;
}

void sound_stopAllChannels(void)
{
	unsigned char i;

	sound_lock_channels(SOUND_LOCK_PRIMARY);

	for (i = 0; i < MIXCHANNELS; i++)
	{
		CacheClearLine(&channel[i]);
		if (channel[i].snd) end_channel(i);
	}

	sound_unlock_channels(SOUND_LOCK_PRIMARY);
}

void sound_load(sound_t *snd, char *name)
{
	sound_file_t *afd;

	myMemSet(snd, 0, sizeof(sound_t));

	afd = sound_file_open(name);
	if (afd)
	{
		unsigned char p[4];
		sound_file_seek(afd, 0x28, SEEK_SET);
		sound_file_read(afd, p, 1, 4);
		snd->len = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
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
	uint32_t perf_start = perf_slave_ticks();
	uint32_t *stream = (uint32_t *)(buff | 0x20000000);
	int32_t *accumulator = (int32_t *)((uintptr_t)mix_accumulator |
		0x20000000u);
	channel_t *schannel = (channel_t *)((uintptr_t)channel | 0x20000000u);
	short volume = *(short *)((uintptr_t)&sndVol | 0x20000000u);
	unsigned char mute = *(unsigned char *)((uintptr_t)&sndMute |
		0x20000000u);
	unsigned c;

	if (!snd_init) {
		sh2_mix_silence(stream, MIXSAMPLES);
		goto done;
	}

	sh2_mix_clear(accumulator, MIXSAMPLES);
	sound_lock_channels(SOUND_LOCK_SECONDARY);

	for (c = 0; c < MIXCHANNELS; c++) {
		channel_t *ch = &schannel[c];
		unsigned output = 0;

		if (!ch->loop || !ch->snd)
			continue;
		while (output < MIXSAMPLES && ch->loop) {
			unsigned span;

			if (!ch->len) {
				sound_t *source;

				if (ch->loop > 0)
					ch->loop--;
				if (!ch->loop || !ch->snd) {
					ch->loop = 0;
					ch->snd = NULL;
					break;
				}

				source = (sound_t *)((uintptr_t)ch->snd | 0x20000000u);
				if (!source->buf || !source->len) {
					ch->loop = 0;
					ch->snd = NULL;
					break;
				}
				ch->buf = source->buf;
				ch->len = source->len;
			}

			span = ch->len;
			if (span > MIXSAMPLES - output)
				span = MIXSAMPLES - output;
			sh2_mix_channel(accumulator + output, ch->buf, span, volume);
			ch->buf += span;
			ch->len -= span;
			output += span;
		}
	}

	sound_unlock_channels(SOUND_LOCK_SECONDARY);
	if (mute)
		sh2_mix_silence(stream, MIXSAMPLES);
	else
		sh2_mix_finalize(stream, accumulator, MIXSAMPLES);

done:
	perf_record(PERF_CPU_SLAVE, PERF_METRIC_SOUND_MIX,
		perf_slave_ticks() - perf_start);
}
