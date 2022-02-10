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

#define SAMPLE_MIN         2
#define SAMPLE_CENTER    517
#define SAMPLE_MAX      1032

#define SND_ATTR_SDRAM  __attribute__((section(".data"), aligned(16)))

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