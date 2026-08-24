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

#ifndef _PERF_H_
#define _PERF_H_

#include <stdint.h>

#include "types.h"

typedef enum perf_cpu_t {
	PERF_CPU_MASTER = 0,
	PERF_CPU_SLAVE,
	PERF_CPU_COUNT
} perf_cpu_t;

typedef enum perf_scene_t {
	PERF_SCENE_OTHER = 0,
	PERF_SCENE_MENU,
	PERF_SCENE_SCROLL,
	PERF_SCENE_PHASE_CHECK,
	PERF_SCENE_LAYERS,
	PERF_SCENE_AUDIO_SYNC,
	PERF_SCENE_SCD_OVERLAY,
	PERF_SCENE_COUNT
} perf_scene_t;

typedef enum perf_metric_t {
	PERF_METRIC_TILEMAP = 0,
	PERF_METRIC_SPRITE,
	PERF_METRIC_SPRITE_ROM,
	PERF_METRIC_SPRITE_SDRAM,
	PERF_METRIC_TEXT,
	PERF_METRIC_LINE_TABLE,
	PERF_METRIC_SOUND_MIX,
	PERF_METRIC_COUNT
} perf_metric_t;

typedef struct perf_counter_t {
	volatile uint32_t ticks;
	volatile uint32_t calls;
	volatile uint32_t maximum;
} perf_counter_t;

extern volatile uint16_t perf_current_scene;
extern perf_counter_t perf_counters[PERF_CPU_COUNT][PERF_SCENE_COUNT]
	[PERF_METRIC_COUNT] ATTR_CACHE_ALIGNED;

uint32_t perf_master_ticks(void) ATTR_DATA_ALIGNED;
uint32_t perf_slave_ticks(void) ATTR_DATA_ALIGNED;
void perf_record(perf_cpu_t cpu, perf_metric_t metric, uint32_t elapsed)
	ATTR_DATA_ALIGNED;
void perf_record_sprite(perf_cpu_t cpu, const void *source, uint32_t elapsed)
	ATTR_DATA_ALIGNED;
void perf_reset(void) ATTR_DATA_ALIGNED;
void perf_set_scene(perf_scene_t scene) ATTR_DATA_ALIGNED;

#endif /* _PERF_H_ */
